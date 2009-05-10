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
#include <fstream>
#include <zlib.h>
#include "EngineClass.h"



using namespace TA3D::UTILS::HPI;

# ifdef TA3D_PLATFORM_WINDOWS
#   define PREFIX "  /"
# else
#   define PREFIX "  --"
# endif

//using namespace TA3D::UTILS::HPI;

void install_TA_files( String HPI_file, String filename );


namespace TA3D
{
namespace
{
    String appName;

    bool hpiviewCmdHelp(String::Vector &args)
    {
        std::cout << "Available commands :" << std::endl
            << PREFIX << "create_gaf     : create a 24/32bits gaf from sprites" << std::endl
            << PREFIX << "extract        : extract a file" << std::endl
            << PREFIX << "extract_gaf    : extract a gaf into sprites" << std::endl
            << PREFIX << "help           : this screen" << std::endl
            << PREFIX << "install        : install TA files" << std::endl
            << PREFIX << "listmods       : list all available mods" << std::endl
            << PREFIX << "mapdescription : extract a map description" << std::endl
            << PREFIX << "minimap        : extract a minimap" << std::endl
            << PREFIX << "print          : show the content of a file" << std::endl
            << PREFIX << "show           : show files matching a pattern" << std::endl
            << PREFIX << "quiet          : runs in quiet mode (no logs)" << std::endl
            << std::endl
            << "For more information on a command type :" << std::endl
            << "# " << appName << " command_name" << std::endl;
        return true;
    }


    /*!
     * \brief
     */
    bool hpiviewCmdShow(String::Vector &args)
    {
        if( args.size() >= 1 )
        {
            HPIManager = new cHPIHandler();
            String::List file_list;
            String ext = args[0];
            HPIManager->getFilelist(ext, file_list);
            file_list.sort();
            for (String::List::const_iterator cur_file = file_list.begin() ; cur_file != file_list.end() ; ++cur_file)
                std::cout << *cur_file << std::endl;
            delete HPIManager;

            args.erase(args.begin());
            return true;
        }
        std::cerr << "SYNTAX: " << appName << " show <pattern>" << std::endl;
        return true;
    }


    /*!
     * \brief Extract a mini map from a .tnt file
     */
    bool hpiviewCmdMiniMap(String::Vector &args)
    {
        if(args.size() >= 2)
        {
            HPIManager = new cHPIHandler();
            TA3D::VARS::pal = new SDL_Color[256];
            TA3D::UTILS::HPI::load_palette(pal);

            SDL_Surface* minimap = load_tnt_minimap_fast_bmp( args[0] );
            if(minimap)
            {
                SDL_SetPalette(minimap, SDL_LOGPAL|SDL_PHYSPAL, pal, 0, 256);
                SDL_SaveBMP( minimap, args[1].c_str() );
            }

            delete[] TA3D::VARS::pal;
            delete HPIManager;

            args.erase(args.begin());
            args.erase(args.begin());

            return true;
        }
        std::cerr << "SYNTAX: " << appName << " minimap \"maps\\map.tnt\" minimap_file.jpg" << std::endl;
        return true;
    }


    /*!
     * \brief
     */
    bool hpiviewCmdMapDescription(String::Vector &args)
    {
        if(args.size() >= 2)
        {
            HPIManager = new cHPIHandler();
            MAP_OTA map_data;
            map_data.load( args[0] );
            std::ofstream   m_File;
            m_File.open(args[1].c_str(), std::ios::out | std::ios::trunc);

            if (m_File.is_open())
            {
                m_File << "[MAP]\n{\n";
                m_File << " missionname=" << map_data.missionname << ";\n";
                m_File << " missiondescription=" << ReplaceString( map_data.missiondescription, "\n", "\\n", false ) << ";\n";
                m_File << " planet=" << map_data.planet << ";\n";
                m_File << " tidalstrength=" << map_data.tidalstrength << ";\n";
                m_File << " solarstrength=" << map_data.solarstrength << ";\n";
                m_File << " gravity=" << map_data.gravity << ";\n";
                m_File << " numplayers=" << ReplaceString( map_data.numplayers, "\n", "\\n", false ) << ";\n";
                m_File << " size=" << ReplaceString( map_data.map_size, "\n", "\\n", false ) << ";\n";
                m_File << " minwindspeed=" << map_data.minwindspeed << ";\n";
                m_File << " maxwindspeed=" << map_data.maxwindspeed << ";\n";
                m_File << "}\n";
                m_File.close();
            }
            delete HPIManager;
            args.erase(args.begin());
            args.erase(args.begin());
            return true;
        }
        std::cerr << "SYNTAX: " << appName << " mapdescription \"maps\\map.ota\" description.tdf" << std::endl;
        return true;
    }


    /*!
     * \brief
     */
    bool hpiviewCmdListMods(String::Vector &args)
    {
        if(args.size() >= 1)
        {
            HPIManager = new cHPIHandler();
            std::ofstream m_File;
            m_File.open( args[0].c_str(), std::ios::out | std::ios::trunc );

            if( m_File.is_open() )
            {
                String::List modlist;
                if (Resources::GlobDirs(modlist, "mods/*"))
                {
                    modlist.sort();
                    for (String::List::const_iterator i = modlist.begin(); i != modlist.end(); ++i)
                    {
                        if (!(i->empty()) && ((*i)[0] != '.'))
                            m_File << *i << "\n";
                    }
                }
                m_File.close();
            }
            delete HPIManager;
            args.erase(args.begin());
            return true;
        }
        LOG_ERROR("Syntax: " << appName << " listmods modlist.txt");
        return true;
    }



    /*!
     * \brief
     */
    bool hpiviewCmdExtract(String::Vector &args)
    {
        if(args.size() >= 1)
        {
            HPIManager = new cHPIHandler();
            uint32 file_size32 = 0;
            byte *data = HPIManager->PullFromHPI(args[0], &file_size32);

            if(data)
            {
                String name = Paths::ExtractFileName(args[0]);
                FILE *dst = TA3D_OpenFile(name, "wb");
                fwrite(data, file_size32, 1, dst);
                fclose(dst);
                delete[] data;
            }
            delete HPIManager;
            args.erase(args.begin());
            return true;
        }
        std::cerr << "SYNTAX: " << appName << " extract <filename>" << std::endl;
        return true;
    }


    /*!
     * \brief
     */
    bool hpiviewCmdPrint(String::Vector &args)
    {
        if(args.size() >= 1)
        {
            HPIManager = new cHPIHandler();
            String::List file_list;
            String ext = args[0];
            HPIManager->getFilelist(ext, file_list);
            file_list.sort();

            for (String::List::iterator cur_file = file_list.begin(); cur_file != file_list.end(); ++cur_file)
            {
                uint32 file_size32 = 0;
                byte* data = HPIManager->PullFromHPI( *cur_file, &file_size32 );
                if(data)
                {
                    std::cout << (const char*)data << std::endl;
                    delete[] data;
                }
            }
            delete HPIManager;
            args.erase(args.begin());
            return true;
        }
        std::cerr << "SYNTAX: " << appName << " print pattern" << std::endl;
        return true;
    }


    /*!
     * \brief
     */
    bool hpiviewCmdInstallTAFiles(String::Vector &args)
    {
        if (args.size() >= 2)
        {
            install_TA_files(args[0], args[1]);
            args.erase(args.begin());
            args.erase(args.begin());
        }
        else
            std::cerr << "SYNTAX: " << appName << " install HPI_file file_in_HPI" << std::endl;
        return true;
    }


    /*!
     * \brief Extract images from a GAF file
     */
    bool hpiviewCmdExtractGAF(String::Vector &args)
    {
        if(args.size() >= 1)
        {
            HPIManager = new cHPIHandler();
            uint32 file_size32 = 0;
            byte *data = HPIManager->PullFromHPI(args[0],&file_size32);

            if(data)
            {
                SDL_SetVideoMode(320, 200, 32, 0);
                TA3D::VARS::pal = new SDL_Color[256];      // Allocate a new palette
                TA3D::UTILS::HPI::load_palette(pal);

                Gaf::AnimationList anims;
                anims.loadGAFFromRawData(data);
                std::ofstream   m_File;
                m_File.open( String::Format("%s.txt", Paths::ExtractFileName( args[0] ).c_str()).c_str(), std::ios::out | std::ios::trunc );

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
                        String filename = String::Format("%s%d.tga", anims[i].name.c_str(), e);
                        m_File << "    [frame" << e << "]\n    {\n";
                        m_File << "        XPos=" << anims[i].ofs_x[ e ] << ";\n";
                        m_File << "        YPos=" << anims[i].ofs_y[ e ] << ";\n";
                        m_File << "        filename=" << filename << ";\n";
                        m_File << "    }\n";
                        SDL_SetPalette(anims[i].bmp[e], SDL_LOGPAL|SDL_PHYSPAL, pal, 0, 256);
                        SDL_SaveBMP( anims[i].bmp[e], filename.c_str() );
                    }
                    m_File << "}\n";
                }

                m_File.flush();
                m_File.close();
                delete[] data;
                delete[] TA3D::VARS::pal;
            }
            delete HPIManager;
            args.erase(args.begin());
            return true;
        }
        std::cerr << "SYNTAX: " << appName << " extract_gaf <filename.gaf>" << std::endl;
        return true;
    }




    /*!
     * \brief Create a truecolor GAF from images
     */
    bool hpiviewCmdCreateGAF(String::Vector &args)
    {
        if(args.size() >= 1)
        {
            TDFParser parser( args[0], false, false, false );
            String filename = parser.pullAsString( "gadget0.filename" );
            FILE *gaf_file = TA3D_OpenFile( Paths::ExtractFileName( filename ), "wb" );

            if (gaf_file)
            {
                SDL_SetVideoMode(320, 200, 32, 0);
                Gaf::Header header;
                header.IDVersion = TA3D_GAF_TRUECOLOR;
                header.Entries   = parser.pullAsInt("gadget0.entries");
                header.Unknown1  = 0;

                fwrite(&header, 12, 1, gaf_file);

                for (int i = 0; i < header.Entries * 4; ++i)
                    putc( 0, gaf_file );

                for (int i = 0; i < header.Entries; ++i)
                {
                    int pos = ftell( gaf_file );
                    fseek( gaf_file, 12 + i * 4, SEEK_SET );
                    fwrite( &pos, 4, 1, gaf_file );
                    fseek( gaf_file, pos, SEEK_SET );

                    Gaf::Entry Entry;

                    Entry.Frames = parser.pullAsInt( String::Format( "gadget%d.frames", i + 1 ) );
                    Entry.Unknown1 = 1;
                    Entry.Unknown2 = 0;
                    Entry.name = parser.pullAsString( String::Format( "gadget%d.name", i + 1 ) );

                    fwrite( &Entry.Frames, 2, 1, gaf_file );
                    fwrite( &Entry.Unknown1, 2, 1, gaf_file );
                    fwrite( &Entry.Unknown2, 4, 1, gaf_file );
                    char tmp[32];
                    memset(tmp, 0, 32);
                    memcpy(tmp, Entry.name.c_str(), Math::Min((int)Entry.name.size(), 32));
                    tmp[31] = 0;
                    fwrite(tmp, 32, 1, gaf_file);

                    Gaf::Frame::Entry FrameEntry;
                    int FrameEntryPos = ftell(gaf_file);
                    FrameEntry.PtrFrameTable = 0;
                    for (int e = 0; e < Entry.Frames; ++e)
                        fwrite( &(FrameEntry), 8, 1, gaf_file );

                    for (int e = 0; e < Entry.Frames; ++e)
                    {
                        pos = ftell( gaf_file );
                        fseek( gaf_file, FrameEntryPos + e * 8, SEEK_SET );
                        FrameEntry.PtrFrameTable = pos;
                        fwrite( &(FrameEntry), 8, 1, gaf_file );
                        fseek( gaf_file, pos, SEEK_SET );

                        Gaf::Frame::Data FrameData;
                        FrameData.XPos = parser.pullAsInt( String::Format( "gadget%d.frame%d.XPos", i + 1, e ) );
                        FrameData.YPos = parser.pullAsInt( String::Format( "gadget%d.frame%d.YPos", i + 1, e ) );
                        FrameData.FramePointers = 0;
                        FrameData.Unknown2 = 0;
                        FrameData.Compressed = 1;

                        SDL_Surface *frame_img = gfx->load_image( parser.pullAsString( String::Format( "gadget%d.frame%d.filename", i + 1, e ) ) );
                        if( frame_img )
                        {
                            FrameData.Width = frame_img->w;
                            FrameData.Height = frame_img->h;
                            bool alpha = false;
                            SDL_LockSurface(frame_img);
                            for( int y = 0 ; y < frame_img->h && !alpha ; y++ )
                                for( int x = 0 ; x < frame_img->w && !alpha ; x++ )
                                    alpha |= (getr(SurfaceInt(frame_img, x, y)) != 255);
                            SDL_UnlockSurface(frame_img);
                            FrameData.Transparency = alpha ? 1 : 0;
                            FrameData.PtrFrameData = ftell( gaf_file ) + 24;

                            fwrite( &FrameData, 24, 1, gaf_file );

                            int buf_size = frame_img->w * frame_img->h * 5 + 10240;
                            byte *buffer = new byte[ buf_size ];

                            int img_size = buf_size;
                            uLongf __size = img_size;
                            compress2 ( buffer, &__size, (Bytef*) frame_img->pixels, frame_img->w * frame_img->h * frame_img->format->BytesPerPixel, 9);
                            img_size = __size;

                            fwrite( &img_size, sizeof( img_size ), 1, gaf_file );		// Save the result
                            fwrite( buffer, img_size, 1, gaf_file );

                            delete[] buffer;
                            SDL_FreeSurface( frame_img );
                        }
                        else
                        {
                            std::cerr << "Error: In frame " << e << ", could not load "
                                << parser.pullAsString(String::Format( "gadget%d.frame%d.filename", i + 1, e ) ).c_str() << std::endl;
                            i = header.Entries;
                            break;
                        }
                    }
                }
            }
            else
                std::cerr << "Error: Could not create file!" << std::endl;
            args.erase(args.begin());
        }
        else
            std::cerr << "SYNTAX: " << appName << " create_gaf gafdescription.txt" << std::endl;
        return true;
    }

} // unnamed namespace
} // namespace TA3D





/*!
 * \brief
 */
int hpiview(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO);

    if(argc >= 2)
    {
        appName = argv[0];

        // TODO Use a better implementation to parse arguments
        String::Vector args;
        for (int i = 1 ; i < argc ; i++)
            args.push_back(argv[i]);
        bool ok = false;
        while (!args.empty())
        {
            String act = args.front();
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
        }
        if (ok)
            return true;
    }
    SDL_Quit();
    return false;
}

