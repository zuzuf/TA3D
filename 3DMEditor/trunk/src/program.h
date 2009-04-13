#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "types.h"

class Program
{
public:
    //! \name Constructor & Destructor
    //@{
    //! Default constructor
    Program() :pLoaded(false) {}
    //! Destructor
    ~Program()
    {
        destroy();
    }
    //@}

    void destroy();


    /*!
    ** \brief Load programs from source files
    */
    QString load(const QString &programFilename);

    QString load_memory(const QString &vertexProgram, const QString &fragmentProgram);

    /*!
    ** \brief On, Off ?
    */
    bool isOn();


    /*!
    ** \brief Activate the program
    */
    void on();

    /*!
    ** \brief Deactivate the program
    */
    void off();


    //! \name Some functions to set shader variables
    //@{
    void setvar1f(const char* var, const float v0);
    void setvar2f(const char* var, const float v0, const float v1);
    void setvar3f(const char* var, const float v0, const float v1, const float v2);
    void setvar4f(const char* var, const float v0, const float v1, const float v2, const float v3);
    void setvar1i(const char* var, const int v0);
    void setvar2i(const char* var, const int v0, const int v1);
    void setvar3i(const char* var, const int v0, const int v1, const int v2);
    void setvar4i(const char* var, const int v0, const int v1, const int v2, const int v3);

    void setmat4f(const char* var, const GLfloat *mat);
    //@}

    /*!
    ** \brief Get if the shader has been loaded by the previous call to `load` or `load_memory`
    **
    ** \see load()
    ** \see load_memory()
    */
    inline bool isLoaded() const
    {
        return pLoaded;
    }


private:
    //! program state ?
    bool pOn;

    //! Is it loaded ?
    bool pLoaded;
    //!
    GLhandleARB		pProgram;
    //!
    GLhandleARB		pFragmentProgram;
    //!
    GLhandleARB		pVertexProgram;

}; // class Program

#endif // __PROGRAM_H__
