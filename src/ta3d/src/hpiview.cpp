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


// Disabled the Sound System
# define TA3D_NO_SOUND

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include <iostream>
#include "tnt.h"
#include "logs/logs.h"
#include "misc/paths.h"
#include "misc/resources.h"
#include <zlib.h>
#include "EngineClass.h"
#include "misc/string.h"
#include "mods/mods.h"
#include "mesh/mesh.h"
#include "mesh/textures.h"
#include "misc/material.light.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include <QFile>

using namespace TA3D;

# ifdef TA3D_PLATFORM_WINDOWS
#   define PREFIX "  /"
# else
#   define PREFIX "  --"
# endif

static QString appName;

void configWindow();

namespace TA3D
{



	void install_TA_files( QString HPI_file, QString filename );

	static bool hpiviewCmdHelp(QStringList& /*args*/)
	{
		std::cout << "Available commands :" << std::endl
			<< PREFIX << "config         : failsafe config GUI" << std::endl
			<< PREFIX << "create_gaf     : create a 24/32bits gaf from sprites" << std::endl
			<< PREFIX << "create_buildpic: create a build picture from a model and a background image" << std::endl
			<< PREFIX << "extract        : extract a file" << std::endl
			<< PREFIX << "extract_gaf    : extract a gaf into sprites" << std::endl
			<< PREFIX << "help           : this screen" << std::endl
			<< PREFIX << "install        : install TA files" << std::endl
			<< PREFIX << "listfiles      : list all archives used by the VFS" << std::endl
			<< PREFIX << "listmods       : list all available mods" << std::endl
			<< PREFIX << "mapdescription : extract a map description" << std::endl
			<< PREFIX << "minimap        : extract a minimap" << std::endl
			<< PREFIX << "print          : show the content of a file" << std::endl
			<< PREFIX << "show           : show files matching a pattern" << std::endl
			<< PREFIX << "quiet          : runs in quiet mode (no logs)" << std::endl
			<< std::endl
			<< "For more information on a command type :" << std::endl
            << "# " << appName.toStdString() << " command_name" << std::endl;
		return true;
	}


	/*!
	 * \brief
	 */
	static bool hpiviewCmdShow(QStringList &args)
	{
		if (args.size() >= 1 )
		{
			QStringList file_list;
			QString ext = args[0];
			VFS::Instance()->getFilelist(ext, file_list);
            std::sort(file_list.begin(), file_list.end());
            for (const QString &cur_file : file_list)
                std::cout << cur_file.toStdString() << std::endl;

			args.erase(args.begin());
			return true;
		}
        std::cerr << "SYNTAX: " << appName.toStdString() << " show <pattern>" << std::endl;
		return true;
	}

	/*!
	 * \brief
	 */
	static bool hpiviewCmdListFiles(QStringList &/*args*/)
	{
		QStringList lArchives;

		VFS::Instance()->getArchivelist(lArchives);
        std::sort(lArchives.begin(), lArchives.end());
        for (const QString &it : lArchives)
            std::cout << it.toStdString() << std::endl;

		return true;
	}

	/*!
	 * \brief Extract a mini map from a .tnt file
	 */
	static bool hpiviewCmdMiniMap(QStringList &args)
	{
		if (args.size() >= 2)
		{
			TA3D::UTILS::load_palette(pal);

			QImage minimap = load_tnt_minimap_fast_bmp( args[0] );
            if (!minimap.isNull())
                minimap.save(args[1]);

			args.erase(args.begin());
			args.erase(args.begin());

			return true;
		}
        std::cerr << "SYNTAX: " << appName.toStdString() << " minimap \"maps/map.tnt\" minimap_file.jpg" << std::endl;
		return true;
	}


	/*!
	 * \brief
	 */
	static bool hpiviewCmdMapDescription(QStringList &args)
	{
		if (args.size() >= 2)
		{
			MAP_OTA map_data;
			map_data.load( args[0] );
            QFile m_FileD(args[1]);
            m_FileD.open(QIODevice::WriteOnly);

            if (m_FileD.isOpen())
			{
                QTextStream m_File(&m_FileD);
				m_File << "[MAP]\n{\n";
				m_File << " missionname=" << map_data.missionname << ";\n";
				QString tmp(map_data.missiondescription);
                tmp.replace("\n", "\\n");
				m_File << " missiondescription=" << tmp << ";\n";
				m_File << " planet=" << map_data.planet << ";\n";
				m_File << " tidalstrength=" << map_data.tidalstrength << ";\n";
				m_File << " solarstrength=" << map_data.solarstrength << ";\n";
				m_File << " gravity=" << map_data.gravity << ";\n";
				tmp = map_data.numplayers;
				tmp.replace("\n", "\\n");
				m_File << " numplayers=" << tmp << ";\n";
				tmp = map_data.map_size;
				tmp.replace("\n", "\\n");
				m_File << " size=" << tmp << ";\n";
				m_File << " minwindspeed=" << map_data.minwindspeed << ";\n";
				m_File << " maxwindspeed=" << map_data.maxwindspeed << ";\n";
				m_File << "}\n";
                m_File.flush();
                m_FileD.close();
			}
			args.erase(args.begin());
			args.erase(args.begin());
			return true;
		}
        std::cerr << "SYNTAX: " << appName.toStdString() << " mapdescription \"maps/map.ota\" description.tdf" << std::endl;
		return true;
	}


	/*!
	 * \brief
	 */
	static bool hpiviewCmdListMods(QStringList &args)
	{
		if (args.size() >= 1)
		{
            QFile m_FileD(args[0]);
            m_FileD.open(QIODevice::WriteOnly);

            if (m_FileD.isOpen())
			{
                QTextStream m_File(&m_FileD);
				QStringList modlist = Mods::instance()->getModNameList(Mods::MOD_INSTALLED);
                std::sort(modlist.begin(), modlist.end());
                for (const QString &i : modlist)
				{
                    if (!(i.isEmpty()) && (i[0] != '.'))
                        m_File << i << "\n";
				}
                m_File.flush();
                m_FileD.close();
			}
			args.erase(args.begin());
			return true;
		}
        LOG_ERROR("Syntax: " << appName << " listmods modlist.txt");
		return true;
	}



	/*!
	 * \brief
	 */
	static bool hpiviewCmdExtract(QStringList &args)
	{
		if (args.size() >= 1)
		{
            QIODevice *file = VFS::Instance()->readFile(args[0]);

			if (file)
			{
                const QString &name = Paths::ExtractFileName(args[0]);
                QFile dst(name);
                dst.open(QIODevice::WriteOnly);
                while(file->bytesAvailable())
                    dst.write(file->read(10240));
				dst.close();
				delete file;
			}
			args.erase(args.begin());
			return true;
		}
        std::cerr << "SYNTAX: " << appName.toStdString() << " extract <filename>" << std::endl;
		return true;
	}


	/*!
	 * \brief
	 */
	static bool hpiviewCmdPrint(QStringList &args)
	{
		if (args.size() >= 1)
		{
			QStringList file_list;
			QString ext = args[0];
			VFS::Instance()->getFilelist(ext, file_list);
            std::sort(file_list.begin(), file_list.end());

            for (const QString &cur_file : file_list)
			{
                QIODevice* file = VFS::Instance()->readFile(cur_file);
				if (file)
				{
                    while(file->bytesAvailable())
                    {
                        const QByteArray &buffer = file->read(10240);
                        std::cout.write(buffer.data(), buffer.size());
                    }
                    std::cout << std::endl;
					delete file;
				}
				else
                    LOG_ERROR("could not open file '" << cur_file << "'");
			}
			args.erase(args.begin());
			return true;
		}
        std::cerr << "SYNTAX: " << appName.toStdString() << " print pattern" << std::endl;
		return true;
	}


	/*!
	 * \brief
	 */
	static bool hpiviewCmdInstallTAFiles(QStringList &args)
	{
		if (args.size() >= 2)
		{
			install_TA_files(args[0], args[1]);
			args.erase(args.begin());
			args.erase(args.begin());
		}
		else
            std::cerr << "SYNTAX: " << appName.toStdString() << " install HPI_file file_in_HPI" << std::endl;
		return true;
	}


	/*!
	 * \brief Extract images from a GAF file
	 */
	static bool hpiviewCmdExtractGAF(QStringList &args)
	{
		if (args.size() >= 1)
		{
            QIODevice *file = VFS::Instance()->readFile(args[0]);

			if (file)
			{
				SDL_SetVideoMode(320, 200, 32, 0);
				TA3D::UTILS::load_palette(pal);

				Gaf::AnimationList anims;
				anims.loadGAFFromRawData(file);
                QFile m_FileD( Paths::ExtractFileName(args[0]) + ".txt");
                m_FileD.open(QIODevice::WriteOnly);
                QTextStream m_File(&m_FileD);

				m_File << "[gadget0]\n{\n";
				m_File << "    filename=" << args[0] << ";\n";
				m_File << "    entries=" << anims.size() << ";\n";
				m_File << "}\n";

				for (sint32 i = 0; i < anims.size(); ++i)
				{
					m_File << "[gadget" << (i + 1) << "]\n{\n";
					m_File << "    frames=" << anims[i].nb_bmp << ";\n";
					m_File << "    name=" << anims[i].name << ";\n";
					for (int e = 0; e < anims[i].nb_bmp; ++e)
					{
                        QString filename = anims[i].name;
						if (e < 10)
                            filename += "000";
						else if (e < 100)
                            filename += "00";
						else if (e < 1000)
                            filename += '0';
                        filename += QString("%1.png").arg(e);
						m_File << "    [frame" << e << "]\n    {\n";
						m_File << "        XPos=" << anims[i].ofs_x[ e ] << ";\n";
						m_File << "        YPos=" << anims[i].ofs_y[ e ] << ";\n";
						m_File << "        filename=" << filename << ";\n";
						m_File << "    }\n";
						save_bitmap( filename, anims[i].bmp[e] );
					}
					m_File << "}\n";
				}

                m_File.flush();
                m_FileD.flush();
                m_FileD.close();
				delete file;
			}
			args.erase(args.begin());
			return true;
		}
        std::cerr << "SYNTAX: " << appName.toStdString() << " extract_gaf <filename.gaf>" << std::endl;
		return true;
	}




	/*!
	 * \brief Create a truecolor GAF from images
	 */
	static bool hpiviewCmdCreateGAF(QStringList &args)
	{
		if (args.size() >= 1)
		{
			TDFParser parser( args[0], false, false, true, true );
			QString filename = parser.pullAsString( "gadget0.filename" );
            QFile gaf_file( Paths::ExtractFileName( filename ) );
            gaf_file.open(QIODevice::WriteOnly);

			LOG_DEBUG("opening '" << filename << "'");

			disable_TA_palette();

            if (gaf_file.isOpen())
			{
				SDL_SetVideoMode(320, 200, 32, 0);
				Gaf::Header header;
				header.IDVersion = TA3D_GAF_TRUECOLOR;
				header.Entries   = parser.pullAsInt("gadget0.entries");
				header.Unknown1  = 0;

				gaf_file.write((const char*)&header, 12);

				for (int i = 0; i < header.Entries * 4; ++i)
                    gaf_file.putChar(0);

				for (int i = 0; i < header.Entries; ++i)
				{
                    int pos = int(gaf_file.pos());
                    gaf_file.seek( 12 + i * 4 );
					gaf_file.write( (const char*)&pos, 4 );
                    gaf_file.seek( pos );

					Gaf::Entry Entry;

                    Entry.Frames = sint16(parser.pullAsInt( QString("gadget%1.frames").arg(i + 1)  ));
					Entry.Unknown1 = 1;
					Entry.Unknown2 = 0;
                    Entry.name = parser.pullAsString( QString("gadget%1.name").arg(i + 1) );

					gaf_file.write( (const char*)&Entry.Frames, 2 );
					gaf_file.write( (const char*)&Entry.Unknown1, 2 );
					gaf_file.write( (const char*)&Entry.Unknown2, 4 );
					char tmp[32];
					memset(tmp, 0, 32);
                    memcpy(tmp, Entry.name.toStdString().c_str(), Math::Min((int)Entry.name.size(), 32));
					tmp[31] = 0;
					gaf_file.write((const char*)tmp, 32);

					Gaf::Frame::Entry FrameEntry;
                    int FrameEntryPos = int(gaf_file.pos());
					FrameEntry.PtrFrameTable = 0;
					for (int e = 0; e < Entry.Frames; ++e)
						gaf_file.write( (const char*)&(FrameEntry), 8 );

					for (int e = 0; e < Entry.Frames; ++e)
					{
                        pos = int(gaf_file.pos());
                        gaf_file.seek( FrameEntryPos + e * 8 );
						FrameEntry.PtrFrameTable = pos;
						gaf_file.write( (const char*)&(FrameEntry), 8 );
                        gaf_file.seek( pos );

						Gaf::Frame::Data FrameData;
                        FrameData.XPos = sint16(parser.pullAsInt( QString("gadget%1.frame%2.XPos").arg(i+1).arg(e) ) );
                        FrameData.YPos = sint16(parser.pullAsInt( QString("gadget%1.frame%2.YPos").arg(i+1).arg(e) ) );
						FrameData.FramePointers = 0;
						FrameData.Unknown2 = 0;
						FrameData.Compressed = 1;

                        QImage frame_img( parser.pullAsString( QString("gadget%1.frame%2.filename").arg(i + 1).arg(e)) );
                        if (!frame_img.isNull())
						{
                            convert_format(frame_img);
                            FrameData.Width = sint16(frame_img.width());
                            FrameData.Height = sint16(frame_img.height());
							bool alpha = false;
                            for( int y = 0 ; y < frame_img.height() && !alpha ; y++ )
                                for( int x = 0 ; x < frame_img.width() && !alpha ; x++ )
									alpha |= (getr(SurfaceInt(frame_img, x, y)) != 255);
							FrameData.Transparency = alpha ? 1 : 0;
                            FrameData.PtrFrameData = sint32(gaf_file.pos()) + 24;

							gaf_file.write( (const char*)&FrameData, 24 );

                            int buf_size = frame_img.width() * frame_img.height() * 5 + 10240;
							byte *buffer = new byte[ buf_size ];

							int img_size = buf_size;
							uLongf __size = img_size;
                            compress2 ( buffer, &__size, (Bytef*) frame_img.bits(), frame_img.byteCount(), 9);
							img_size = int(__size);

							gaf_file.write( (const char*)&img_size, sizeof( img_size ) );		// Save the result
							gaf_file.write( (const char*)buffer, img_size );

							DELETE_ARRAY(buffer);
						}
						else
						{
							std::cerr << "Error: In frame " << e << ", could not load "
                                      << parser.pullAsString(QString("gadget%1.frame%2.filename").arg(i + 1).arg(e) ).toStdString() << std::endl;
							i = header.Entries;
							break;
						}
					}
				}
			}
			else
				std::cerr << "Error: Could not create file!" << std::endl;
			enable_TA_palette();
			args.erase(args.begin());
		}
		else
            std::cerr << "SYNTAX: " << appName.toStdString() << " create_gaf gafdescription.txt" << std::endl;
		return true;
	}

	/*!
	 * \brief Create a truecolor build picture for the given unit with the given background
	 */
	static bool hpiviewCmdCreateBuildPic(QStringList &args)
	{
		if (args.size() >= 2)
		{
            const QString &modelname = args[0];
            const QString &filename = args[1];
            const QString &outputfilename = Paths::ExtractFileNameWithoutExtension(modelname) + ".tga";

			// Starts a minimal TA3D environnement
			InterfaceManager = new IInterfaceManager();

			// Initalizing SDL video
			if (SDL_Init(SDL_INIT_VIDEO) < 0 )
                throw std::runtime_error("SDL_Init(SDL_INIT_VIDEO) yielded unexpected result." );

			// Installing SDL timer
			if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0 )
                throw std::runtime_error( "SDL_InitSubSystem(SDL_INIT_TIMER) yielded unexpected result." );

			// Installing SDL timer
			if (SDL_InitSubSystem(SDL_INIT_EVENTTHREAD) != 0 )
                throw std::runtime_error( "SDL_InitSubSystem(SDL_INIT_EVENTTHREAD) yielded unexpected result." );

			gfx = new GFX();

			init_keyboard();

			// Now we can load the model and its textures, set up a basic camera and render the model
			if (Paths::ExtractFileExt(modelname).toLower() == ".3do")		// Load textures
				texture_manager.all_texture();

			Model *model = MeshTypeManager::load(modelname);
			QImage background = gfx->load_image(filename);
            if (model && !background.isNull())
			{
                glViewport(0, 0, background.width(), background.height());           // Use picture viewport
                gfx->width = background.width();
                gfx->height = background.height();

				Camera cam;
				HWLight sun;

				sun.Att = 0.0f;
				// Direction
				sun.Dir.x = -1.0f;
				sun.Dir.y = 2.0f;
				sun.Dir.z = 1.0f;
				sun.Dir.unit();
				// Lights
				sun.LightAmbient[0]  = 0.25f;
				sun.LightAmbient[1]  = 0.25f;
				sun.LightAmbient[2]  = 0.25f;
				sun.LightAmbient[3]  = 0.25f;
				sun.LightDiffuse[0]  = 1.0f;
				sun.LightDiffuse[1]  = 1.0f;
				sun.LightDiffuse[2]  = 1.0f;
				sun.LightDiffuse[3]  = 1.0f;
				sun.LightSpecular[0] = 0.0f;
				sun.LightSpecular[1] = 0.0f;
				sun.LightSpecular[2] = 0.0f;
				sun.LightSpecular[3] = 0.0f;
				// Direction
				sun.Directionnal = true;

				glEnable(GL_COLOR_MATERIAL);
				lp_CONFIG->shadow_quality = 0;
				lp_CONFIG->disable_GLSL = true;

				gfx->SetDefState();
				gfx->clearAll();

				gfx->set_2D_mode();
				GLuint tex = gfx->make_texture(background);
                gfx->drawtexture(tex, 0.0f, 0.0f, (float)background.width(), (float)background.height(), 0xFFFFFFFF);
				gfx->destroy_texture(tex);
				gfx->unset_2D_mode();

                cam.setWidthFactor(background.width(), background.height());
				float h = model->top - model->bottom;
				cam.rpos = Vector3D(0.0f, model->bottom + h * 0.5f, 1.5f * model->size2);
				cam.znear = 0.001f;
				cam.zfar = 1000.0f;
				cam.dir = Vector3D(0.0f, 0.0f, -1.0f);
				sun.Enable();
				sun.Set(cam);
				cam.setView();

				glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
				glRotatef(-25.0f, 0.0f, 1.0f, 0.0f);

				model->hideFlares();
				model->draw(0.0f);

                QImage result = gfx->create_surface_ex(24, background.width(), background.height());
                glReadPixels(0, 0, result.width(), result.height(), GL_BGR, GL_UNSIGNED_BYTE, result.bits());
				vflip_bitmap(result);
				save_bitmap(outputfilename, result);

				gfx->flip();
			}
			else
			{
				if (!model)
                    std::cerr << "error : could not load model file : '" << modelname.toStdString() << "'" << std::endl;
				else
					delete model;
                if (background.isNull())
                    std::cerr << "error : could not load background image file : '" << filename.toStdString() << "'" << std::endl;
            }

			// Clean everything
			texture_manager.destroy();

			delete gfx;
			gfx = NULL;

			InterfaceManager = NULL;

			args.erase(args.begin());
			args.erase(args.begin());
		}
		else
            std::cerr << "SYNTAX: " << appName.toStdString() << " create_buildpic model.3do/3dm/3so background_image_file" << std::endl;
		return true;
	}

	/*!
	 * \brief
	 */
	int hpiview(int argc, char *argv[])
	{
		SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO);

		if (argc >= 2)
		{
			VFS::Instance()->reload();
			appName = argv[0];

			// TODO Use a better implementation to parse arguments
			QStringList args;
			for (int i = 1 ; i < argc ; i++)
				args.push_back(argv[i]);
			bool ok = false;
			while (!args.empty())
			{
				QString act = args.front();
				args.erase(args.begin());
				if (act == "help" || act == "--help" || act == "/help" || act == "-h" || act == "/?")
					ok |= hpiviewCmdHelp(args);
				else if (act == "show" || act == "/show" || act == "--show")
					ok |= hpiviewCmdShow(args);
				else if (act == "minimap" || act == "/minimap" || act == "--minimap")
					ok |= hpiviewCmdMiniMap(args);
				else if (act == "mapdescription" || act == "--mapdescription" || act == "/mapdescription")
					ok |= hpiviewCmdMapDescription(args);
				else if (act == "listmods" || act == "--listmods" || act == "/listmods")
					ok |= hpiviewCmdListMods(args);
				else if (act == "extract" || act == "--extract" || act == "/extract")
					ok |= hpiviewCmdExtract(args);
				else if (act == "print" || act == "--print" || act == "/print")
					ok |= hpiviewCmdPrint(args);
				else if (act == "install" || act == "--install" || act == "/install")
					ok |= hpiviewCmdInstallTAFiles(args);
				else if (act == "extract_gaf" || act == "--extract_gaf" || act == "/extract_gaf")
					ok |= hpiviewCmdExtractGAF(args);
				else if (act == "create_gaf" || act == "--create_gaf" || act == "/create_gaf")
					ok |= hpiviewCmdCreateGAF(args);
				else if (act == "create_buildpic" || act == "--create_buildpic" || act == "/create_buildpic")
					ok |= hpiviewCmdCreateBuildPic(args);
				else if (act == "config" || act == "--config" || act == "/config")
					configWindow();
				else if (act == "listfiles" || act == "--listfiles" || act == "/listfiles")
					ok |= hpiviewCmdListFiles(args);
			}
			if (ok)
				return true;
		}
		SDL_Quit();
		return false;
	}
}
