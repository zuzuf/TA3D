#ifndef __TA3D_GFX_SHADER_H__
# define __TA3D_GFX_SHADER_H__

# include "../stdafx.h"


namespace TA3D
{
// namespace GFX
// {


    /*! \class SHADER
    **
    ** \brief Shader
    */
    class SHADER
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        SHADER() :succes(false) {}
        //! Destructor
        ~SHADER() {destroy();}
        //@}
        
        void destroy();



        void load(const char* fragment_file, const char*vertex_file);

        void load_memory(const char* fragment_data, int frag_len, const char *vertex_data, int vert_len);


        void on();

        void off();

        void setvar1f(const char *var_name,float v0);

        void setvar2f(const char *var_name,float v0,float v1);

        void setvar3f(const char *var_name,float v0,float v1,float v2);

        void setvar4f(const char *var_name,float v0,float v1,float v2,float v3);

        void setvar1i(const char *var_name,int v0);

        void setvar2i(const char *var_name,int v0,int v1);

        void setvar3i(const char *var_name,int v0,int v1,int v2);

        void setvar4i(const char *var_name,int v0,int v1,int v2,int v3);

    public:
        GLhandleARB		program;
        GLhandleARB		fragment;
        GLhandleARB		vertex;
        bool			succes;

    }; // class SHADER




    // } // namespace GFX
} // namespace TA3D

#endif // __TA3D_GFX_SHADER_H__
