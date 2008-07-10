
#include "skin.h"
#include "../../misc/paths.h"
#include "../../TA3D_Exception.h"


using namespace TA3D::Exceptions;


namespace TA3D
{


    SKIN::~SKIN()
    {
         for (sint8 i = 0; i < 2; ++i)
        {
            progress_bar[i].destroy();
            button_img[i].destroy();
            checkbox[i].destroy();
            option[i].destroy();
            scroll[i].destroy();
        }
        scroll[2].destroy();
        text_background.destroy();
        menu_background.destroy();
        wnd_border.destroy();
        wnd_title_bar.destroy();
        selection_gfx.destroy();
        gfx->destroy_texture(wnd_background);
    }


    void SKIN::init()
    {
        prefix.clear();
        text_y_offset = 0;

        for (sint8 i = 0; i < 2 ; ++i)
            button_img[i].init();
        text_background.init();
        menu_background.init();
        wnd_border.init();
        wnd_title_bar.init();
        selection_gfx.init();
        for (sint8 i = 0; i < 2; ++i)
            progress_bar[i].init();

        wnd_background = 0;
        checkbox[1].init();
        checkbox[0].init();
        option[1].init();
        option[0].init();
        scroll[2].init();
        scroll[1].init();
        scroll[0].init();
    }

    void SKIN::destroy()
    {
        for (sint8 i = 0; i < 2; ++i)
        {
            progress_bar[i].destroy();
            button_img[i].destroy();
            checkbox[i].destroy();
            option[i].destroy();
            scroll[i].destroy();
        }
        scroll[2].destroy();
        text_background.destroy();
        menu_background.destroy();
        wnd_border.destroy();
        wnd_title_bar.destroy();
        selection_gfx.destroy();
        prefix.clear();
        Name.clear();
        gfx->destroy_texture(wnd_background);
    }


    void SKIN::load_tdf(const String& filename, const float scale)
    {
        GuardEnter(SKIN::load_tdf);

        destroy();		// In case there is a skin loaded so we don't waste memory

        cTAFileParser *skinFile;

        try
        {
            // we need to try catch this cause the config file may not exists
            // and if it don't exists it will throw an error on reading it, which
            // will be caught in our main function and the application will exit.
            skinFile = new TA3D::UTILS::cTAFileParser( filename );
        }
        catch (...)
        {
            GuardLeave();
            return;
        }

        // Grab the skin's name, so we can now if a skin is already in use
        String::size_type e = filename.find(".");
        if (e != String::npos)
            Name = filename.substr( 0, e );
        else
            Name = filename;

        e = Name.find_last_of("/\\");

        if (e != String::npos)
            Name = Name.substr(e + 1, Name.size() - e - 1);

        Name = skinFile->PullAsString("skin.name", Name); // The TDF may override the skin name

        prefix = skinFile->PullAsString("skin.prefix", ""); // The prefix to use for 
        text_y_offset = skinFile->PullAsInt("skin.text y offset", 0);

        wnd_border.load(skinFile->PullAsString("skin.window borders"), "skin.border_", skinFile, scale);
        button_img[0].load(skinFile->PullAsString("skin.button0"), "skin.button_", skinFile, scale);
        button_img[1].load(skinFile->PullAsString("skin.button1"), "skin.button_", skinFile, scale);
        text_background.load(skinFile->PullAsString("skin.text background"), "skin.text_", skinFile, scale);
        menu_background.load(skinFile->PullAsString("skin.menu background"), "skin.menu_", skinFile, scale);
        wnd_title_bar.load(skinFile->PullAsString("skin.title bar"), "skin.title_", skinFile, scale);
        progress_bar[0].load(skinFile->PullAsString("skin.progress bar0"), "skin.bar0_", skinFile, scale);
        progress_bar[1].load(skinFile->PullAsString("skin.progress bar1"), "skin.bar1_", skinFile, scale);
        selection_gfx.load(skinFile->PullAsString("skin.selection"), "skin.selection_", skinFile, scale);
        option[0].load(skinFile->PullAsString("skin.option0"), "skin.option_", skinFile, scale);
        option[1].load(skinFile->PullAsString("skin.option1"), "skin.option_", skinFile, scale);
        checkbox[0].load(skinFile->PullAsString("skin.checkbox0"), "skin.checkbox_", skinFile, scale);
        checkbox[1].load(skinFile->PullAsString("skin.checkbox1"), "skin.checkbox_", skinFile, scale);

        scroll[0].load(skinFile->PullAsString("skin.v_scroll"), "skin.v_scroll_", skinFile, scale);
        scroll[1].load(skinFile->PullAsString("skin.h_scroll"), "skin.h_scroll_", skinFile, scale);
        scroll[2].load(skinFile->PullAsString("skin.s_scroll"), "skin.s_scroll_", skinFile, scale);

        String tex_file_name (skinFile->PullAsString("skin.window background"));
        if(TA3D::Paths::Exists(tex_file_name))
            wnd_background = gfx->load_texture( tex_file_name, FILTER_LINEAR );
        delete skinFile; 

        GuardLeave();
    }


} // namespace TA3D
