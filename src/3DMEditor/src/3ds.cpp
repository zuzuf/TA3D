#include "config.h"
#include "types.h"
#include "3ds.h"
#include "mesh.h"
#include "gfx.h"
#include <stdlib.h>
#include <stdio.h>
#include <QDebug>

struct TA3D_3DS_CHUNK_DATA
{
    uint16	ID;
    uint32	length;
};

class TA3D_3DS_MATERIAL
{
public:
    QString	NAME;
    float	AMBIENT[4];
    float	DIFFUSE[4];
    float	SPECULAR[4];
    float	SHININESS;
    float	SHIN2PCT;
    float	SHIN3PCT;
    float	TRANSPARENCY;
    bool	TWO_SIDE;
    float	TEXMAP;
    QString	MAPNAME;
    TA3D_3DS_MATERIAL	*next;

    TA3D_3DS_MATERIAL()
    {
        next = NULL;
        NAME.clear();
        MAPNAME.clear();
    }

    ~TA3D_3DS_MATERIAL()
    {
        delete next;
        NAME.clear();
        MAPNAME.clear();
    }

    TA3D_3DS_MATERIAL	*find( const QString &name )
    {
        if( !NAME.isEmpty() && NAME.compare( name, Qt::CaseInsensitive ) == 0 )	return this;
        if( next != NULL )
            return next->find( name );
        return NULL;
    }
};

QString read_ASCIIZ( QFile &src)
{
    char name[1000];
    int i = 0;
	unsigned char c = 0;
	while (src.getChar((char*)&c) && c != 0)
        name[ i++ ] = c;
    name[ i ] = 0;
    return QString(name);
}

void read_color_chunk( float color[], QFile &src )
{
    TA3D_3DS_CHUNK_DATA chunk;
	src.read( (char*)&chunk.ID, 2 );
	src.read( (char*)&chunk.length, 4 );
    switch( chunk.ID )
    {
    case COL_RGB:
    case COL_RGB2:
		src.read( (char*)color, sizeof( float ) * 3 );
        break;
    case COL_TRU:
    case COL_TRU2:
        for( int i = 0 ; i < 3 ;i++ )
		{
			unsigned char c;
			src.getChar((char*)&c);
			color[ i ] = c / 255.0f;
		}
        break;
    default:
		src.read( chunk.length - 6 );
    };
}

float read_percent_chunk( QFile &src )
{
    TA3D_3DS_CHUNK_DATA chunk;
	src.read( (char*)&chunk.ID, 2 );
	src.read( (char*)&chunk.length, 4 );
    switch( chunk.ID )
    {
    case PER_INT:
        {
            uint16	percent;
			src.read( (char*)&percent, 2 );
            return percent;
        }
    case PER_FLOAT:
        {
            float	percent;
			src.read( (char*)&percent, 4 );
            return percent;
        }
    default:
		src.read( chunk.length - 6 );
    };
    return 0.0f;
}

QString read_MAT_MAPNAME_chunk( QFile &src )
{
    TA3D_3DS_CHUNK_DATA chunk;
	src.read( (char*)&chunk.ID, 2 );
	src.read( (char*)&chunk.length, 4 );
    switch( chunk.ID )
    {
    case MAT_MAPNAME:
        return read_ASCIIZ( src );
    default:
		src.read( chunk.length - 6 );
    };
    return NULL;
}

void Mesh::load3DS(const QString &filename, float scale)
{
	QFile src_3ds(filename);
	src_3ds.open(QFile::ReadOnly);
	if (src_3ds.isOpen())
    {
        destroy();
        QString filepath = filename;
        if (filepath.lastIndexOf("/") != -1)
            filepath = filepath.mid(0, filepath.lastIndexOf("/") + 1);
        else if (filepath.lastIndexOf("\\") != -1)
            filepath = filepath.mid(0, filepath.lastIndexOf("\\") + 1);
        TA3D_3DS_CHUNK_DATA	chunk;
        Mesh *cur_obj = NULL;
        Mesh *read_obj = NULL;
        TA3D_3DS_MATERIAL	*material = NULL;
        Vector3D local[4];
		while (src_3ds.read( (char*)&chunk.ID, sizeof( chunk.ID ) ))
        {
			if (src_3ds.read( (char*)&chunk.length, sizeof( chunk.length ) ) == 0)
				break;
			switch (chunk.ID)
            {
            case MAIN3DS:
                //				printf("MAIN3DS (%d,%d)\n", chunk.ID, chunk.length);
                break;
            case EDIT3DS:
                //					printf("-EDIT3DS (%d,%d)\n", chunk.ID, chunk.length);
                break;
            case EDIT_MATERIAL:
                //						printf("--EDIT_MATERIAL (%d,%d)\n", chunk.ID, chunk.length);
                {
                    TA3D_3DS_MATERIAL	*new_mat = new TA3D_3DS_MATERIAL;
                    new_mat->next = material;
                    material = new_mat;
                }
                break;
            case MAT_NAME:
                //							printf("---MAT_NAME (%d,%d)\n", chunk.ID, chunk.length);
                {
                    QString name = read_ASCIIZ( src_3ds );
                    //								printf( "name = %s\n", name );
					if (material)
                        material->NAME = name;
                }
                break;
            case MAT_AMBIENT:
                //							printf("---MAT_AMBIENT (%d,%d)\n", chunk.ID, chunk.length);
				if (material)
                    read_color_chunk( material->AMBIENT, src_3ds );
                else
					src_3ds.read( chunk.length - 6 );
                break;
            case MAT_DIFFUSE:
                //							printf("---MAT_DIFFUSE (%d,%d)\n", chunk.ID, chunk.length);
				if (material)
                    read_color_chunk( material->DIFFUSE, src_3ds );
                else
					src_3ds.read( chunk.length - 6 );
                break;
            case MAT_SPECULAR:
                //							printf("---MAT_SPECULAR (%d,%d)\n", chunk.ID, chunk.length);
				if (material)
                    read_color_chunk( material->SPECULAR, src_3ds );
                else
					src_3ds.read( chunk.length - 6 );
                break;
            case MAT_SHININESS:
                //							printf("---MAT_SHININESS (%d,%d)\n", chunk.ID, chunk.length);
				if (material)
                    material->SHININESS = read_percent_chunk( src_3ds );
                else
					src_3ds.read( chunk.length - 6 );
                break;
            case MAT_SHIN2PCT:
                //							printf("---MAT_SHIN2PCT (%d,%d)\n", chunk.ID, chunk.length);
				if (material)
                    material->SHIN2PCT = read_percent_chunk( src_3ds );
                else
					src_3ds.read( chunk.length - 6 );
                break;
            case MAT_SHIN3PCT:
                //							printf("---MAT_SHIN3PCT (%d,%d)\n", chunk.ID, chunk.length);
				if (material)
                    material->SHIN3PCT = read_percent_chunk( src_3ds );
                else
					src_3ds.read( chunk.length - 6 );
                break;
            case MAT_TRANSPARENCY:
                //							printf("---MAT_TRANSPARENCY (%d,%d)\n", chunk.ID, chunk.length);
				if (material)
                    material->TRANSPARENCY = read_percent_chunk( src_3ds );
                else
					src_3ds.read( chunk.length - 6 );
                break;
            case MAT_TWO_SIDE:
                //							printf("---MAT_TWO_SIDE (%d,%d)\n", chunk.ID, chunk.length);
				if (material)
                    material->TWO_SIDE = true;
				src_3ds.read( chunk.length - 6 );
                break;
            case MAT_TEXMAP:
                //							printf("---MAT_TEXMAP (%d,%d)\n", chunk.ID, chunk.length);
                if (material)
                {
                    uint16 n_id;
					src_3ds.peek( (char*)&n_id, 2 );
                    if (n_id == MAT_MAPNAME)
                    {
                        material->MAPNAME = read_MAT_MAPNAME_chunk( src_3ds );
                        material->TEXMAP = read_percent_chunk( src_3ds );
                    }
                    else
                    {
                        material->TEXMAP = read_percent_chunk( src_3ds );
                        material->MAPNAME = read_MAT_MAPNAME_chunk( src_3ds );
                    }
                    material->MAPNAME = filepath + material->MAPNAME;
                }
                else
					src_3ds.read( chunk.length - 6 );
                break;
            case EDIT_CONFIG1:
                //						printf("--EDIT_CONFIG1 (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case EDIT_CONFIG2:
                //						printf("--EDIT_CONFIG2 (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case EDIT_VIEW_P1:
                //						printf("--EDIT_VIEW_P1 (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case EDIT_VIEW_P2:
                //						printf("--EDIT_VIEW_P2 (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case EDIT_VIEW_P3:
                //						printf("--EDIT_VIEW_P2 (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case EDIT_VIEW1:
                //						printf("--EDIT_VIEW1 (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case EDIT_BACKGR:
                //						printf("--EDIT_BACKGR (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case EDIT_AMBIENT:
                //						printf("--EDIT_AMBIENT (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case EDIT_OBJECT:
                //						printf("--EDIT_OBJECT (%d,%d)\n", chunk.ID, chunk.length);
                if (cur_obj)
                {
                    Mesh *n_obj = new Mesh;
                    cur_obj->next = n_obj;
                    cur_obj = n_obj;
                }
                else
                    cur_obj = this;
                cur_obj->type = MESH_TRIANGLES;
                cur_obj->name = read_ASCIIZ( src_3ds );		// Read the object's name
                read_obj = cur_obj;
                break;
            case OBJ_LIGHT:
                //							printf("---OBJ_LIGHT (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case OBJ_CAMERA:
                //							printf("---OBJ_CAMERA (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case OBJ_UNKNWN01:
                //							printf("---OBJ_UNKNWN01 (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case OBJ_UNKNWN02:
                //							printf("---OBJ_UNKNWN02 (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case OBJ_TRIMESH:
                //							printf("---OBJ_TRIMESH (%d,%d)\n", chunk.ID, chunk.length);
                if (read_obj->vertex.size() > 0)		// Add a sub object
                {
                    read_obj->child = new Mesh;
                    read_obj = read_obj->child;
                    read_obj->type = MESH_TRIANGLES;
                    read_obj->name = cur_obj->name;
                }
                local[0].x = 1.0f;		local[0].y = 0.0f;		local[0].z = 0.0f;
                local[1].x = 0.0f;		local[1].y = 1.0f;		local[1].z = 0.0f;
                local[2].x = 0.0f;		local[2].y = 0.0f;		local[2].z = 1.0f;
                local[3].x = 0.0f;		local[3].y = 0.0f;		local[3].z = 0.0f;
                break;
            case TRI_VERTEXL:
                {
                    //								printf("----TRI_VERTEXL (%d,%d)\n", chunk.ID, chunk.length);
                    uint16 nb_vtx;
					src_3ds.read( (char*)&nb_vtx, 2 );
                    read_obj->vertex.resize( nb_vtx );
                    read_obj->normal.resize( nb_vtx );
                    if (read_obj->tcoord.isEmpty())
                    {
                        read_obj->tcoord.resize( nb_vtx * 2 );
                        for( int i = 0 ; i < read_obj->vertex.size() ; i++ )
                        {
                            read_obj->tcoord[ i << 1 ] = 0.0f;
                            read_obj->tcoord[ (i << 1) + 1 ] = 0.0f;
                        }
                    }
					src_3ds.read( (char*)read_obj->vertex.data(), sizeof( Vector3D ) * nb_vtx );
                    for( int i = 0 ; i < read_obj->vertex.size() ; i++ )
                    {
                        read_obj->vertex[ i ] = scale * (read_obj->vertex[ i ].x * local[ 0 ] + read_obj->vertex[ i ].y * local[ 1 ] + read_obj->vertex[ i ].z * local[ 2 ] + local[ 3 ]);
                        read_obj->normal[ i ] = Vec();
                    }
                }
                break;
            case TRI_FACEL2:
                //								printf("----TRI_FACEL2 (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case TRI_MATERIAL:
                //								printf("----TRI_MATERIAL (%d,%d)\n", chunk.ID, chunk.length);
                {
                    QString material_name = read_ASCIIZ( src_3ds );
                    //									printf("material name = %s\n", material_name );

                    TA3D_3DS_MATERIAL	*cur_mat = (material != NULL) ? material->find( material_name ) : NULL ;

                    if (cur_mat)
                    {
                        //										printf("material found\n");
                        read_obj->flag |= SURFACE_ADVANCED | SURFACE_LIGHTED;
                        if (!cur_mat->MAPNAME.isEmpty())
                        {
                            //											printf("loading texture %s\n", cur_mat->MAPNAME );
                            read_obj->flag |= SURFACE_TEXTURED;
                            read_obj->tex.resize(1);
                            read_obj->tex[0] = Gfx::instance()->loadTexture( cur_mat->MAPNAME.trimmed() );
                        }
                        if (cur_mat->TRANSPARENCY > 0.0f)
                        {
                            read_obj->flag |= SURFACE_BLENDED;
                            read_obj->color = (read_obj->color & 0xFFFFFF00) | (uint32)(cur_mat->TRANSPARENCY * 255);
                        }
                    }
                    else
                        printf("WARNING: material not found!!\n");

                    uint16	nb_faces;
					src_3ds.read( (char*)&nb_faces, 2 );
                    for( int i = 0 ; i < nb_faces ; i++ )
                    {
                        uint16 cur_face;
						src_3ds.read( (char*)&cur_face, 2 );
                    }
                }
                break;
            case TRI_MAPPING:
                //								printf("----TRI_MAPPING (%d,%d)\n", chunk.ID, chunk.length);
                {
                    uint16	nb_vtx;
					src_3ds.read( (char*)&nb_vtx, 2 );
                    read_obj->tcoord.resize(2 * nb_vtx);
					src_3ds.read( (char*)read_obj->tcoord.data(), 2 * sizeof( float ) * nb_vtx );
                    for( int i = 0 ; i < nb_vtx ; i++ )
                        read_obj->tcoord[ i * 2 + 1 ] = 1.0f - read_obj->tcoord[ i * 2 + 1 ];
                }
                break;
            case TRI_FACEL1:
                //								printf("----TRI_FACEL1 (%d,%d)\n", chunk.ID, chunk.length);
                uint16 nb_index;
				src_3ds.read( (char*)&nb_index, 2 );
                read_obj->index.resize(nb_index * 3);
                for( int i = 0 ; i < nb_index * 3 ; i += 3 )
                {
                    uint16 idx[3];
					src_3ds.read( (char*)idx, 2 * 3 );
                    read_obj->index[i] = idx[0];
                    read_obj->index[i+1] = idx[1];
                    read_obj->index[i+2] = idx[2];
                    if (!read_obj->vertex.isEmpty())
                    {
                        Vector3D AB,AC;
                        AB = read_obj->vertex[ read_obj->index[ i + 1 ] ] - read_obj->vertex[ read_obj->index[ i ] ];
                        AC = read_obj->vertex[ read_obj->index[ i + 2 ] ] - read_obj->vertex[ read_obj->index[ i ] ];
                        AB = AB ^ AC;
                        AB.unit();
                        read_obj->normal[ read_obj->index[ i ] ] = read_obj->normal[ read_obj->index[ i ] ] + AB;
                        read_obj->normal[ read_obj->index[ i + 1 ] ] = read_obj->normal[ read_obj->index[ i + 1 ] ] + AB;
                        read_obj->normal[ read_obj->index[ i + 2 ] ] = read_obj->normal[ read_obj->index[ i + 2 ] ] + AB;
                    }
                    uint16 face_info;
					src_3ds.read( (char*)&face_info, 2 );
                }
                if (!read_obj->vertex.isEmpty())
                    for( int i = 0 ; i < read_obj->normal.size() ; i++ )
                        read_obj->normal[ i ].unit();
                break;
            case TRI_SMOOTH:
                //								printf("----TRI_SMOOTH (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            case TRI_LOCAL:
                //								printf("----TRI_LOCAL (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( (char*)&(local[0]), sizeof( Vector3D ) );		// X
				src_3ds.read( (char*)&(local[1]), sizeof( Vector3D ) );		// Y
				src_3ds.read( (char*)&(local[2]), sizeof( Vector3D ) );		// Z
				src_3ds.read( (char*)&(local[3]), sizeof( Vector3D ) );		// local origin
                break;
            case TRI_VISIBLE:
                //								printf("----TRI_VISIBLE (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
                break;
            default:
                //				printf("unknown (%d,%d)\n", chunk.ID, chunk.length);
				src_3ds.read( chunk.length - 6 );
            }
        }
        if (material)
            delete material;
		src_3ds.close();

        computeInfo();
        emit loaded();
    }
}
