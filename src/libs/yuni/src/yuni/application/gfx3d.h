#ifndef __YUNI_APPLICATION_GFX_3D_H__
# define __YUNI_APPLICATION_GFX_3D_H__

# include "application.h"
# include "../core/event/event.h"



namespace Yuni
{
namespace Application
{

	/*!
	** \brief 3D Application
	*/
	class Gfx3D : public Application::IApplication, public IEventObserver<Gfx3D>
	{
	public:
		//! Threading policy
		typedef IEventObserver<Gfx3D>::ThreadingPolicy  ThreadingPolicy;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Constructor
		** \param argc The number of arguments
		** \param argv All arguments
		*/
		Gfx3D(int argc, char* argv[]);

		//! Destructor
		virtual ~Gfx3D();
		//@}


		//! \name Application title
		//@{
		//! Get the application title
		String title() const;

		//! Set the application title
		template<class StringT> void title(const StringT& newTitle);
		//@}


		//! \name Events
		//@{

		/*!
		** \brief What to do on execute.
		*/
		virtual void onExecute();

		//@}


	private:
		//! Application title
		String pTitle;

	}; // class Application::Gfx3D





} // namespace Application
} // namespace Yuni

# include "gfx3d.hxx"

#endif // __YUNI_APPLICATION_GFX_3D_H__
