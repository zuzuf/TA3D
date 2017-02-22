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

/*-----------------------------------------------------------------------------------\
  |                                         gaf.cpp                                    |
  |  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
  | des fichiers gaf de total annihilation qui sont les fichiers contenant les images  |
  | et les animations du jeu.                                                          |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "gaf.h"
#include <vector>
#include "gfx/glfunc.h"
#include <zlib.h>
#include <QIODevice>
#include <misc/paths.h>

#define READ(X) file->read((char*)&X, sizeof(X))

namespace TA3D
{

	namespace
	{

		/*!
		 * \brief Get the name of a GAF entry
		 */
        void convertGAFCharToString(QIODevice* file, QString& out)
		{
			char dt[33];
			memset(dt, 0, 33);
			file->read(dt, 32);
			dt[32] = '\0';
            out = QString::fromUtf8(dt, (uint32)strlen(dt));
		}

	}

    Gaf::Header::Header(QIODevice* file)
	{
		file->seek(0);
        READ(IDVersion);
        READ(Entries);
        READ(Unknown1);
	}

	Gaf::Frame::Data::Data()
		:Width(0), Height(0), XPos(0), YPos(0), Transparency(0), Compressed(0),
		FramePointers(0), Unknown2(0), PtrFrameData(0), Unknown3(0)
	{}

    Gaf::Frame::Data::Data(QIODevice* file)
	{
		LOG_ASSERT(file != NULL);
        READ(Width);
        READ(Height);
        READ(XPos);
        READ(YPos);
        READ(Transparency);
        READ(Compressed);
        READ(FramePointers);
        READ(Unknown2);
        READ(PtrFrameData);
        READ(Unknown3);
	}


    void Gaf::ToTexturesList(std::vector<GLuint>& out, const QString& filename, const QString& imgname,
							 int* w, int* h, const bool truecolor, const int filter)
	{
		out.clear();

        QIODevice* file = VFS::Instance()->readFile(filename);		// Try to open it as a file
		if (file)
		{
			sint32 idx = RawDataGetEntryIndex(file, imgname);
			if (idx == -1)
			{
				delete file;
				return;
			}

			sint32 nb_img = RawDataImageCount(file, idx);
			if (nb_img <= 0)
			{
				delete file;
				return;
			}

			out.resize(nb_img);

			int indx(0);
			for (std::vector<GLuint>::iterator i = out.begin(); i != out.end(); ++i, ++indx)
			{
				uint32 fw, fh;
                const QString &cache_filename = filename + "-" + imgname + QString("-%1.bin").arg(indx);
				*i = gfx->load_texture_from_cache(cache_filename, filter, &fw, &fh);

				if (!(*i))
				{
					SDL_Surface* img = Gaf::RawDataToBitmap(file, idx, indx, NULL, NULL, truecolor);

					if (!img)
					{
						delete file;
						return;
					}

					if (w) w[indx] = img->w;
					if (h) h[indx] = img->h;

					bool with_alpha = false;
					for (int y = 0; y < img->h && !with_alpha; ++y)
						for (int x = 0; x < img->w && !with_alpha; ++x)
							with_alpha |= geta( SurfaceInt(img, x, y) ) != 255;
					if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
						gfx->set_texture_format(with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB);
					else
						gfx->set_texture_format(with_alpha ? gfx->defaultTextureFormat_RGBA() : gfx->defaultTextureFormat_RGB());

					*i = gfx->make_texture(img, filter);
					gfx->save_texture_to_cache(cache_filename, *i, img->w, img->h, with_alpha);
					SDL_FreeSurface(img);
				}
				else
				{
					if (w) w[indx] = fw;
					if (h) h[indx] = fh;
				}
			}
			delete file;
			return;
		}
		// Now try to open it as a GAF-like directory
        QStringList folderList;
        VFS::Instance()->getFilelist(filename + '/' + imgname + "/*", folderList);
        if (!folderList.isEmpty())			// So this is a directory with a GAF-like tree structure
		{
            std::sort(folderList.begin(), folderList.end());
            int k = -1;
            for(const QString &i : folderList)
			{
                ++k;
				uint32 width, height;
				out.push_back(gfx->load_texture(*i, filter, &width, &height));
				if (w)
					w[k] = width;
				if (h)
					h[k] = height;
			}
		}
	}



    GLuint Gaf::ToTexture(QString filename, const QString& imgname, int* w, int* h, const bool truecolor, const int filter)
	{
		// Remove GAF extension
        if (filename.endsWith(".gaf"))
            filename.chop(4);

        const QString &cache_filename = filename + "-" + imgname + ".bin";
		uint32 fw;
		uint32 fh;
		GLuint first_try = gfx->load_texture_from_cache(cache_filename, filter, &fw, &fh);

		if (first_try)
		{
			if (w)  *w = fw;
			if (h)  *h = fh;
			return first_try;
		}

		// Now try to open it as a GAF-like directory
        QStringList folderList;
        VFS::Instance()->getFilelist(filename + '/' + imgname + "/*", folderList);
        if (!folderList.isEmpty())			// So this is a directory with a GAF-like tree structure
		{
            std::sort(folderList.begin(), folderList.end());
			return gfx->load_texture(folderList.front(), filter, (uint32*)w, (uint32*)h);
		}

		// Add GAF extension
        filename += ".gaf";

        QIODevice *file = VFS::Instance()->readFile(filename);			// Try to open it as file
		if (file)
		{
			sint32 idx = file->size() > 0 ? RawDataGetEntryIndex(file, imgname) : -1;
			if (idx != -1)
			{
				SDL_Surface *img = Gaf::RawDataToBitmap(file, idx, 0, NULL, NULL, truecolor);
				if (img)
				{
					if (w) *w = img->w;
					if (h) *h = img->h;
					bool with_alpha = false;

					for (int y = 0; y < img->h && !with_alpha; ++y)
					{
						for (int x = 0; x < img->w && !with_alpha; ++x)
							with_alpha |= SurfaceByte(img, (x << 2) + 3, y) != 255;
					}
					if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
						gfx->set_texture_format(with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB);
					else
						gfx->set_texture_format(with_alpha ? gfx->defaultTextureFormat_RGBA() : gfx->defaultTextureFormat_RGB());

					GLuint gl_img = gfx->make_texture(img,filter);
					gfx->save_texture_to_cache(cache_filename, gl_img, img->w, img->h, with_alpha);

					SDL_FreeSurface(img);
					delete file;
					return gl_img;
				}
			}
			delete file;
			return 0;
		}
		return 0;
	}



    QString Gaf::RawDataGetEntryName(QIODevice *file, int entry_idx)
	{
		LOG_ASSERT(file != NULL);
		if (entry_idx < 0)
			return nullptr;
		Gaf::Header header(file);
		if (entry_idx >= header.Entries)
			return nullptr;

		sint32 *pointers = new sint32[header.Entries];

		file->read(pointers, header.Entries * (int)sizeof(sint32));

		Gaf::Entry entry;
		file->seek(pointers[entry_idx]);
        READ(entry.Frames);
        READ(entry.Unknown1);
        READ(entry.Unknown2);
        convertGAFCharToString(file, entry.name);

		DELETE_ARRAY(pointers);
		return entry.name;
	}


    sint32 Gaf::RawDataGetEntryIndex(QIODevice *file, const QString& name)
	{
		LOG_ASSERT(file != NULL);
		sint32 nb_entry = RawDataEntriesCount(file);
        const QString &cmpQString = name.toUpper();
		for (int i = 0; i < nb_entry; ++i)
            if (Gaf::RawDataGetEntryName(file, i).toUpper() == cmpQString)
				return i;
		return -1;
	}



    sint32 Gaf::RawDataImageCount(QIODevice *file, const int entry_idx)
	{
		LOG_ASSERT(file != NULL);
		if (entry_idx < 0)
			return 0;
		Gaf::Header header(file);
		if (entry_idx >= header.Entries)		// Si le fichier contient moins d'images que img_idx, il y a erreur
			return 0;

		sint32* pointers = new sint32[header.Entries];
		file->read(pointers, header.Entries * (int)sizeof(sint32));

		Gaf::Entry entry;
		file->seek(pointers[entry_idx]);
        READ(entry.Frames);
        READ(entry.Unknown1);
        READ(entry.Unknown2);
        convertGAFCharToString(file, entry.name);

		DELETE_ARRAY(pointers);
		return entry.Frames;
	}



    SDL_Surface* Gaf::RawDataToBitmap(QIODevice* file, const sint32 entry_idx, const sint32 img_idx, short* ofs_x, short* ofs_y, const bool truecolor)
	{
		LOG_ASSERT(file != NULL);
		if (entry_idx < 0 || img_idx < 0)
			return NULL;
		Gaf::Header header(file);
		if (entry_idx >= header.Entries) // Si le fichier contient moins d'images que img_idx, il y a erreur
			return NULL;

		sint32 *pointers = new sint32[header.Entries];
		file->read(pointers, header.Entries * (int)sizeof(sint32));

		Gaf::Entry entry;
		file->seek(pointers[entry_idx]);
        READ(entry.Frames);
        READ(entry.Unknown1);
        READ(entry.Unknown2);
        convertGAFCharToString(file, entry.name);

		if (entry.Frames <= 0 || img_idx >= entry.Frames)
		{
			DELETE_ARRAY(pointers);
			return NULL;
		}

		Gaf::Frame::Entry* frame = new Gaf::Frame::Entry[entry.Frames];

		for (sint32 i = 0; i < entry.Frames; ++i)
		{
            READ(frame[i].PtrFrameTable);
            READ(frame[i].Unknown1);
		}

		SDL_Surface *frame_img = NULL;

		try
		{
			if (frame[img_idx].PtrFrameTable < 0)
			{
				DELETE_ARRAY(pointers);
				DELETE_ARRAY(frame);
				return NULL;
			}
			file->seek(frame[img_idx].PtrFrameTable);
			Gaf::Frame::Data framedata(file);
			uint32 frames = framedata.PtrFrameData;

			if (ofs_x)
				*ofs_x = framedata.XPos;
			if (ofs_y)
				*ofs_y = framedata.YPos;

			sint32 nb_subframe = framedata.FramePointers;
			sint32 frame_x = framedata.XPos;
			sint32 frame_y = framedata.YPos;
			sint32 frame_w = framedata.Width;
			sint32 frame_h = framedata.Height;

			if (header.IDVersion == TA3D_GAF_TRUECOLOR)
			{
				file->seek(framedata.PtrFrameData);
				sint32 img_size = 0;
                READ(img_size);
                file->read((char*)&img_size, sizeof(img_size));
				char *buf = new char[img_size];
				file->read(buf, img_size);

				frame_img = gfx->create_surface_ex( framedata.Transparency ? 32 : 24, framedata.Width, framedata.Height );
				uLongf len = frame_img->w * frame_img->h * frame_img->format->BytesPerPixel;
				uncompress ( (Bytef*) frame_img->pixels, &len, (Bytef*) buf, img_size);
				delete[] buf;
			}
			else
			{
				for (int subframe = 0; subframe < nb_subframe || subframe < 1 ; ++subframe)
				{
					if (nb_subframe)
					{
						uint32 f_pos;
						file->seek(frames + 4 * subframe);
                        READ(f_pos);
						file->seek(f_pos);

                        READ(framedata.Width);
                        READ(framedata.Height);
                        READ(framedata.XPos);
                        READ(framedata.YPos);

                        READ(framedata.Transparency);
                        READ(framedata.Compressed);
                        READ(framedata.FramePointers);
                        READ(framedata.Unknown2);
                        READ(framedata.PtrFrameData);
                        READ(framedata.Unknown3);
					}

					SDL_Surface *img = NULL;

					if (framedata.Compressed) // Si l'image est comprimée
					{
						LOG_ASSERT(framedata.Width  >= 0 && framedata.Width  < 4096);
						LOG_ASSERT(framedata.Height >= 0 && framedata.Height < 4096);
						if (!truecolor)
						{
							img = gfx->create_surface_ex(8, framedata.Width, framedata.Height);
							SDL_FillRect(img, NULL, 0);
						}
						else
						{
							img = gfx->create_surface_ex(32, framedata.Width, framedata.Height);
							SDL_FillRect(img, NULL, 0);
						}

						sint16 length;
						file->seek(framedata.PtrFrameData);
						for (int i = 0; i < img->h; ++i) // Décode les lignes les unes après les autres
						{
                            READ(length);
							int x(0);
							int e(0);
							do
							{
								const byte mask = (byte)file->getc();
								++e;
								if (mask & 0x01)
								{
									if (!truecolor)
										x += mask >> 1;
									else
									{
										int l = mask >> 1;
										while (l > 0 && x < img->w)
										{
											SurfaceInt(img, x++, i) = 0x00000000U;
											--l;
										}
									}
								}
								else
								{
									if (mask & 0x02)
									{
										int l = (mask >> 2) + 1;
										const byte c = (byte)file->getc();
										const uint32 c32 = makeacol32(pal[c].r, pal[c].g, pal[c].b,0xFF);
										while (l > 0 && x < img->w)
										{
											if (!truecolor)
											{
												SurfaceByte(img, x, i) = c;
												x++;
											}
											else
												SurfaceInt(img, x++, i) = c32;
											--l;
										}
										++e;
									}
									else
									{
										int l = (mask >> 2) + 1;
										while (l > 0 && x < img->w)
										{
											if (truecolor)
											{
												const byte c = (byte)file->getc();
												SurfaceInt(img, x++, i) = makeacol32(pal[c].r, pal[c].g, pal[c].b, 0xFF);
											}
											else
											{
												SurfaceByte(img, x, i) = byte(file->getc());
												x++;
											}
											++e;
											--l;
										}
									}
								}
							} while (e < length && x < img->w);
							file->seek(file->tell() + length - e);
						}
					}
					else
					{
						// Si l'image n'est pas comprimée
						img = gfx->create_surface_ex(8, framedata.Width, framedata.Height);
						SDL_FillRect(img, NULL, 0);

						file->seek(framedata.PtrFrameData);
						SDL_LockSurface(img);
						for (int i = 0; i < img->h; ++i) // Copie les octets de l'image
							file->read(((char*)(img->pixels)) + i * img->pitch, img->w);
						SDL_UnlockSurface(img);

						if (truecolor)
						{
							SDL_Surface *tmp = convert_format_copy(img);
							for (int y = 0 ; y < tmp->h; ++y)
							{
								for (int x = 0; x < tmp->w; ++x)
								{
									if (SurfaceByte(img, x, y) == framedata.Transparency)
										SurfaceInt(tmp, x, y) = 0x00000000;
									else
										SurfaceInt(tmp, x, y) |= makeacol(0,0,0, 0xFF);
								}
							}
							SDL_FreeSurface(img);
							img = tmp;
						}
					}

					if (nb_subframe == 0)
						frame_img = img;
					else
					{
						if (subframe == 0)
						{
							if (!truecolor)
							{
								frame_img = gfx->create_surface_ex(8,frame_w,frame_h);
								SDL_FillRect(frame_img, NULL, 0);
							}
							else
							{
								frame_img = gfx->create_surface_ex(32,frame_w,frame_h);
								SDL_FillRect(frame_img, NULL, 0);
							}
							blit(img, frame_img, 0, 0, frame_x - framedata.XPos, frame_y - framedata.YPos, img->w, img->h);
						}
						else
						{
							if (truecolor)
							{
								for (int y = 0; y < img->h; ++y)
								{
									int Y = y + frame_y - framedata.YPos;
									if (Y < 0 || Y >= frame_img->h)
										continue;
									int X = frame_x - framedata.XPos;
									for (int x = 0; x < img->w; ++x)
									{
										if (X >= 0 && X < frame_img->w)
										{
											uint32 c = SurfaceInt(frame_img, X, Y);
											int r = getr(c);
											int g = getg(c);
											int b = getb(c);
											int a = geta(c);

											c = SurfaceInt(img, x, y);
											int r2 = getr(c);
											int g2 = getg(c);
											int b2 = getb(c);
											int a2 = geta(c);

											r = (r * (255 - a2) + r2 * a2) >> 8;
											g = (g * (255 - g2) + g2 * a2) >> 8;
											b = (b * (255 - b2) + b2 * a2) >> 8;

											SurfaceInt(frame_img, X, Y) = makeacol(r, g, b, a);
										}
										++X;
									}
								}
							}
							else
								masked_blit(img, frame_img, 0, 0, frame_x - framedata.XPos, frame_y - framedata.YPos, img->w, img->h );
						}
						SDL_FreeSurface(img);
					}
				}
			}
		}
		catch(...)
		{
			LOG_ERROR("GAF data corrupt!");
			DELETE_ARRAY(pointers);
			DELETE_ARRAY(frame);
			return NULL;
		};

		DELETE_ARRAY(pointers);
		DELETE_ARRAY(frame);
		return frame_img;
	}



    void Gaf::Animation::loadGAFFromRawData(QIODevice *file, const int entry_idx, const bool truecolor, const QString& fname)
	{
		LOG_ASSERT(file != NULL);
		if (entry_idx < 0 || !file)
			return;
		filename = fname;

		nb_bmp = Gaf::RawDataImageCount(file,entry_idx);

		bmp.resize(nb_bmp, NULL);
		glbmp.resize(nb_bmp, 0);
		ofs_x.resize(nb_bmp, 0);
		ofs_y.resize(nb_bmp, 0);
		w.resize(nb_bmp, 0);
		h.resize(nb_bmp, 0);
		name  = Gaf::RawDataGetEntryName(file, entry_idx);
		pAnimationConverted = false;

		int i(0);
		int f(0);
		for (; i < nb_bmp; ++i)
		{
			if ((bmp[i-f] = Gaf::RawDataToBitmap(file, entry_idx, i, &(ofs_x[i-f]), &(ofs_y[i-f]), truecolor)) != NULL)
			{
				w[i-f] = (short)bmp[i-f]->w;
				h[i-f] = (short)bmp[i-f]->h;
				if (!truecolor)
					bmp[i-f] = convert_format(bmp[i-f]);
			}
			else
				++f;
		}
		nb_bmp -= f;
		bmp.resize(nb_bmp);
		glbmp.resize(nb_bmp);
		ofs_x.resize(nb_bmp);
		ofs_y.resize(nb_bmp);
		w.resize(nb_bmp);
		h.resize(nb_bmp);
	}

    void Gaf::Animation::loadGAFFromDirectory(const QString &folderName, const QString &entryName)
	{
		filename = folderName;

        QStringList files;
        VFS::Instance()->getFilelist(folderName + '/' + entryName + "/*", files);
        std::sort(files.begin(), files.end());

		nb_bmp = (sint32)files.size();

		bmp.resize(nb_bmp, NULL);
		glbmp.resize(nb_bmp, 0);
		ofs_x.resize(nb_bmp, 0);
		ofs_y.resize(nb_bmp, 0);
		w.resize(nb_bmp, 0);
		h.resize(nb_bmp, 0);
		name = entryName;
		pAnimationConverted = false;

		int i(0);
		int f(0);
		for (; i < nb_bmp; ++i)
		{
			if ((bmp[i-f] = gfx->load_image(files[i])) != NULL)
			{
				w[i-f] = (short)bmp[i-f]->w;
				h[i-f] = (short)bmp[i-f]->h;
				ofs_x[i-f] = 0;
				ofs_y[i-f] = 0;
			}
			else
				++f;
		}
		nb_bmp -= f;
		bmp.resize(nb_bmp);
		glbmp.resize(nb_bmp);
		ofs_x.resize(nb_bmp);
		ofs_y.resize(nb_bmp);
		w.resize(nb_bmp);
		h.resize(nb_bmp);
	}


	void Gaf::Animation::init()
	{
		logo = false;
		nb_bmp = 0;
		bmp.clear();
		ofs_x.clear();
		ofs_y.clear();
		glbmp.clear();
		w.clear();
		h.clear();
		pAnimationConverted = false;
		filename.clear();
		name.clear();
	}


	void Gaf::Animation::destroy()
	{
		filename.clear();
		name.clear();
		for (std::vector<SDL_Surface*>::iterator i = bmp.begin() ; i != bmp.end() ; ++i)
			if (*i)
				SDL_FreeSurface(*i);
		for (std::vector<GLuint>::iterator i = glbmp.begin() ; i != glbmp.end() ; ++i)
			gfx->destroy_texture(*i);
		w.clear();
		h.clear();
		ofs_x.clear();
		ofs_y.clear();
		bmp.clear();
		glbmp.clear();
		init();
	}

	void Gaf::Animation::clean()
	{
		for (std::vector<SDL_Surface*>::iterator i = bmp.begin() ; i != bmp.end() ; ++i)
			if (*i)
			{
				SDL_FreeSurface(*i);
				*i = NULL;
			}
		name.clear();
	}


	void Gaf::Animation::convert(bool NO_FILTER, bool COMPRESSED)
	{
		if (pAnimationConverted)
			return;
		pAnimationConverted = true;
		for (int i = 0; i < nb_bmp; ++i)
		{
            const QString &cache_filename = filename + '-' + (name.isEmpty() ? "none" : name) + QString("-%1.bin").arg(i);

            if (!filename.isEmpty())
				glbmp[i] = gfx->load_texture_from_cache(cache_filename, NO_FILTER ? FILTER_NONE : FILTER_TRILINEAR );
			else
				glbmp[i] = 0;

			if (!glbmp[i])
			{
				bmp[i] = convert_format(bmp[i]);
				if (g_useTextureCompression && COMPRESSED && lp_CONFIG->use_texture_compression)
					gfx->set_texture_format(GL_COMPRESSED_RGBA_ARB);
				else
					gfx->set_texture_format(gfx->defaultTextureFormat_RGBA());
				glbmp[i] = gfx->make_texture(bmp[i], NO_FILTER ? FILTER_NONE : FILTER_TRILINEAR );
                if (!filename.isEmpty())
					gfx->save_texture_to_cache(cache_filename, glbmp[i], bmp[i]->w, bmp[i]->h, true);
			}
		}
	}

	void Gaf::AnimationList::clear()
	{
		pList.clear();
	}

	Gaf::AnimationList::~AnimationList()
	{
		pList.clear();
	}


    sint32 Gaf::AnimationList::loadGAFFromRawData(QIODevice *file, const bool doConvert, const QString& fname)
	{
		if (file)
		{
			pList.clear();
			pList.resize(Gaf::RawDataEntriesCount(file));
			for (uint32 i = 0 ; i < pList.size() ; ++i)
				pList[i].loadGAFFromRawData(file, i, true, fname);
			if (doConvert)
				convert();
			return (sint32)pList.size();
		}
		return 0;
	}

    sint32 Gaf::AnimationList::loadGAFFromDirectory(const QString &folderName, const bool doConvert)
	{
        QStringList entries;
        VFS::Instance()->getDirlist(folderName + "/*", entries);
		pList.clear();
		pList.resize(entries.size());
		for (uint32 i = 0 ; i < pList.size() ; ++i)
			pList[i].loadGAFFromDirectory(folderName, Paths::ExtractFileName(entries[i]));
		if (doConvert)
			convert();
		return (sint32)pList.size();
	}

    sint32 Gaf::AnimationList::findByName(const QString& name) const
	{
		for (uint32 i = 0 ; i < pList.size() ; ++i)
		{
			if (name == pList[i].name)
				return i;
		}
		return -1;
	}


	void Gaf::AnimationList::clean()
	{
		for (AnimationVector::iterator i = pList.begin() ; i != pList.end() ; ++i)
			i->clean();
	}


	void Gaf::AnimationList::convert(const bool no_filter, const bool compressed)
	{
		for (AnimationVector::iterator i = pList.begin() ; i != pList.end() ; ++i)
			i->convert(no_filter, compressed);
	}


	sint32 Gaf::RawDataEntriesCount(File* file)
	{
		file->seek(4);
		sint32 v;
		file->read(v);
		return v;
	}


} // namespace TA3D

