
#include "settings.h"
#include "../logs/logs.h"
#include "paths.h"
#include "../TA3D_NameSpace.h"
#include "../TA3D_Exception.h"
#include "../ta3dbase.h"
#include "../languages/i18n.h"



namespace TA3D
{
namespace Settings
{



    bool Backup(const String& filename)
    {
        LOG_INFO("Making a backup for `" << filename << "`...");
        if(TA3D::Paths::Exists(filename))
        {
            FILE *src = TA3D_OpenFile(filename, "rb" );
            if( src )
            {
                FILE *dst = TA3D_OpenFile(filename + ".bak", "wb");
                if(dst)
                {
                    uint32 src_size = FILE_SIZE(filename.c_str());
                    byte *buf = new byte[ src_size ];
                    fread( buf, src_size, 1, src );
                    fwrite( buf, src_size, 1, dst );
                    delete[] buf;
                    fclose( dst );
                }
                else
                {
                    fclose(src);
                    LOG_ERROR("Can not make the backup: Impossible to write `" << filename << ".bak`");
                    return false;
                }
                fclose(src);
                LOG_INFO("The backup is done.");
                return true;
            }
            LOG_WARNING("Can not make the backup: Impossible to open file");
            return false;
        }
        LOG_WARNING("Can not make the backup: File not found");
        return false;
    }


    bool Save()
    {
        lp_CONFIG->Lang = LANG;
        if( !TA3D::VARS::lp_CONFIG )
            return false;

        // Make a copy that can be restored if TA3D does not start any more
        TA3D::Settings::Backup(TA3D::Paths::ConfigFile);

        std::ofstream m_File;
        m_File.open(TA3D::Paths::ConfigFile.c_str(), std::ios::out | std::ios::trunc );
        if( !m_File.is_open() )
        {
            LOG_ERROR("Impossible to write settings: `" << TA3D::Paths::ConfigFile << "`");
            return false;
        }

        LOG_INFO("Saving settings in `" << TA3D::Paths::ConfigFile << "`...");
        m_File << "[TA3D]\n{\n";
        m_File << "               FPS Limit=" << TA3D::VARS::lp_CONFIG->fps_limit << ";\n";
        m_File << "                Shadow R=" << TA3D::VARS::lp_CONFIG->shadow_r << ";\n";
        m_File << "             Time Factor=" << TA3D::VARS::lp_CONFIG->timefactor << ";\n";
        m_File << "          Shadow Quality=" << TA3D::VARS::lp_CONFIG->shadow_quality << ";// 0->100\n";
        m_File << "          Priority Level=" << TA3D::VARS::lp_CONFIG->priority_level << ";// 0, 1, 2\n";
        m_File << "                    FSAA=" << TA3D::VARS::lp_CONFIG->fsaa << ";\n";
        m_File << "                Language=" << TA3D::VARS::lp_CONFIG->Lang << ";\n";
        m_File << "           Water Quality=" << TA3D::VARS::lp_CONFIG->water_quality << ";//0->4\n";
        m_File << "            Screen Width=" << TA3D::VARS::lp_CONFIG->screen_width << ";\n";
        m_File << "           Screen Height=" << TA3D::VARS::lp_CONFIG->screen_height << ";\n";
        m_File << "             Color Depth=" << (int)TA3D::VARS::lp_CONFIG->color_depth << ";\n";
        m_File << "                Show FPS=" << TA3D::VARS::lp_CONFIG->showfps << ";\n";
        m_File << "          Show Wireframe=" << TA3D::VARS::lp_CONFIG->wireframe << ";\n";
        m_File << "          Show particles=" << TA3D::VARS::lp_CONFIG->particle << ";\n";
        m_File << "Show explosion particles=" << TA3D::VARS::lp_CONFIG->explosion_particles << ";\n";
        m_File << "              Show Waves=" << TA3D::VARS::lp_CONFIG->waves << ";\n";
        m_File << "            Show Shadows=" << TA3D::VARS::lp_CONFIG->shadow << ";\n";
        m_File << "       Show Height Lines=" << TA3D::VARS::lp_CONFIG->height_line << ";\n";
        m_File << "         Show FullScreen=" << TA3D::VARS::lp_CONFIG->fullscreen << ";\n";
        m_File << "          Detail Texture=" << TA3D::VARS::lp_CONFIG->detail_tex << ";\n";
        m_File << "    Draw Console Loading=" << TA3D::VARS::lp_CONFIG->draw_console_loading << ";\n";
        m_File << "             Last Script=" << ReplaceChar( TA3D::VARS::lp_CONFIG->last_script, '\\', '/' ) << ";\n";
        m_File << "                Last Map=" << ReplaceChar( TA3D::VARS::lp_CONFIG->last_map, '\\', '/' ) << ";\n";
        m_File << "                Last FOW=" << (int)TA3D::VARS::lp_CONFIG->last_FOW << ";\n";
        TA3D::VARS::lp_CONFIG->last_MOD = TA3D::VARS::TA3D_CURRENT_MOD;
        m_File << "                Last MOD=" << TA3D::VARS::lp_CONFIG->last_MOD << ";\n";
        m_File << "             Player name=" << TA3D::VARS::lp_CONFIG->player_name << ";\n";
        m_File << "        Camera Zoom Mode=" << (int)TA3D::VARS::lp_CONFIG->camera_zoom << ";\n";
        m_File << "    Camera Default Angle=" << TA3D::VARS::lp_CONFIG->camera_def_angle << ";\n";
        m_File << "   Camera Default Height=" << TA3D::VARS::lp_CONFIG->camera_def_h << ";\n";
        m_File << "       Camera Zoom Speed=" << TA3D::VARS::lp_CONFIG->camera_zoom_speed << ";\n";
        m_File << "                    Skin=" << TA3D::VARS::lp_CONFIG->skin_name << ";\n";
        m_File << "       Use Texture Cache=" << TA3D::VARS::lp_CONFIG->use_texture_cache << ";\n";
        m_File << "              Net Server=" << TA3D::VARS::lp_CONFIG->net_server << ";\n";
        m_File << "}\n";

        m_File.flush();
        m_File.close();
        LOG_INFO("Settings has been saved.");
        return true;
    }



    bool Restore(const String& filename) 
    {
        if( TA3D::Paths::Exists(filename + ".bak"))
        {
            FILE *src = TA3D_OpenFile(filename + ".bak", "rb");
            if( src )
            {
                FILE *dst = TA3D_OpenFile(filename, "wb");
                if(dst)
                {
                    uint32 src_size = FILE_SIZE( (filename + ".bak").c_str());
                    byte *buf = new byte[ src_size ];
                    fread( buf, src_size, 1, src );
                    fwrite( buf, src_size, 1, dst );
                    delete[] buf;
                    fclose( dst );
                }
                else
                {
                    fclose(src);
                    LOG_WARNING("Can not restore the file: Can not write `" << filename << "`");
                    return false;
                }
                fclose(src);
                LOG_INFO("The file `" << filename << ".bak` have been restored.");
                return true;
            }
            LOG_WARNING("Can not restore the file: Can not open `" << filename << ".bak`");
            return false;
        }
        LOG_WARNING("Can not restore the file: File not found: `" << filename << "`");
        return false;
    }


    bool Load()
    {
        cTAFileParser *cfgFile;

        try { // we need to try catch this cause the config file may not exists
            // and if it don't exists it will throw an error on reading it, which
            // will be caught in our main function and the application will exit.
            cfgFile = new TA3D::UTILS::cTAFileParser(TA3D::Paths::ConfigFile);
        }
        catch( ... )
        {
            LOG_WARNING("Impossible to parse `" << TA3D::Paths::ConfigFile << "`");
            return false;
        }

        TA3D::VARS::lp_CONFIG->fps_limit = cfgFile->PullAsFloat( "TA3D.FPS Limit" );
        TA3D::VARS::lp_CONFIG->shadow_r  = cfgFile->PullAsFloat( "TA3D.Shadow R" );
        TA3D::VARS::lp_CONFIG->timefactor = cfgFile->PullAsFloat( "TA3D.Time Factor" );

        TA3D::VARS::lp_CONFIG->shadow_quality = cfgFile->PullAsInt( "TA3D.Shadow Quality" );
        TA3D::VARS::lp_CONFIG->priority_level = cfgFile->PullAsInt( "TA3D.Priority Level" );
        TA3D::VARS::lp_CONFIG->fsaa = cfgFile->PullAsInt( "TA3D.FSAA" );
        TA3D::VARS::lp_CONFIG->Lang = cfgFile->PullAsInt( "TA3D.Language" );
        TA3D::VARS::lp_CONFIG->water_quality = cfgFile->PullAsInt( "TA3D.Water Quality" );
        TA3D::VARS::lp_CONFIG->screen_width = cfgFile->PullAsInt( "TA3D.Screen Width" );
        TA3D::VARS::lp_CONFIG->screen_height = cfgFile->PullAsInt( "TA3D.Screen Height" );
        TA3D::VARS::lp_CONFIG->color_depth = cfgFile->PullAsInt( "TA3D.Color Depth", 32 );

        TA3D::VARS::lp_CONFIG->showfps = cfgFile->PullAsBool( "TA3D.Show FPS" );
        TA3D::VARS::lp_CONFIG->wireframe = cfgFile->PullAsBool( "TA3D.Show Wireframe" );
        TA3D::VARS::lp_CONFIG->particle = cfgFile->PullAsBool( "TA3D.Show particles" );
        TA3D::VARS::lp_CONFIG->waves = cfgFile->PullAsBool( "TA3D.Show Waves" );
        TA3D::VARS::lp_CONFIG->shadow = cfgFile->PullAsBool( "TA3D.Show Shadows" );
        TA3D::VARS::lp_CONFIG->height_line = cfgFile->PullAsBool( "TA3D.Show Height Lines" );
        TA3D::VARS::lp_CONFIG->fullscreen = cfgFile->PullAsBool( "TA3D.Show FullScreen", false );
        TA3D::VARS::lp_CONFIG->detail_tex = cfgFile->PullAsBool( "TA3D.Detail Texture" );
        TA3D::VARS::lp_CONFIG->draw_console_loading = cfgFile->PullAsBool( "TA3D.Draw Console Loading" );

        TA3D::VARS::lp_CONFIG->last_script = ReplaceChar( cfgFile->PullAsString( "TA3D.Last Script", "scripts\\default.c" ), '/', '\\' );
        TA3D::VARS::lp_CONFIG->last_map = ReplaceChar( cfgFile->PullAsString( "TA3D.Last Map", "" ), '/', '\\' );
        TA3D::VARS::lp_CONFIG->last_FOW = cfgFile->PullAsInt( "TA3D.Last FOW", 0 );
        TA3D::VARS::lp_CONFIG->last_MOD = cfgFile->PullAsString( "TA3D.Last MOD", "" );

        TA3D::VARS::lp_CONFIG->camera_zoom = cfgFile->PullAsInt( "TA3D.Camera Zoom Mode", ZOOM_NORMAL );
        TA3D::VARS::lp_CONFIG->camera_def_angle = cfgFile->PullAsFloat( "TA3D.Camera Default Angle", 63.44f );
        TA3D::VARS::lp_CONFIG->camera_def_h = cfgFile->PullAsFloat( "TA3D.Camera Default Height", 200.0f );
        TA3D::VARS::lp_CONFIG->camera_zoom_speed = cfgFile->PullAsFloat( "TA3D.Camera Zoom Speed", 1.0f );

        TA3D::VARS::lp_CONFIG->use_texture_cache = cfgFile->PullAsBool( "TA3D.Use Texture Cache", false );

        TA3D::VARS::lp_CONFIG->skin_name = cfgFile->PullAsString( "TA3D.Skin", "" );

        TA3D::VARS::lp_CONFIG->net_server = cfgFile->PullAsString( "TA3D.Net Server", "ta3d.darkstars.co.uk" );

        TA3D::VARS::TA3D_CURRENT_MOD = TA3D::VARS::lp_CONFIG->last_MOD;

        TA3D::VARS::lp_CONFIG->player_name = cfgFile->PullAsString( "TA3D.Player name", "player" );

        delete cfgFile; 

        LANG = lp_CONFIG->Lang;
        // Apply settings for the current language
        I18N::Instance()->currentLanguage(lp_CONFIG->Lang);
        return true;
    }



} // namespace Settings
} // namespace TA3D
