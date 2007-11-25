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
#include "matrix.h"
#define TA3D_NO_SOUND
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"

using namespace TA3D::UTILS::HPI;

volatile uint32 msec_timer = 0;

CAMERA *game_cam=NULL;

int main(int argc,char *argv[])
{
allegro_init();

set_uformat(U_ASCII);		// Juste histoire d'avoir un affichage correct des textes

if(argc>=2) {
	if(strcasecmp(argv[1],"show")==0) {
		HPIManager=new cHPIHandler("");

		List<String> file_list;
		String ext = argc > 2 ? argv[2] : "";
		HPIManager->GetFilelist(ext,&file_list);
		file_list.sort();
		for(List<String>::iterator cur_file=file_list.begin();cur_file!=file_list.end();cur_file++)
			printf("%s\n",cur_file->c_str());

		delete HPIManager;
		}
	else if(strcasecmp(argv[1],"extract")==0) {
		HPIManager=new cHPIHandler("");

		uint32 file_size32 = 0;
		byte *data = HPIManager->PullFromHPI(argv[2],&file_size32);

		if(data) {
			char *name=argv[2];
			char *f=argv[2];
			while(f[0]) {
				if(f[0]=='\\' || f[0]=='/')
					name=f+1;
				f++;
				}
			FILE *dst = TA3D_OpenFile(name,"wb");
			fwrite(data,file_size32,1,dst);
			fclose(dst);
			free(data);
			}

		delete HPIManager;
		}
	else if(strcasecmp(argv[1],"print")==0) {			// like extract but write on the standard output
		HPIManager=new cHPIHandler("");

		List<String> file_list;
		String ext = argc > 2 ? argv[2] : "";
		HPIManager->GetFilelist(ext,&file_list);
		file_list.sort();

		for(List<String>::iterator cur_file=file_list.begin();cur_file!=file_list.end();cur_file++) {
			uint32 file_size32 = 0;
			byte *data = HPIManager->PullFromHPI( *cur_file, &file_size32 );

			if(data) {
				printf("%s\n",(char*)data);
				free(data);
				}
			}

		delete HPIManager;
		}
	else if(strcasecmp(argv[1],"install")==0) {

#ifdef TA3D_PLATFORM_WINDOWS					// Possible cd-rom path for windows
		int		nb_possible_path = 23;
		char	*possible_path[] = {	"D:\\", "E:\\", "F:\\", "G:\\", "H:\\", "I:\\", "J:\\", "K:\\", "L:\\", "M:\\", "N:\\", "O:\\", "P:\\", "Q:\\", "R:\\", "S:\\", "T:\\",
										"U:\\", "V:\\", "W:\\", "X:\\", "Y:\\", "Z:\\" };
#else											// Possible cd-rom path for other platforms
		int		nb_possible_path = 42;
		char	*possible_path[] = {	"/media/cdrom/", "/media/cdrom0/", "/media/cdrom1/", "/media/cdrom2/", "/media/cdrom3/", "/media/cdrom4/", "/media/cdrom5/",
										"/mnt/cdrom/", "/mnt/cdrom0/", "/mnt/cdrom1/", "/mnt/cdrom2/", "/mnt/cdrom3/", "/mnt/cdrom4/", "/mnt/cdrom5/",
										"/mnt/dvd/", "/mnt/dvd0/", "/mnt/dvd1/", "/mnt/dvd2/", "/mnt/dvd3/", "/mnt/dvd4/", "/mnt/dvd5/",
										"/mnt/dvdrecorder/", "/mnt/dvdrecorder0/", "/mnt/dvdrecorder1/", "/mnt/dvdrecorder2/", "/mnt/dvdrecorder3/", "/mnt/dvdrecorder4/", "/mnt/dvdrecorder5/",
										"/media/dvd/", "/media/dvd0/", "/media/dvd1/", "/media/dvd2/", "/media/dvd3/", "/media/dvd4/", "/media/dvd5/",
										"/media/dvdrecorder/", "/media/dvdrecorder0/", "/media/dvdrecorder1/", "/media/dvdrecorder2/", "/media/dvdrecorder3/", "/media/dvdrecorder4/", "/media/dvdrecorder5/" };
#endif

		allegro_message( "please mount/insert your TA cdrom now" );

		String path_to_TA_cd = "";

		if( argc >= 3 ) {						// Explicit path given
			path_to_TA_cd = argv[2];
			for( int i = 0 ; i < path_to_TA_cd.size() ; i++ )			// Use UNIX like path
				if( path_to_TA_cd[ i ] == '\\' )
					path_to_TA_cd[ i ] = '/';
			if( path_to_TA_cd[ path_to_TA_cd.size() ] != '/' )			// Check it ends with a '\'
				path_to_TA_cd += "/";
			}
		else									// Look for a path where we can find totala3.hpi
			for(int i = 0 ; i < nb_possible_path ; i++ ) {
				path_to_TA_cd = possible_path[ i ];
				if( exists( ( path_to_TA_cd + "totala3.hpi" ).c_str() )
				 && exists( ( path_to_TA_cd + "totala2.hpi" ).c_str() ) )		break;		// We found a path to TA's cd ( in fact to needed files )
				}

		HPIManager=new cHPIHandler( path_to_TA_cd );

		int buf_size = 1024*1024;
		uint32 file_size32 = 0;
		byte *data = HPIManager->PullFromHPI_zone( "install\\totala1.hpi", 0, buf_size, &file_size32);			// Extract the totala1.hpi file from the TA CD
		bool success = true;

		if(data) {
			set_color_depth(32);
			set_gfx_mode( GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0 );
			clear( screen );

			rectfill( screen, 0, 0, 640, 480, 0x7F7F7F );

			textprintf_centre_ex( screen, font, 320, 40, 0x0, -1, "Extracting totala1.hpi ( step 1/2 )" );

			FILE *dst = TA3D_OpenFile("totala1.hpi","wb");

			fwrite(data,buf_size,1,dst);

			for( int pos = buf_size ; pos < file_size32 ; pos += buf_size ) {
				int read_size = min( buf_size, (int)(file_size32-pos) );
				free(data);
				data = HPIManager->PullFromHPI_zone( "install\\totala1.hpi", pos, read_size, &file_size32);			// Extract the totala1.hpi file from the TA CD
				fwrite(data+pos,read_size,1,dst);

				rectfill( screen, 100, 60, 540, 80, makecol( 255, 0, 0 ) );
				rectfill( screen, 100, 60, 100 + 440 * (pos+read_size>>10) / (file_size32>>10), 80, makecol( 255, 255, 0 ) );
				textprintf_centre_ex( screen, font, 320, 66, 0x0, -1, "%d%%", 100 * (pos+read_size>>10) / (file_size32>>10) );
				}

			fclose(dst);
			free(data);
			}
		else
			success = false;

		if( exists( ( path_to_TA_cd + "totala2.hpi" ).c_str() ) && success ) {
			textprintf_centre_ex( screen, font, 320, 140, 0x0, -1, "Copying totala2.hpi ( step 2/2 )" );
			rectfill( screen, 100, 160, 540, 180, makecol( 255, 0, 0 ) );
			textprintf_centre_ex( screen, font, 320, 166, 0x0, -1, "0%%" );

			FILE *src = TA3D_OpenFile( path_to_TA_cd + "totala2.hpi", "rb" );
			FILE *dst = TA3D_OpenFile( "totala2.hpi", "wb" );
			int limit = FILE_SIZE( ( path_to_TA_cd + "totala2.hpi" ).c_str() );
			byte buf[ buf_size ];			// a 1Mo buffer

			for( int pos = 0 ; pos < limit ; pos+= buf_size ) {
				int read_size = min( buf_size, limit-pos );
				fread( buf, read_size, 1, src );
				fwrite( buf, read_size, 1, dst );

				rectfill( screen, 100, 160, 540, 180, makecol( 255, 0, 0 ) );
				rectfill( screen, 100, 160, 100 + 440 * (pos+read_size>>10) / (limit>>10), 180, makecol( 255, 255, 0 ) );
				textprintf_centre_ex( screen, font, 320, 166, 0x0, -1, "%d%%", 100 * (pos+read_size>>10) / (limit>>10) );
				}

			fclose( dst );
			fclose( src );
			}
		else
			success = false;

		if( ! success )							// Print an error message
			allegro_message( "Installation failed:\n    Unable to find TA's cd path!!\n\nplease use this syntax:\n    hpiview install path_to_TA_cd\n                                         " );
		else
			allegro_message( "Installation Successful!!\nNow just run ta3d from its base directory and have fun ;-)!" );

		delete HPIManager;
		}
	}

allegro_exit();

return 0;
}
END_OF_MAIN();

void reset_keyboard()
{
}
void reset_mouse()
{
}
