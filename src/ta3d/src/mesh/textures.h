#ifndef __TA3D_TEXTURES_H__
#define __TA3D_TEXTURES_H__

# include <misc/string.h>
# include <misc/hash_table.h>
# include <gaf.h>

namespace TA3D
{
    //	Classe pour la gestion des textures du jeu
    class TEXTURE_MANAGER
    {
    public:
        int	 nbtex;			// Nombre de textures
        Gaf::Animation* tex;			// Textures
		HashMap<int>::Dense tex_hashtable;  // To speed up texture search

		TEXTURE_MANAGER();
        ~TEXTURE_MANAGER() {destroy();}

        void init();

        void destroy();


        int get_texture_index(const String& texture_name);

        GLuint get_gl_texture(const String& texture_name, const int frame = 0);

        SDL_Surface *get_bmp_texture(const String& texture_name, const int frame = 0);

		int load_gaf(File *data, bool logo);

        int all_texture();

    }; // class TEXTURE_MANAGER



    extern TEXTURE_MANAGER	texture_manager;
} // namespace TA3D

#endif
