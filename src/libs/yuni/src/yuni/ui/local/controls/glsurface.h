#ifndef __YUNI_UI_LOCAL_CONTROLS_GLSURFACE_H__
# define __YUNI_UI_LOCAL_CONTROLS_GLSURFACE_H__

# include "../../../yuni.h"
# include "surface.h"


namespace Yuni
{
namespace Private
{
namespace UI
{
namespace Local
{

	/*!
	** \brief Abstract surface for OpenGL Rendering
	*/
    class GLSurface: public Surface
	{
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		GLSurface() {}
		//! Destructor
		virtual ~GLSurface() {}
		//@}

		//! Initialize the surface
		bool initialize();

		//! Resize the surface
		void resize(float width, float height);


	protected:
		//! Make this GL context the current one
		virtual void makeCurrent() = 0;

	}; // class GLSurface




} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

#endif // __YUNI_UI_LOCAL_CONTROLS_GLSURFACE_H__
