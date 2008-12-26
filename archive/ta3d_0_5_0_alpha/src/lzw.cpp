/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "stdafx.h"
#include "lzw.h"

#define LZW_DIC_SIZE		4096
#define LZW_HASH_LIMIT( a )	(a & 0xFFF)

#define LZW_CODE_CLEAR	0x100
#define LZW_CODE_EOF	0x101

void BYTE_FLOW::put_code( int code )
{
	int pos_b = pos >> 3;
	int pos_c = pos & 0x7;
	flow[ pos_b ] |= code >> (s - 8 + pos_c);
	if( 16 - s - pos_c > 0 ) 
		flow[ pos_b + 1 ] |= code << (16 - s - pos_c);
	else {
		flow[ pos_b + 1 ] |= code >> (s + pos_c - 16);
		flow[ pos_b + 2 ] |= code << (24 - s - pos_c);
		}
	pos += s;
}

int BYTE_FLOW::get_code()
{
	int code = 0;
	int pos_b = pos >> 3;
	int pos_c = pos & 0x7;
	int mask = ~( 0xFFFFFFFF << s );
	code = (flow[ pos_b ] & ( mask >> (s - 8 + pos_c) )) << (s - 8 + pos_c);
	if( 16 - s - pos_c > 0 )
		code |= ( flow[ pos_b + 1 ] & (mask << (16 - s - pos_c)) ) >> (16 - s - pos_c);
	else {
		code |= ( flow[ pos_b + 1 ] & (mask >> (s + pos_c - 16)) ) << (s + pos_c - 16);
		code |= ( flow[ pos_b + 2 ] & (mask << (24 - s - pos_c)) ) >> (24 - s - pos_c);
		}
	pos += s;
	return code;
}

inline bool compare(byte *a,byte *b,int n)
{
	int i, l = n >> 2 ;
	for( i = 0 ; i < l ; i++ )
		if( ((uint32*)a)[i] != ((uint32*)b)[i] )
			return false;
	for( i = (l << 2) ; i < n ; i++ )
		if( a[i] != b[i] )
			return false;
	return true;
}

byte *LZW_compress( byte *data, int size )
{
	byte			*dic_data[ LZW_DIC_SIZE ];
	int				dic_length[ LZW_DIC_SIZE ];
	List< uint32 >	dic_hash[ LZW_DIC_SIZE ];
	int				dic_size = 258;

	for( int i = 256 ; i < LZW_DIC_SIZE ; i++ )
		dic_data[i] = NULL;

	for( int i = 0 ; i < 256 ; i++ ) {
		dic_length[ i ] = 1;
		dic_data[ i ] = new byte[1];
		dic_data[ i ][0] = i;
		}

	byte *cur = new byte[ size ];
	int	 cur_size = 0;
	byte *buf = new byte[ size + 4 << 1 ];

	memset( buf, 0, size + 4 << 1 );

	BYTE_FLOW	flow( buf + 8 );			// Starts with 9 bits codes

	uint32 prev_code = 0;
	uint32 hash_var = 0;

	for( int i = 0 ; i < size ; i++ )
		if( cur_size == 0 )
			hash_var = prev_code = cur[ cur_size++ ] = data[ i ];
		else {
			cur[ cur_size++ ] = data[ i ];
			hash_var = (hash_var << 5) - hash_var + data[ i ];
			uint32 hash_code = LZW_HASH_LIMIT( hash_var );
			int cur_code = -1;

			for( List< uint32 >::iterator e = dic_hash[ hash_code ].begin() ; e != dic_hash[ hash_code ].end() ; e++ )
				if( dic_length[ *e ] == cur_size && compare( cur, dic_data[ *e ], cur_size ) )
					{	cur_code = *e;	break;	}

			if( cur_code == -1 ) {
				// Emit prev_code
				flow.put_code( prev_code );
				if( dic_size < LZW_DIC_SIZE ) {
					dic_data[ dic_size ] = new byte[ cur_size ];
					memcpy( dic_data[ dic_size ], cur, cur_size );
					dic_length[ dic_size ] = cur_size;
					dic_hash[ hash_code ].push_back( dic_size++ );
					if( !(dic_size & (dic_size - 1) ) )			// We need more bits to store the codes
						flow.set_size( flow.s + 1 );
					}
				else {
					for( int i = 0 ; i < LZW_DIC_SIZE ; i++ )
						dic_hash[ i ].clear();
					for( int i = 258 ; i < LZW_DIC_SIZE ; i++ )
						delete[] dic_data[ i ];
					dic_size = 258;
					// Emit LZW_CODE_CLEAR
					flow.put_code( LZW_CODE_CLEAR );
					flow.set_size( 9 );					// Back to 9bits codes
					}
				hash_var = cur[ 0 ] = cur_code = data[ i ];
				cur_size = 1;
				}
			prev_code = cur_code;
			}

	// Emit prev_code
	flow.put_code( prev_code );
	// Emit LZW_CODE_EOF
	flow.put_code( LZW_CODE_EOF );

	((uint32*)buf)[0] = flow.pos + 7 >> 3;		// Store the compressed data length at the beginning of the buffer
	((uint32*)buf)[1] = size;					// Store the decompressed data length

	for( int i = 0 ; i < dic_size ; i++ )
		if( dic_data[ i ] )
			delete[] dic_data[ i ];

	for( int i = 0 ; i < LZW_DIC_SIZE ; i++ )
		dic_hash[ i ].clear();

	delete[] cur;

	return buf;
}

byte *LZW_decompress( byte *data, int *p_size )
{
	byte			*dic_data[ LZW_DIC_SIZE ];
	int				dic_length[ LZW_DIC_SIZE ];
	int				dic_size = 258;
	int				size = ((uint32*)data)[1];		// Size of decompressed buffer

	if( p_size )	*p_size = size;

	for( int i = 256 ; i < LZW_DIC_SIZE ; i++ )
		dic_data[i] = NULL;

	for( int i = 0 ; i < 256 ; i++ ) {
		dic_length[ i ] = 1;
		dic_data[ i ] = new byte[1];
		dic_data[ i ][0] = i;
		}

	byte	*buf = new byte[ size ];
	int		buf_pos = 0;

	BYTE_FLOW	flow( data + 8 );			// Starts with 9 bits codes

	int prev_code = -1;
	int code;

	while( ( code = flow.get_code() ) != LZW_CODE_EOF && buf_pos < size ) {
		if( code < 0 || code > dic_size ) {			// Should never happen
			for( int i = 0 ; i < dic_size ; i++ )
				if( dic_data[ i ] )
					delete[] dic_data[ i ];
			delete[] buf;
			return NULL;
			}

		if( code == LZW_CODE_CLEAR ) {
			for( int i = 258 ; i < LZW_DIC_SIZE ; i++ )
				if( dic_data[ i ] ) {
					delete[] dic_data[ i ];
					dic_data[ i ] = NULL;
					}
			dic_size = 258;
			flow.set_size( 9 );
			prev_code = -1;
			continue;
			}

		if( prev_code != -1 && dic_size < LZW_DIC_SIZE ) {
			dic_length[ dic_size ] = dic_length[ prev_code ] + 1;
			dic_data[ dic_size ] = new byte[ dic_length[ dic_size ] ];
			memcpy( dic_data[ dic_size ], dic_data[ prev_code ], dic_length[ prev_code ] );

			if( code < dic_size )		// Normal case
				dic_data[ dic_size ][ dic_length[ dic_size ] - 1 ] = dic_data[ code ][ 0 ];
			else						// Special case : you use a string that is not yet in memory !!
				dic_data[ dic_size ][ dic_length[ dic_size ] - 1 ] = dic_data[ dic_size ][ 0 ];
			dic_size++;
			if( !((dic_size+1) & dic_size ) )			// We need more bits to store the codes
				flow.set_size( flow.s + 1 );
			}

		memcpy( buf + buf_pos, dic_data[ code ], dic_length[ code ] );
		buf_pos += dic_length[ code ];

		prev_code = code;
		}

	for( int i = 0 ; i < dic_size ; i++ )
		if( dic_data[ i ] )
			delete[] dic_data[ i ];

	return buf;
}
