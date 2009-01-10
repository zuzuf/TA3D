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
#include "jpeg/ta3d_jpg.h"
#include <iostream>
#include "tnt.h"
#include "logs/logs.h"
#include "misc/paths.h"
#include "misc/resources.h"
#include <fstream>



using namespace TA3D::UTILS::HPI;

# ifdef TA3D_PLATFORM_WINDOWS
#   define PREFIX "  /"
# else
#   define PREFIX "  --"
# endif

//using namespace TA3D::UTILS::HPI;

void install_TA_files( String def_path = "" );


namespace TA3D
{
namespace
{

    bool hpiviewCmdHelp(char** argv)
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
            << std::endl
            << "For more information on a command type :" << std::endl
            << "# " << argv[0] << " command_name" << std::endl;
        return true;
    }


    /*!
     * \brief
     */
    bool hpiviewCmdShow(int argc, char** argv)
    {
        if( argc >= 3 )
        {
            HPIManager = new cHPIHandler();
            String::List file_list;
            String ext = (argc > 2) ? argv[2] : "";
            HPIManager->getFilelist(ext, file_list);
            file_list.sort();
            for (String::List::const_iterator cur_file=file_list.begin();cur_file!=file_list.end(); ++cur_file)
                std::cout << *cur_file << std::endl;
            delete HPIManager;
            return true;
        }
        std::cerr << "SYNTAX: " << argv[0] << " show <pattern>" << std::endl;
        return true;
    }


    /*!
     * \brief Extract a mini map from a .tnt file
     */
    bool hpiviewCmdMiniMap(int argc, char** argv)
    {
        if(argc >= 4)
        {
            HPIManager = new cHPIHandler();
            TA3D::VARS::pal = new RGB[256];
            TA3D::UTILS::HPI::load_palette(pal);
            set_palette(pal);      // Activate the palette

            jpgalleg_init();

            set_color_depth( 32 );
            BITMAP* minimap = load_tnt_minimap_fast_bmp( argv[2] );
            if(minimap)
                save_bitmap( argv[3], minimap, NULL );

            delete[] TA3D::VARS::pal;
            delete HPIManager;
            return true;
        }
        std::cerr << "SYNTAX: " << argv[0] << " minimap \"maps\\map.tnt\" minimap_file.jpg" << std::endl;
        return true;
    }


    /*!
     * \brief
     */
    bool hpiviewCmdMapDescription(int argc, char** argv)
    {
        if(argc >= 4)
        {
            HPIManager = new cHPIHandler();
            MAP_OTA map_data;
            map_data.load( argv[2] );
            std::ofstream   m_File;
            m_File.open(argv[3], std::ios::out | std::ios::trunc);

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
            return true;
        }
        std::cerr << "SYNTAX: " << argv[0] << " mapdescription \"maps\\map.ota\" description.tdf" << std::endl;
        return true;
    }


    /*!
     * \brief
     */
    bool hpiviewCmdListMods(int argc, char** argv)
    {
        if(argc >= 3)
        {
            HPIManager = new cHPIHandler();
            std::ofstream m_File;
            m_File.open( argv[2], std::ios::out | std::ios::trunc );

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
            return true;
        }
        LOG_ERROR("Syntax: " << argv[0] << " listmods modlist.txt");
        return true;
    }



    /*!
     * \brief
     */
    bool hpiviewCmdExtract(int argc, char** argv)
    {
        if(argc >= 3)
        {
            HPIManager = new cHPIHandler();
            uint32 file_size32 = 0;
            byte *data = HPIManager->PullFromHPI(argv[2], &file_size32);

            if(data)
            {
                char *name = argv[2];
                char *f = argv[2];
                while(f[0])
                {
                    if(f[0]=='\\' || f[0]=='/')
                        name=f+1;
                    f++;
                }
                FILE *dst = TA3D_OpenFile(name,"wb");
                fwrite(data,file_size32,1,dst);
                fclose(dst);
                delete[] data;
            }
            delete HPIManager;
            return true;
        }
        std::cerr << "SYNTAX: " << argv[0]<< " extract <filename>" << std::endl;
        return true;
    }


    /*!
     * \brief
     */
    bool hpiviewCmdPrint(int argc, char** argv)
    {
        if(argc >= 3)
        {
            HPIManager = new cHPIHandler();
            String::List file_list;
            String ext = argc > 2 ? argv[2] : "";
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
            return true;
        }
        std::cerr << "SYNTAX: " << argv[0] << " print pattern" << std::endl;
        return true;
    }


    /*!
     * \brief
     */
    bool hpiviewCmdInstallTAFiles(int argc, char** argv)
    {
        install_TA_files((argc >= 3) ? argv[2] : "");
        return true;
    }


    /*!
     * \brief Extract images from a GAF file
     */
    bool hpiviewCmdExtractGAF(int argc, char** argv)
    {
        if(argc >= 3)
        {
            HPIManager=new cHPIHandler();
            uint32 file_size32 = 0;
            byte *data = HPIManager->PullFromHPI(argv[2],&file_size32);

            if(data)
            {
                set_color_depth( 32 );
                set_gfx_mode( GFX_AUTODETECT_WINDOWED, 320, 200, 0, 0 );
                TA3D::VARS::pal=new RGB[256];      // Allocate a new palette
                TA3D::UTILS::HPI::load_palette(pal);
                set_palette(pal);      // Activate the palette

                Gaf::AnimationList anims;
                anims.loadGAFFromRawData(data);
                std::ofstream   m_File;
                m_File.open( format("%s.txt", get_filename( argv[2] )).c_str(), std::ios::out | std::ios::trunc );

                m_File << "[gadget0]\n{\n";
                m_File << "    filename=" << argv[2] << ";\n";
                m_File << "    entries=" << anims.size() << ";\n";
                m_File << "}\n";

                for (sint32 i = 0; i < anims.size(); ++i)
                {
                    m_File << "[gadget" << (i + 1) << "]\n{\n";
                    m_File << "    frames=" << anims[i].nb_bmp << ";\n";
                    m_File << "    name=" << anims[i].name << ";\n";
                    for (int e = 0; e < anims[i].nb_bmp; ++e)
                    {
                        String filename = format("%s%d.tga", anims[i].name.c_str(), e);
                        m_File << "    [frame" << e << "]\n    {\n";
                        m_File << "        XPos=" << anims[i].ofs_x[ e ] << ";\n";
                        m_File << "        YPos=" << anims[i].ofs_y[ e ] << ";\n";
                        m_File << "        filename=" << filename << ";\n";
                        m_File << "    }\n";
                        save_bitmap(filename.c_str(), anims[i].bmp[e], NULL);
                    }
                    m_File << "}\n";
                }

                m_File.flush();
                m_File.close();
                delete[] data;
                delete[] TA3D::VARS::pal;
            }
            delete HPIManager;
            return true;
        }
        std::cerr << "SYNTAX: " << argv[0] << " extract_gaf <filename.gaf>" << std::endl;
        return true;
    }




    /*!
     * \brief Create a truecolor GAF from images
     */
    bool hpiviewCmdCreateGAF(int argc, char** argv)
    {
        if(argc >= 3)
        {
            jpgalleg_init();
            cTAFileParser parser( argv[2], false, false, false );
            String filename = parser.pullAsString( "gadget0.filename" );
            FILE *gaf_file = TA3D_OpenFile( get_filename( filename.c_str() ), "wb" );

            if (gaf_file)
            {
                set_color_depth(32);
                set_gfx_mode(GFX_AUTODETECT_WINDOWED, 320, 200, 0, 0);
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

                    Entry.Frames = parser.pullAsInt( format( "gadget%d.frames", i + 1 ) );
                    Entry.Unknown1 = 1;
                    Entry.Unknown2 = 0;
                    Entry.name = parser.pullAsString( format( "gadget%d.name", i + 1 ) );

                    fwrite( &Entry.Frames, 2, 1, gaf_file );
                    fwrite( &Entry.Unknown1, 2, 1, gaf_file );
                    fwrite( &Entry.Unknown2, 4, 1, gaf_file );
                    char tmp[32];
                    memset(tmp, 0, 32);
                    memcpy(tmp, Entry.name.c_str(), Math::Min((int)Entry.name.size(), 32));
                    tmp[32] = 0;
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
                        FrameData.XPos = parser.pullAsInt( format( "gadget%d.frame%d.XPos", i + 1, e ) );
                        FrameData.YPos = parser.pullAsInt( format( "gadget%d.frame%d.YPos", i + 1, e ) );
                        FrameData.FramePointers = 0;
                        FrameData.Unknown2 = 0;
                        FrameData.Compressed = 1;

                        BITMAP *frame_img = gfx->load_image( parser.pullAsString( format( "gadget%d.frame%d.filename", i + 1, e ) ) );
                        if( frame_img )
                        {
                            FrameData.Width = frame_img->w;
                            FrameData.Height = frame_img->h;
                            bool alpha = false;
                            for( int y = 0 ; y < frame_img->h && !alpha ; y++ )
                                for( int x = 0 ; x < frame_img->w && !alpha ; x++ )
                                    alpha |= (frame_img->line[y][(x<<2)+3] != 255);
                            FrameData.Transparency = alpha ? 1 : 0;
                            FrameData.PtrFrameData = ftell( gaf_file ) + 24;

                            fwrite( &FrameData, 24, 1, gaf_file );

                            int buf_size = frame_img->w * frame_img->h * 5 + 10240;
                            byte *buffer = new byte[ buf_size ];

                            int img_size = buf_size;
                            if( save_memory_jpg_ex( buffer, &img_size, frame_img, NULL, 95, JPG_SAMPLING_444, NULL ) ) // RGB channels
                            {
                                img_size = buf_size;
                                if( save_memory_jpg_ex( buffer, &img_size, frame_img, NULL, 95, JPG_SAMPLING_444 | JPG_OPTIMIZE, NULL ) )		// RGB channels
                                    printf("error saving '%s'\n", parser.pullAsString( format( "gadget%d.frame%d.filename", i + 1, e ) ).c_str() );
                            }

                            fwrite( &img_size, sizeof( img_size ), 1, gaf_file );		// Save the result
                            fwrite( buffer, img_size, 1, gaf_file );

                            if( alpha ) // Alpha channel
                            {
                                for( int y = 0 ; y < frame_img->h ; y++ )
                                {
                                    for( int x = 0 ; x < frame_img->w ; x++ )
                                    {
                                        uint32 c = frame_img->line[y][(x<<2)+3];
                                        ((uint32*)(frame_img->line[y]))[x] = c * 0x010101;
                                    }
                                }
                                img_size = buf_size;
                                if(save_memory_jpg_ex( buffer, &img_size, frame_img, NULL, 100, JPG_GREYSCALE | JPG_OPTIMIZE, NULL ) )
                                {
                                    img_size = buf_size;
                                    if(save_memory_jpg_ex( buffer, &img_size, frame_img, NULL, 100, JPG_GREYSCALE, NULL))
                                    {
                                        std::cerr << "Error saving alpha channel for '"
                                            << parser.pullAsString(format("gadget%d.frame%d.filename", i + 1, e ) ).c_str() << "'" << std::endl;
                                    }
                                }

                                fwrite( &img_size, sizeof( img_size ), 1, gaf_file );		// Save the result
                                fwrite( buffer, img_size, 1, gaf_file );
                            }

                            delete[] buffer;
                            destroy_bitmap( frame_img );
                        }
                        else
                        {
                            std::cerr << "Error: In frame " << e << ", could not load "
                                << parser.pullAsString(format( "gadget%d.frame%d.filename", i + 1, e ) ).c_str() << std::endl;
                            i = header.Entries;
                            break;
                        }
                    }
                }
            }
            else
                std::cerr << "Error: Could not create file!" << std::endl;
        }
        else
            std::cerr << "SYNTAX: " << argv[0] << " create_gaf gafdescription.txt" << std::endl;
        return true;
    }

} // unnamed namespace
} // namespace TA3D





/*!
 * \brief
 */
int hpiview(int argc, char *argv[])
{
    allegro_init();
    set_uformat(U_ASCII); // TODO Ensure UTF8 is working

    if(argc >= 2)
    {
        // TODO Use a better implementation to parse arguments
        String act(argv[1]);
        if (act == "help" || act == "--help" || act == "/help" || act == "-h" || act == "/?")
            return hpiviewCmdHelp(argv);
        if (act == "show" || act == "/show" || act == "--show")
            return hpiviewCmdShow(argc, argv);
        if (act == "minimap" || act == "/minimap" || act == "--minimap")
            return hpiviewCmdMiniMap(argc, argv);
        if (act == "mapdescription" || act == "--mapdescription" || act == "/mapdescription")
            return hpiviewCmdMapDescription(argc, argv);
        if (act == "listmods" || act == "--listmods" || act == "/listmods")
            return hpiviewCmdListMods(argc, argv);
        if (act == "extract" || act == "--extract" || act == "/extract")
            return hpiviewCmdExtract(argc, argv);
        if (act == "print" || act == "--print" || act == "/print")
            return hpiviewCmdPrint(argc, argv);
        if (act == "install" || act == "--install" || act == "/install")
            return hpiviewCmdInstallTAFiles(argc, argv);
        if (act == "extract_gaf" || act == "--extract_gaf" || act == "/extract_gaf")
            return hpiviewCmdExtractGAF(argc, argv);
        if (act == "create_gaf" || act == "--create_gaf" || act == "/create_gaf")
            return hpiviewCmdCreateGAF(argc, argv);
    }
    allegro_exit();
    return false;
}

