#include "../stdafx.h"
#include "../TA3D_NameSpace.h"
#include "../misc/paths.h"
#include "textures.h"

namespace TA3D
{
    TEXTURE_MANAGER	texture_manager;

    void TEXTURE_MANAGER::init()
    {
        tex_hashtable.emptyHashTable();
        tex_hashtable.initTable(__DEFAULT_HASH_TABLE_SIZE);
        nbtex = 0;
        tex = NULL;
    }

    void TEXTURE_MANAGER::destroy()
    {
        if (tex)
            delete[] tex;
        init();
    }



    int TEXTURE_MANAGER::get_texture_index(const String& texture_name)
    {
        if (nbtex == 0)
            return -1;
        return tex_hashtable.find( texture_name ) - 1;
    }


    GLuint TEXTURE_MANAGER::get_gl_texture(const String& texture_name, const int frame)
    {
        int index = get_texture_index(texture_name);
        return (index == -1) ? 0 : tex[index].glbmp[frame];
    }


    SDL_Surface* TEXTURE_MANAGER::get_bmp_texture(const String& texture_name, const int frame)
    {
        int index = get_texture_index(texture_name);
        return (index== -1) ? NULL : tex[index].bmp[frame];
    }


    int TEXTURE_MANAGER::all_texture()
    {
        // Crée des textures correspondant aux couleurs de la palette de TA
        nbtex = 256;
        tex = new Gaf::Animation[nbtex];
        for (int i = 0; i < 256; ++i)
        {
            tex[i].nb_bmp = 1;
            tex[i].bmp = new SDL_Surface*[1];
            tex[i].glbmp = new GLuint[1];
            tex[i].ofs_x = new short[1];
            tex[i].ofs_y = new short[1];
            tex[i].w = new short[1];
            tex[i].h = new short[1];
            tex[i].name = String::Format("_%d", i);

            tex[i].ofs_x[0] = 0;
            tex[i].ofs_y[0] = 0;
            tex[i].w[0] = 16;
            tex[i].h[0] = 16;
            tex[i].bmp[0] = gfx->create_surface_ex(32,16,16);
            SDL_FillRect(tex[i].bmp[0], NULL, makeacol(pal[i].r, pal[i].g, pal[i].b, 0xFF));

            tex_hashtable.insert(tex[i].name,i + 1);
        }

        String::List file_list;
        VFS::instance()->getFilelist("textures\\*.gaf", file_list);
        for (String::List::const_iterator cur_file = file_list.begin(); cur_file != file_list.end(); ++cur_file)
        {
            byte *data = VFS::instance()->readFile(*cur_file);
            load_gaf(data, String::ToUpper(Paths::ExtractFileName(*cur_file)) == "LOGOS.GAF");
            delete[] data;
        }
        return 0;
    }


    int TEXTURE_MANAGER::load_gaf(byte* data, bool logo)
    {
        sint32 nb_entry = Gaf::RawDataEntriesCount(data);
        int n_nbtex = nbtex + nb_entry;
        Gaf::Animation* n_tex = new Gaf::Animation[n_nbtex];
        for (int i = 0; i < nbtex; ++i)
        {
            n_tex[i] = tex[i];
            tex[i].init();
        }
        if (tex)
            delete[] tex;
        tex = n_tex;
        for (int i = 0; i < nb_entry; ++i)
        {
            tex[nbtex + i].loadGAFFromRawData(data, i, false);
            tex[nbtex + i].logo = logo;
            tex_hashtable.insert(tex[nbtex + i].name, nbtex + i + 1);
        }
        nbtex += nb_entry;
        return 0;
    }
}