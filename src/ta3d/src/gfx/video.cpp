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
#include <sounds/manager.h>
#include <threads/mutex.h>

#define LOG_PREFIX_VIDEO "[video] "

using namespace Yuni::Core::IO::File;

namespace TA3D
{
	using namespace TA3D::UTILS;

	GLuint Video::gltex = 0;
	SDL_Surface *Video::buf = NULL;

	Synchronizer mpegSynchronizer(2);

	void Video::update(SDL_Surface *img, sint32, sint32, uint32, uint32)
	{
		// Swap the buffers
		void *tmp = buf->pixels;
		buf->pixels = img->pixels;
		img->pixels = tmp;
		mpegSynchronizer.sync();
	}

	void Video::play(const String &filename)
	{
		SMPEG_Info info;
		SMPEG *mpeg;

		String tmp;
		tmp << TA3D::Paths::Caches << Paths::ExtractFileName(filename) << ".mpg";

		File *file = VFS::Instance()->readFile(filename);
		if (file)
		{
			if (file->isReal())					// Check if the file is real
				tmp = file->getRealFilename();	// if it is we don't need to copy it
			else if (!Yuni::Core::IO::File::Exists(tmp) || Yuni::Core::IO::File::Size(tmp) != (uint32)file->size())
			{
				Stream tmp_file;
				LOG_DEBUG(LOG_PREFIX_VIDEO << "Creating temporary file for " << filename << " (" << tmp << ")");

				tmp_file.open(tmp, Yuni::Core::IO::OpenMode::write);
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
			}
			delete file;
		}
		else
		{
			LOG_ERROR(LOG_PREFIX_VIDEO << "Impossible to read the file `" << filename << "`");
			return;
		}

		const bool audioRunning = sound_manager;
		if (audioRunning)
			sound_manager = NULL;

		// Create the MPEG stream
		mpeg = SMPEG_new(tmp.c_str(), &info, 1);

		if (SMPEG_error(mpeg))
		{
			SMPEG_delete(mpeg);
			Mix_CloseAudio();
			SDL_QuitSubSystem( SDL_INIT_CDROM );
			SDL_QuitSubSystem( SDL_INIT_AUDIO );
			SDL_AudioQuit();
			LOG_ERROR(LOG_PREFIX_VIDEO << "could not read file '" << filename << "'");
			if (audioRunning)
			{
				sound_manager = new Audio::Manager;
				sound_manager->loadTDFSounds(true);
				sound_manager->loadTDFSounds(false);
			}
			return;
		}

		// Play video and audio separately (otherwise they are not synced if you don't use
		// SMPEG's built-in playback which requires letting it control SDL audio system)
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

		gfx->set_texture_format( gfx->defaultTextureFormat_RGB() );
		SDL_Surface *img = gfx->create_surface(w, h);
		buf = gfx->create_surface(w, h);
		gltex = gfx->create_texture(w, h, FILTER_LINEAR, true);

		SMPEG_setdisplay(mpeg, img, NULL, update);
		SMPEG_scaleXY(mpeg, img->w, img->h);

		// Those special 5/4 modes are native 4/3 monitor modes with rectangular pixels
		const float aspectRatio = (SCREEN_W * 4 == SCREEN_H * 5) ? 4.0f / 3.0f : float(SCREEN_W) / float(SCREEN_H);
		const float screenRatio = float(SCREEN_W) / float(SCREEN_H);
		const float movieRatio = float(info.width) / float(info.height);

		gfx->SetDefState();
		gfx->set_2D_mode();
		gfx->clearAll();
		gfx->flip();
		gfx->clearAll();
		// Play it, and wait for playback to complete
		mpegSynchronizer.setNbThreadsToSync(2);
		SMPEG_play(mpeg);
		uint32 timer = msec_timer;
		const float screen_w = static_cast<float>(SCREEN_W);
		const float screen_h = static_cast<float>(SCREEN_H);
		while (SMPEG_status(mpeg) == SMPEG_PLAYING)
		{
			mpegSynchronizer.sync();

			glBindTexture(GL_TEXTURE_2D, gltex);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buf->w, buf->h, GL_RGBA, GL_UNSIGNED_BYTE, buf->pixels);

			if (aspectRatio >= movieRatio)
			{
				const float vw = movieRatio * screenRatio / aspectRatio * screen_h;
				gfx->drawtexture(gltex, 0.5f * (screen_w - vw), 0.0f, 0.5f * (screen_w + vw), screen_h);
			}
			else
			{
				const float vh = aspectRatio / (movieRatio * screenRatio) * screen_w;
				gfx->drawtexture(gltex, 0.0f, 0.5f * (screen_h - vh), screen_w, 0.5f * (screen_h + vh));
			}
			gfx->flip();

			if (msec_timer - timer > 100)
			{
				poll_inputs();
				if (keypressed() || mouse_b != 0)
					break;
				timer = msec_timer;
			}
		}
		mpegSynchronizer.setNbThreadsToSync(0);
		mpegSynchronizer.release();
		SMPEG_delete(mpeg);
		gfx->unset_2D_mode();
		clear_keybuf();
		gfx->destroy_texture(gltex);
		SDL_FreeSurface(img);
		SDL_FreeSurface(buf);

		Mix_CloseAudio();
		SDL_QuitSubSystem( SDL_INIT_CDROM );
		SDL_QuitSubSystem( SDL_INIT_AUDIO );
		SDL_AudioQuit();
		if (audioRunning)
		{
			sound_manager = new Audio::Manager;
			sound_manager->loadTDFSounds(true);
			sound_manager->loadTDFSounds(false);
		}
	}
}
