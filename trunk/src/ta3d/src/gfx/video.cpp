#include "video.h"
#include "glfunc.h"
#include <TA3D_NameSpace.h>
#include <vfs/vfs.h>
#include <misc/paths.h>
#include <misc/math.h>
#include <smpeg/smpeg.h>
#include <input/keyboard.h>
#include <input/mouse.h>
#include <yuni/core/io/file/stream.h>

#define LOG_PREFIX_VIDEO "[video] "

using namespace Yuni::Core::IO::File;

namespace TA3D
{
	using namespace TA3D::UTILS;

	GLuint Video::gltex = 0;

	void Video::update(SDL_Surface *img, sint32, sint32, uint32, uint32)
	{
		glBindTexture(GL_TEXTURE_2D, gltex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->w, img->h, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
		gfx->drawtexture(gltex, 0.0f, 0.0f, SCREEN_W - 1.0f, SCREEN_H - 1.0f);
		gfx->flip();
	}

	void Video::play(const String &filename)
	{
		String tmp;
		tmp << TA3D::Paths::Caches << Paths::ExtractFileName(filename) << ".mpg";

		if (!Yuni::Core::IO::File::Exists(tmp))
		{
			File *file = VFS::Instance()->readFile(filename);
			if (file)
			{
				Stream tmp_file;
				LOG_DEBUG(LOG_PREFIX_VIDEO << "Creating temporary file for " << filename << " (" << tmp << ")");

				tmp_file.open(tmp, OpenMode::write);
				if (tmp_file.opened())
				{
					char *buf = new char[10240];
					for(int i = 0 ; i < file->size() ; i += 10240)
					{
						int l = Math::Min(10240, file->size() - i);
						file->read(buf, l);
						tmp_file.write(buf, l);
					}
					delete[] buf;
					tmp_file.flush();
					tmp_file.close();
					# ifdef TA3D_PLATFORM_WINDOWS
					tmp.convertSlashesIntoBackslashes();
					# endif
				}
				else
				{
					LOG_ERROR(LOG_PREFIX_VIDEO << "Impossible to create the temporary file `" << tmp << "`");
					return;
				}
				delete file;
			}
		}

		SMPEG *mpeg;
		SMPEG_Info info;

		// Create the MPEG stream
		mpeg = SMPEG_new(tmp.c_str(), &info, 1);

		if (SMPEG_error(mpeg))
		{
			SMPEG_delete(mpeg);
			LOG_ERROR(LOG_PREFIX_VIDEO << "could not read file '" << filename << "'");
			return;
		}
		SMPEG_enableaudio(mpeg, 1);
		SMPEG_enablevideo(mpeg, 1);
		SMPEG_setvolume(mpeg, 100);

		int w = info.width;
		int h = info.height;
		if (!g_useNonPowerOfTwoTextures)
		{
			w = 1 << Math::Log2(w);
			h = 1 << Math::Log2(h);
			if (w < info.width)
				w <<= 1;
			if (h < info.height)
				h <<= 1;
		}

		SDL_Surface *img = gfx->create_surface(w, h);
		gltex = gfx->create_texture(w, h, FILTER_BILINEAR, true);

		SMPEG_setdisplay(mpeg, img, NULL, update);
		SMPEG_scaleXY(mpeg, img->w, img->h);

		gfx->SetDefState();
		gfx->set_2D_mode();
		// Play it, and wait for playback to complete
		SMPEG_play(mpeg);
		while (SMPEG_status(mpeg) == SMPEG_PLAYING)
		{
			poll_inputs();
			if (keypressed())
				break;
			SDL_Delay(100);
		}
		SMPEG_delete(mpeg);
		gfx->unset_2D_mode();
		clear_keybuf();
		gfx->destroy_texture(gltex);
	}
}
