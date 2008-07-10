#ifndef __TA3D_GFX_GUI_SKIN_OBJECT_H__
# define __TA3D_GFX_GUI_SKIN_OBJECT_H__

# include "../../stdafx.h"
# include "../../cTAFileParser.h"


namespace TA3D
{


    /*! \class SKIN_OBJECT
    **
    ** \brief
    */
    class SKIN_OBJECT
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        SKIN_OBJECT();
        //! Destructor
        ~SKIN_OBJECT();
        //@}

        /*!
        ** \brief
        */
        void init();

        /*!
        ** \brief
        */
        void destroy();

        /*!
        ** \brief
        **
        ** \param filename
        ** \param prefix
        ** \param parser
        ** \param borderSize
        */
        void load(const String& filename, const String& prefix, UTILS::cTAFileParser* parser, float borderSize = 1.0f );

        /*!
        ** \brief
        **
        ** \param X1
        ** \param Y1
        ** \param X2
        ** \param Y2
        ** \param bkg
        */
        void draw(const float X1, const float Y1, const float X2, const float Y2, const bool bkg = true) const;

    public:
        //!
        GLuint  tex;

        //!
        float x1;
        //!
        float y1;
        //!
        float x2;
        //!
        float y2;

        //!
        float t_x1;
        //!
        float t_y1;
        //!
        float t_x2;
        //!
        float t_y2;

        //!
        uint32 w;
        //!
        uint32 h;

        //!
        float sw;
        //!
        float sh;

    }; // class SKIN_OBJECT




}


#endif // __TA3D_GFX_GUI_SKIN_OBJECT_H__
