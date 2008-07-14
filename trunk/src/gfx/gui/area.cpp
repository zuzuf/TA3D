
#include "area.h"
#include "../../misc/paths.h"
#include "skin.h"
#include "../../TA3D_Exception.h"
#include "../../console.h"
#include "../../TA3D_NameSpace.h"
#include "../../gui.h"
#include "../../misc/math.h"


using namespace TA3D::Exceptions;


namespace TA3D
{

    WND	*AREA::doGetWnd(const String& message)
    {
        String lmsg (message);
        lmsg.toLower();
        if (lmsg == cached_key && cached_wnd)
            return cached_wnd;
        sint16 e = wnd_hashtable.Find(lmsg) - 1;
        if (e >= 0)
        {
            cached_key = lmsg;
            cached_wnd = vec_wnd[e];
            return cached_wnd;
        }
        return NULL;
    }



    WND	*AREA::get_wnd(const String& message)
    {
        MutexLocker locker(pMutex);
        return doGetWnd(message);
    }


    void AREA::set_enable_flag(const String& message, const bool enable)
    {
        pMutex.lock();
        GUIOBJ* guiobj = doGetObject(message);
        if (guiobj)
        {
            if (enable)
                guiobj->Flag &= ~FLAG_DISABLED;
            else
                guiobj->Flag |= FLAG_DISABLED;
        }
        pMutex.unlock();
    }


    void AREA::set_state(const String& message, const bool state)
    {
        pMutex.lock();
        GUIOBJ* guiobj = doGetObject(message);
        if (guiobj)
            guiobj->Etat = state;
        pMutex.unlock();
    }

    void AREA::set_value(const String& message, const sint32 value)
    {
        pMutex.lock();
        GUIOBJ* guiobj = doGetObject(message);
        if (guiobj)
            guiobj->Value = value;
        pMutex.unlock();
    }

    void AREA::set_data(const String& message, const sint32 data)
    {
        pMutex.lock();
        GUIOBJ* guiobj = doGetObject(message);
        if (guiobj)
            guiobj->Data = data;
        pMutex.unlock();
    }

    void AREA::set_caption(const String& message, const String& caption)
    {
        pMutex.lock();
        GUIOBJ* guiobj = doGetObject(message);
        if (guiobj && guiobj->Text.size() > 0)
        {
            if (guiobj->Flag & FLAG_CENTERED)
            {
                float length = gui_font.length(guiobj->Text[0]) * guiobj->s;
                guiobj->x1 += length * 0.5f;
                guiobj->x2 -= length * 0.5f;
            }
            guiobj->Text[0] = caption;
            if (guiobj->Flag & FLAG_CENTERED)
            {
                float length = gui_font.length(guiobj->Text[0]) * guiobj->s;
                guiobj->x1 -= length * 0.5f;
                guiobj->x2 += length * 0.5f;
            }
        }
        pMutex.unlock();
    }


    uint16 AREA::check()
    {
        poll_mouse();
        poll_keyboard();
        bool scroll = ((msec_timer - scroll_timer) >= 250);
        if (scroll)
        {
            while (msec_timer - scroll_timer >= 250)
                scroll_timer += 250;
        }

        uint16 is_on_gui = 0;
        pMutex.lock();
        for (uint16 i = 0; i < vec_wnd.size(); ++i)
        {
            if (!is_on_gui || (vec_wnd[vec_z_order[i]]->get_focus && !vec_wnd[vec_z_order[i]]->hidden))
            {
                is_on_gui |= vec_wnd[vec_z_order[i]]->check(amx, amy, amz, amb, scroll, skin);
                if (((is_on_gui && mouse_b && !vec_wnd[vec_z_order[0]]->get_focus)
                     || vec_wnd[ vec_z_order[i]]->get_focus) && i > 0 && !vec_wnd[ vec_z_order[i]]->background_wnd)
                {
                    uint16 old = vec_z_order[i];
                    for (uint16 e = i; e > 0; --e)
                        vec_z_order[e] = vec_z_order[e - 1];
                    vec_z_order[0] = old; // Get the focus
                }
            }
        }

        if (Console == NULL || !Console->activated())
            clear_keybuf();

        scrolling = scroll;
        amx = mouse_x;
        amy = mouse_y;
        amz = mouse_z;
        amb = mouse_b;
        pMutex.unlock();

        return is_on_gui;
    }



    uint16 AREA::load_window(const String& filename)
    {
        MutexLocker locker(pMutex);

        uint16 wnd_idx = vec_wnd.size();
        vec_wnd.push_back(new WND());				// Adds a window to the vector
        vec_z_order.push_back(wnd_idx);

        if (Paths::ExtractFileExt(filename) == ".gui")
            vec_wnd[wnd_idx]->load_gui(filename, gui_hashtable); // Loads the window from a *.gui file
        else
            vec_wnd[wnd_idx]->load_tdf(filename, skin);	// Loads the window from a *.tdf file

        for (uint16 i = wnd_idx; i > 0; --i)		// The new window appear on top of the others
            vec_z_order[i] = vec_z_order[i - 1];
        vec_z_order[0] = wnd_idx;
        wnd_hashtable.Insert(String::ToLower(vec_wnd[wnd_idx]->Name), wnd_idx + 1);	// + 1 because it returns 0 on Find failure
        return wnd_idx;
    }


    void AREA::draw()
    {
        pMutex.lock();
        if (background)
        {
            gfx->drawtexture(background, 0, 0, gfx->width, gfx->height);
            glDisable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        String help_msg;
        // Draws all the windows in focus reversed order so the focused window is drawn on top of the others
        for (sint32 i = vec_wnd.size() - 1; i >=0 ; --i)			
            vec_wnd[vec_z_order[i]]->draw(help_msg, i == 0, true, skin);
        if( !help_msg.empty())
            PopupMenu( mouse_x + 20, mouse_y + 20, help_msg, skin );

        pMutex.unlock();
    }

    void AREA::load_tdf(const String& filename)
    {
        doLoadTDF(filename);
    }

    void AREA::doLoadTDF(const String& filename)
    {
        GuardEnter(AREA::load_tdf);
        destroy();		// In case there is an area loaded so we don't waste memory
        cTAFileParser* areaFile;

        String skin_name = (lp_CONFIG != NULL && !lp_CONFIG->skin_name.empty()) ? lp_CONFIG->skin_name : "";
        if (!skin_name.empty() && TA3D::Paths::Exists(skin_name))
        {
            skin = new SKIN();
            skin->load_tdf(skin_name, 1.0f);
        }

        try  // we need to try catch this cause the config file may not exists
            // and if it don't exists it will throw an error on reading it, which
            // will be caught in our main function and the application will exit.
        {
            String real_filename = filename;
            if (skin != NULL && !skin->prefix.empty())
            {
                int name_len = strlen(get_filename(real_filename.c_str()));
                if (name_len > 0)
                {
                    real_filename.clear();
                    real_filename << filename.substr(0, filename.size() - name_len) << skin->prefix
                        << get_filename(filename.c_str());
                }
                else
                    real_filename << skin->prefix;
                if (!HPIManager->Exists(real_filename))	// If it doesn't exist revert to the default name
                    real_filename = filename;
            }
            if (skin)
                delete skin;
            skin = NULL;
            areaFile = new TA3D::UTILS::cTAFileParser(real_filename);
        }
        catch( ... )
        {
            GuardLeave();
            return;
        }

        name = filename;		// Grab the area's name
        String::size_type e = name.find(".");		// Extracts the file name

        if (e != String::npos)
            name = name.substr( 0, e );
        e = name.find_last_of( "/\\" );
        if (e != String::npos)
            name = name.substr(e + 1, name.size() - e - 1);

        name = areaFile->pullAsString("area.name", name);					// The TDF may override the area name
        skin_name = (lp_CONFIG != NULL && !lp_CONFIG->skin_name.empty())
            ? lp_CONFIG->skin_name
            : areaFile->pullAsString("area.skin");

        if (TA3D::Paths::Exists(skin_name)) // Loads a skin
        {
            int area_width = areaFile->pullAsInt("area.width", SCREEN_W);
            int area_height = areaFile->pullAsInt("area.height", SCREEN_W);
            float skin_scale = Math::Min((float)SCREEN_H / area_height, (float)SCREEN_W / area_width);
            skin = new SKIN();
            skin->load_tdf(skin_name, skin_scale);
        }

        String::Vector windows_to_load;
        ReadVectorString(windows_to_load, areaFile->pullAsString("area.windows"));
        for(String::Vector::iterator i = windows_to_load.begin(); i != windows_to_load.end(); ++i)
            load_window(*i);

        String background_name = areaFile->pullAsString("area.background");
        if(skin && !skin->prefix.empty())
        {
            int name_len = strlen(get_filename(background_name.c_str()));
            if (name_len > 0)
                background_name = background_name.substr(0, background_name.size() - name_len) + skin->prefix + get_filename(background_name.c_str());
            else
                background_name += skin->prefix;
        }

        if (TA3D::Paths::Exists(background_name)) // Loads a background image
            background = gfx->load_texture(background_name);
        else
        {
            if (skin && !skin->prefix.empty())
            {
                // No prefixed version, retry with default background
                background_name = areaFile->pullAsString("area.background"); 
                // Loads a background image
                if (TA3D::Paths::Exists(background_name)) 
                    background = gfx->load_texture( background_name );
            }
        }
        delete areaFile; 
        GuardLeave();
    }



    AREA::AREA(const String& area_name)
        :gui_hashtable(), wnd_hashtable()
    {
        cached_wnd = NULL;
        name = area_name;		// Gives it a name

        vec_wnd.clear();		// Starts with an empty vector
        vec_z_order.clear();	// No windows at start

        background = 0;			// By default we have no background

        amx = mouse_x;
        amy = mouse_y;
        amz = mouse_z;
        amb = mouse_b;

        skin = NULL;			// Default: no skin

        scroll_timer = msec_timer;
        scrolling = false;

        InitInterface();		// Initialization of the interface
    }


    void AREA::destroy()
    {
        cached_key.clear();
        cached_wnd = NULL;

        gui_hashtable.EmptyHashTable();
        gui_hashtable.InitTable( __DEFAULT_HASH_TABLE_SIZE );

        wnd_hashtable.EmptyHashTable();
        wnd_hashtable.InitTable( __DEFAULT_HASH_TABLE_SIZE );

        name.clear();

        for (std::vector<WND*>::iterator i = vec_wnd.begin(); i != vec_wnd.end(); ++i)
            delete *i;
        vec_wnd.clear();			// Empty the window vector
        vec_z_order.clear();		// No more windows at end

        gfx->destroy_texture(background);		// Destroy the texture (using safe destroyer)

        if (skin) // Destroy the skin
            delete skin;
        skin = NULL;
    }


    AREA::~AREA()
    {
        DeleteInterface();			// Shut down the interface

        cached_key.clear();
        cached_wnd = NULL;
        gui_hashtable.EmptyHashTable();
        wnd_hashtable.EmptyHashTable();
        name.clear();

        for (std::vector<WND*>::iterator i = vec_wnd.begin(); i != vec_wnd.end(); ++i)
            delete *i;
        vec_wnd.clear();			// Empty the window vector
        vec_z_order.clear();		// No more windows at end

        gfx->destroy_texture(background); // Destroy the texture
        if (skin)					// Destroy the skin
            delete skin;
    }

    uint32 AREA::InterfaceMsg(const lpcImsg msg)
    {
        if (msg->MsgID != TA3D_IM_GUI_MSG) // Only GUI messages
            return INTERFACE_RESULT_CONTINUE;

        if (msg->lpParm1 == NULL)
        {
            GuardInfo( "AREA : bad format for interface message!\n" );
            return INTERFACE_RESULT_HANDLED;		// Oups badly written things
        }
        return this->msg((char*) msg->lpParm1);
    }

    int	AREA::msg(String message)				// Send that message to the area
    {
        pMutex.lock();

        uint32 result = INTERFACE_RESULT_CONTINUE;
        message.toLower(); // Get the string associated with the signal

        String::size_type i = message.find('.');
        if (i != String::npos)
        {
            String key = message.substr(0, i); // Extracts the key
            message = message.substr(i + 1, message.size() - i - 1); // Extracts the end of the message

            WND* the_wnd = doGetWnd(key);
            if (the_wnd)
                result = the_wnd->msg(message);
        }
        else
        {
            if (message == "clear")
            {
                for (std::vector<WND*>::iterator i = vec_wnd.begin(); i != vec_wnd.end(); ++i)
                    delete *i;
                vec_wnd.clear();
                vec_z_order.clear();
                wnd_hashtable.EmptyHashTable();
                wnd_hashtable.InitTable( __DEFAULT_HASH_TABLE_SIZE );
            }
            else
                if (message == "end_the_game")
                {
                    // TODO Code here ?
                }
        }
        pMutex.unlock();
        return result;				// Ok we're done with it
    }

    bool AREA::get_state(const String& message)
    {
        MutexLocker locker(pMutex);

        String::size_type i = message.find('.');
        if (i != String::npos)
        {
            String key = message.substr(0, i); // Extracts the key
            String obj_name = message.substr(i + 1, message.size() - i - 1);

            if (key == "*")
            {
                for ( uint16 e = 0; e < vec_wnd.size(); ++e)// Search the window containing the object corresponding to the key
                {
                    GUIOBJ* the_obj = vec_wnd[e]->get_object(obj_name);
                    if (the_obj)
                        return the_obj->Etat;
                }
            }
            else
            {
                WND* the_wnd = doGetWnd(key);
                if (the_wnd)
                    return the_wnd->get_state(obj_name);
            }
        }
        else
        {
            WND* the_wnd = doGetWnd(message);
            if (the_wnd)
                return the_wnd->get_state("");
        }
        return false;
    }


    sint32 AREA::get_value(const String& message)
    {
        MutexLocker locker(pMutex);

        String::size_type i = message.find('.');
        if (i != String::npos)
        {
            String key = message.substr(0, i);
            String obj_name = message.substr(i + 1, message.size() - i - 1);
            if (key == "*")
            {
                for (uint16 e = 0; e < vec_wnd.size(); ++e)	// Search the window containing the object corresponding to the key
                {
                    GUIOBJ *the_obj = vec_wnd[e]->get_object(obj_name);
                    if (the_obj)
                        return the_obj->Value;
                }
            }
            else
            {
                WND* the_wnd = doGetWnd(key);
                if (the_wnd)
                    return the_wnd->get_value(obj_name);
            }
        }
        return -1;
    }

    String AREA::get_caption(const String& message)
    {
        MutexLocker locker(pMutex);

        String::size_type i = message.find('.');
        if (i != String::npos)
        {
            String key = message.substr(0, i);						// Extracts the key
            String obj_name = message.substr(i + 1, message.size() - i - 1);
            if (key == "*")
            {
                for (uint16 e = 0; e < vec_wnd.size(); ++e)// Search the window containing the object corresponding to the key
                {
                    GUIOBJ* the_obj = vec_wnd[e]->get_object(obj_name);
                    if (the_obj)
                    {
                        if (the_obj->Text.size() > 0)
                            return the_obj->Text[0];	// Return what we found
                        return "";
                    }
                }
            }
            else
            {
                WND* the_wnd = doGetWnd(key);
                if (the_wnd)
                    return the_wnd->get_caption( obj_name );
            }
        }
        return "";
    }



    GUIOBJ* AREA::doGetObject(const String& message)
    {
        String::size_type i = message.find('.');
        if (i != String::npos)
        {
            String key = message.substr(0, i);						// Extracts the key
            String obj_name = message.substr(i + 1, message.size() - i -1);
            if (key == "*")
            {
                for (uint16 e = 0; e < vec_wnd.size(); ++e)	// Search the window containing the object corresponding to the key
                {
                    GUIOBJ *the_obj = vec_wnd[e]->get_object(obj_name);
                    if (the_obj)
                        return the_obj;
                }
            }
            else
            {
                WND* the_wnd = doGetWnd(key);
                if (the_wnd)
                    return the_wnd->get_object(obj_name);
            }
        }
        return NULL;
    }



    GUIOBJ* AREA::get_object(const String& message, bool /*skip_hidden*/)
    {
        MutexLocker locker(pMutex);
        return doGetObject(message);
    }



} // namespace TA3D
