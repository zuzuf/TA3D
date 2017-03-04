#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include <misc/paths.h>
#include "textures.h"

namespace TA3D
{



	TEXTURE_MANAGER	texture_manager;

	TEXTURE_MANAGER::TEXTURE_MANAGER() : nbtex(0), tex(NULL)
	{
	}



	void TEXTURE_MANAGER::init()
	{
		tex_hashtable.clear();
		nbtex = 0;
		tex = NULL;
	}

	void TEXTURE_MANAGER::destroy()
	{
		DELETE_ARRAY(tex);
		init();
	}



	int TEXTURE_MANAGER::get_texture_index(const QString& texture_name)
	{
		if (nbtex == 0)
			return -1;
        return tex_hashtable[ texture_name.toUpper() ] - 1;
	}


    GfxTexture::Ptr TEXTURE_MANAGER::get_gl_texture(const QString& texture_name, const int frame)
	{
		int index = get_texture_index(texture_name);
        return (index == -1) ? GfxTexture::Ptr() : tex[index].glbmp[frame];
	}


	QImage TEXTURE_MANAGER::get_bmp_texture(const QString& texture_name, const int frame)
	{
		int index = get_texture_index(texture_name);
        return (index == -1) ? QImage() : tex[index].bmp[frame];
	}


	int TEXTURE_MANAGER::all_texture()
	{
		// Crée des textures correspondant aux couleurs de la palette de TA
		nbtex = 256;
		tex = new Gaf::Animation[nbtex];
		for (int i = 0; i < 256; ++i)
		{
			tex[i].nb_bmp = 1;
            tex[i].bmp.resize(1, QImage());
            tex[i].glbmp.resize(1, GfxTexture::Ptr());
			tex[i].ofs_x.resize(1, 0);
			tex[i].ofs_y.resize(1, 0);
			tex[i].w.resize(1, 0);
			tex[i].h.resize(1, 0);
            tex[i].name = QString("_%1").arg(i);

			tex[i].ofs_x[0] = 0;
			tex[i].ofs_y[0] = 0;
			tex[i].w[0] = 16;
			tex[i].h[0] = 16;
			tex[i].bmp[0] = gfx->create_surface_ex(32,16,16);
            tex[i].bmp[0].fill(pal.at(i));

            tex_hashtable[tex[i].name] = i + 1;
		}

		{
			QStringList file_list;
            VFS::Instance()->getFilelist("textures/*.gaf", file_list);
            for (const QString &cur_file : file_list)
			{
                QIODevice *file = VFS::Instance()->readFile(cur_file);
                const QString filename = Paths::ExtractFileName(cur_file).toUpper();
				load_gaf(file, filename == "LOGOS.GAF" || filename == "LOGOS");
				delete file;
			}
		}
		{
			QStringList file_list;
            VFS::Instance()->getDirlist("textures/*", file_list);
            for (const QString &cur_file : file_list)
			{
                const QString filename = Paths::ExtractFileName(cur_file).toUpper();
                load_gaf(cur_file, filename == "LOGOS.GAF" || filename == "LOGOS");
			}
		}

		return 0;
	}


    void TEXTURE_MANAGER::load_gaf(QIODevice *file, bool logo)
	{
		sint32 nb_entry = Gaf::RawDataEntriesCount(file);
		int n_nbtex = nbtex + nb_entry;
		Gaf::Animation* n_tex = new Gaf::Animation[n_nbtex];
		for (int i = 0; i < nbtex; ++i)
		{
			n_tex[i] = tex[i];
			tex[i].init();
		}
		DELETE_ARRAY(tex);
		tex = n_tex;
		for (int i = 0; i < nb_entry; ++i)
		{
			tex[nbtex + i].loadGAFFromRawData(file, i, false);
			tex[nbtex + i].logo = logo;
            tex_hashtable[tex[nbtex + i].name.toUpper()] = nbtex + i + 1;
		}
		nbtex += nb_entry;
	}


	void TEXTURE_MANAGER::load_gaf(const QString &filename, bool logo)
	{
		QStringList elts;
        sint32 nb_entry = VFS::Instance()->getDirlist(filename + "/*", elts);
		int n_nbtex = nbtex + nb_entry;
		Gaf::Animation* n_tex = new Gaf::Animation[n_nbtex];
		for (int i = 0; i < nbtex; ++i)
		{
			n_tex[i] = tex[i];
			tex[i].init();
		}
		DELETE_ARRAY(tex);
		tex = n_tex;
		for (int i = 0; i < nb_entry; ++i)
		{
			tex[nbtex + i].loadGAFFromDirectory(filename, Paths::ExtractFileName(elts[i]));
			tex[nbtex + i].logo = logo;
            tex_hashtable[tex[nbtex + i].name.toUpper()] = nbtex + i + 1;
		}
		nbtex += nb_entry;
	}

} // namespace TA3D
