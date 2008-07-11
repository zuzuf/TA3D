
#include "object.h"
#include "../gfx.h"
#include "../../TA3D_NameSpace.h"
#include "../../gui.h"




namespace TA3D
{




    GUIOBJ::GUIOBJ()
        :Type(0), Focus(false), Etat(false),
        x1(0.0f), y1(0.0f), x2(0.0f), y2(0.0f), Func(NULL), Data(0),
        Pos(0), Value(-1), s(1.0f), Flag(0),
        MouseOn(false), activated(false), destroy_img(false),
        u1(0.0f), v1(0.0f), u2(1.0f), v2(1.0f),
        wait_a_turn(false), current_state(0), nb_stages(0), shortcut_key(-1)
    {}


    GUIOBJ::~GUIOBJ()
    {
        for (TexturesVector::iterator i = gltex_states.begin(); i != gltex_states.end(); ++i)
            i->destroy();
        if (destroy_img)
        {
            GLuint gl_data = (GLuint) Data;
            gfx->destroy_texture(gl_data);
            Data = 0;
        }
    }



    uint32 GUIOBJ::msg(const String& message, WND* wnd)		// Reacts to a message transfered from the Interface
    {
        String lw(message);
        lw.toLower();
        wait_a_turn = true;

        if (lw == "hide")            { Flag |= FLAG_HIDDEN;          return INTERFACE_RESULT_HANDLED; }
        if (lw == "show")            { Flag &= ~FLAG_HIDDEN;         return INTERFACE_RESULT_HANDLED; }
        if (lw == "switch")          { Flag |= FLAG_SWITCH;          return INTERFACE_RESULT_HANDLED; }
        if (lw == "unswitch")        { Flag &= ~FLAG_SWITCH;         return INTERFACE_RESULT_HANDLED; }
        if (lw == "fill")            { Flag |= FLAG_FILL;            return INTERFACE_RESULT_HANDLED; }
        if (lw == "unfill")          { Flag &= ~FLAG_FILL;           return INTERFACE_RESULT_HANDLED; }
        if (lw == "enable")          { Flag &= ~FLAG_DISABLED;       return INTERFACE_RESULT_HANDLED; }
        if (lw == "disable")         { Flag |= FLAG_DISABLED;        return INTERFACE_RESULT_HANDLED; }
        if (lw == "highlight")       { Flag |= FLAG_HIGHLIGHT;       return INTERFACE_RESULT_HANDLED; }
        if (lw == "unhighlight")     { Flag &= ~FLAG_HIGHLIGHT;      return INTERFACE_RESULT_HANDLED; }
        if (lw == "can_get_focus")   { Flag |= FLAG_CAN_GET_FOCUS;   return INTERFACE_RESULT_HANDLED; }
        if (lw == "cant_get_focus")  { Flag &= ~FLAG_CAN_GET_FOCUS;  return INTERFACE_RESULT_HANDLED; }
        if (lw == "can_be_clicked")  { Flag |= FLAG_CAN_BE_CLICKED;  return INTERFACE_RESULT_HANDLED; }
        if (lw == "cant_be_clicked") { Flag &= ~FLAG_CAN_BE_CLICKED; return INTERFACE_RESULT_HANDLED; }
        if (StartsWith(lw ,"caption=")) // Change the GUIOBJ's caption
        {
            if (!Text.empty())
                Text[0] = message.substr(8, message.size() - 8);
            return INTERFACE_RESULT_HANDLED;
        }
        if (lw == "focus")
        {
            if (wnd)
            {
                for (int i = 0; i < wnd->NbObj; ++i)
                    wnd->Objets[i].Focus = false;
                Focus = true;
            }
            return INTERFACE_RESULT_HANDLED;
        }
        return INTERFACE_RESULT_CONTINUE;
    }



    void GUIOBJ::set_caption(const String& caption)
    {
        switch (Type)
        {
            case OBJ_OPTIONC:
            case OBJ_OPTIONB: x2 = x1 + (int)gui_font.length(caption) + 4;
            case OBJ_TEXT:
            case OBJ_MENU:
            case OBJ_FMENU:
            case OBJ_TEXTBAR:
            case OBJ_BUTTON:  Text[0] = caption;
        }
        if (Type == OBJ_TEXTBAR && Text[0].size() >= Data)
            Text[0].resize(Data - 1);
    }


    void GUIOBJ::create_ta_button(float X1, float Y1, const String::Vector& Caption,
                                  const std::vector<GLuint>& states, int nb_st)
    {
        gltex_states.clear();
        gltex_states.resize(states.size());
        for (unsigned int i = 0 ; i < states.size(); ++i) // Create the texture Vector
            gltex_states[i].set(states[i]);

        Type = OBJ_TA_BUTTON;
        x1 = X1;
        y1 = Y1;
        if (!gltex_states.empty())
        {
            x2 = X1 + gltex_states[0].width;
            y2 = Y1 + gltex_states[0].height;
        }
        else
        {
            x2 = X1;
            y2 = Y1;
        }
        nb_stages = nb_st;
        Etat = false;
        Focus = false;
        current_state = 0;
        Text = Caption;
        Func = NULL;
        s = 1.0f;
        Flag = FLAG_CAN_BE_CLICKED | FLAG_MULTI_STATE;
    }


    void GUIOBJ::create_button(const float X1, const float Y1, const float X2, const float Y2,
                               const String& caption, void (*F)(int), const float size)
    {
        Type = OBJ_BUTTON;
        x1 = X1;
        y1 = Y1;
        x2 = X2;
        y2 = Y2;
        Etat = false;
        Focus = false;
        Text.resize(1);
        Text[0] = caption;
        Func = F;
        s = size;
        Flag = FLAG_CAN_BE_CLICKED;
    }


    void GUIOBJ::create_optionc(const float X1, const float Y1, const String& caption, const bool ETAT,
                                void (*F)(int), SKIN *skin, const float size)
    {
        Type = OBJ_OPTIONC;
        x1 = X1;
        y1 = Y1;

        x2 = X1 +(int)(gui_font.length(caption) * size) + 4;
        y2 = Y1;

        if (skin && skin->checkbox[0].tex && skin->checkbox[1].tex)
        {
            x2 += max( skin->checkbox[0].sw, skin->checkbox[1].sw);
            y2 += max( skin->checkbox[0].sh, skin->checkbox[1].sh);
        }
        else
        {
            x2 += 8;
            y2 += 12;
        }
        Etat = ETAT;	
        Focus = false;
        Text.resize(1);
        Text[0] = caption;
        Func = F;
        Flag = FLAG_SWITCH | FLAG_CAN_BE_CLICKED;
        s = size;
    }



    void GUIOBJ::create_optionb(const float X1, const float Y1, const String& Caption, const bool ETAT,
                                void (*F)(int), SKIN* skin, const float size)
    {
        Type = OBJ_OPTIONB;
        x1 = X1;
        y1 = Y1;

        x2 = X1 + (int)(gui_font.length(Caption) * size) + 4;
        y2 = Y1;
        
        if (skin && skin->option[0].tex && skin->option[1].tex)
        {
            x2 += max( skin->option[0].sw, skin->option[1].sw);
            y2 += max( skin->option[0].sh, skin->option[1].sh);
        }
        else
        {
            x2 += 8;
            y2 += 12;
        }
        Etat = ETAT;	
        Focus = false;
        Text.resize(1);
        Text[0] = Caption;
        Func = F;
        Flag = FLAG_SWITCH | FLAG_CAN_BE_CLICKED;
        s = size;
    }



    void GUIOBJ::create_textbar(const float X1, const float Y1, const float X2, const float Y2, const String& Caption,
                                const unsigned int MaxChar, void(*F)(int), const float size)
    {
        Type = OBJ_TEXTBAR;
        x1 = X1;
        y1 = Y1;
        x2 = X2;
        y2 = Y2;
        Etat  = false;	
        Focus = false;
        Text.resize(1);
        Text[0] = Caption;
        if (Text[0].size() >= MaxChar && MaxChar > 1)
            Text[0].resize(MaxChar - 1);
        Flag = FLAG_CAN_BE_CLICKED | FLAG_CAN_GET_FOCUS;
        Func = F;
        Data = MaxChar;
        s = size;
    }


    void GUIOBJ::create_menu(const float X1, const float Y1,const String::Vector& Entry,
                             void (*F)(int), const float size)
    {
        Type = OBJ_FMENU;
        x1 = X1;
        y1 = Y1;
        x2 = X1 + 168;
        y2 = (int)(Y1 + gui_font_h * size * Entry.size() + gui_font_h * size);
        Etat = false;	
        Focus = false;
        Text = Entry;
        Func = F;
        Flag = FLAG_CAN_BE_CLICKED | FLAG_CAN_GET_FOCUS;
        s = size;
    }



    void GUIOBJ::create_menu(const float X1, const float Y1, const float X2, const float Y2,
                             const String::Vector& Entry, void (*F)(int), const float size)
    {
        Type = OBJ_MENU;
        x1 = X1;
        y1 = Y1;
        x2 = X2;
        y2 = Y2;
        Etat = false;	
        Focus = false;
        Text = Entry;
        Pos = 0; // Position sur la liste
        Func = F;
        Flag = FLAG_CAN_BE_CLICKED | FLAG_CAN_GET_FOCUS;
        s = size;
    }



    // Crée une barre de progression
    void GUIOBJ::create_pbar(const float X1, const float Y1, const float X2, const float Y2,
                             const int PCent, const float size)
    {
        Type = OBJ_PBAR;
        x1 = X1;
        y1 = Y1;
        x2 = X2;
        y2 = Y2;
        Etat = false;	
        Focus = false;
        Text.clear();
        Func = NULL;
        Data = PCent;
        Flag = 0;
        s = size;
    }



    // Crée un objet text
    void GUIOBJ::create_text(const float X1, const float Y1, const String& Caption, const int Col, const float size)
    {
        Type = OBJ_TEXT;
        x1 = X1;
        y1 = Y1;
        x2 = (int)(X1 + Caption.length() * 8 * size);
        y2 = (int)(Y1 + gui_font_h * size);
        Etat = false;	
        Focus = false;
        Text.resize(1);
        Text[0] = Caption;
        Func = NULL;
        Data = Col;
        s = size;
        Flag = 0;
    }

    void GUIOBJ::create_line(const float X1, const float Y1, const float X2, const float Y2, const int Col)
    {
        Type = OBJ_LINE;
        x1 = X1;
        y1 = Y1;
        x2 = X2;
        y2 = Y2;
        Etat = false;	
        Focus = false;
        Text.clear();
        Func = NULL;
        Data = Col;
    }

    void GUIOBJ::create_box(const float X1, const float Y1, const float X2, const float Y2, const int Col)
    {
        Type = OBJ_BOX;
        x1 = X1;
        y1 = Y1;
        x2 = X2;
        y2 = Y2;
        Etat = false;	
        Focus = false;
        Text.clear();
        Func = NULL;
        Data = Col;
        Flag = FLAG_CAN_BE_CLICKED;
    }



    void GUIOBJ::create_img(const float X1, const float Y1, const float X2, const float Y2, const GLuint img)
    {
        Type = OBJ_IMG;
        x1 = X1;
        y1 = Y1;
        x2 = X2;
        y2 = Y2;
        Etat = false;	
        Focus = false;
        Text.clear();
        Func = NULL;
        Data = (uint32)img;
        Flag = FLAG_CAN_BE_CLICKED;
    }


    void GUIOBJ::create_list(const float X1, const float Y1, const float X2, const float Y2, const String::Vector& Entry, const float size)
    {
        Type = OBJ_LIST;
        x1 = X1;
        y1 = Y1;
        x2 = X2;
        y2 = Y2;
        Etat = false;	
        Focus = false;
        Text = Entry;
        Func = NULL;
        Data = 0;
        Pos = 0;
        Flag = FLAG_CAN_BE_CLICKED | FLAG_CAN_GET_FOCUS; // To detect when something has changed
        s = size;
    }



} // namespace TA3D
