#ifndef __YUNI_APPLICATION_CONSOLE_H__
# define __YUNI_APPLICATION_CONSOLE_H__

# include "application.h"



namespace Yuni
{
namespace Application
{


	/*!
	** \brief Console Application
	**
	** \code
	** class HelloWorld : public Yuni::Application::Console
	** {
	** public:
	**	  HelloWorld(nt argc, char* argv[]) : Yuni::Application::Console(argc, argv) {}
	**	  virtual ~HelloWorld() {}
	**
	**	  virtual void execute()
	**	  {
	**		  std::cout << "Hello world" << std::endl;
	**	  }
	** };
	**
	** int main(int argc, char* argv[])
	** {
	**	  HelloWorld app(argc, argv);
	**	  app.execute();
	**	  return app.exitCode();
	** }
	** \endcode
	*/
	class Console : public Application::IApplication
	{
	public:
		//! Ancestor type
		typedef Application::IApplication  AncestorType;
		//! The Threading Policy
		typedef AncestorType::ThreadingPolicy ThreadingPolicy;
		//! Pointer
		typedef Console* Ptr;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Constructor
		** \param argc The number of arguments
		** \param argv All arguments
		*/
		Console(int argc, char* argv[]);
		//! Destructor
		virtual ~Console();
		//@}

	}; // class Application::Console






} // namespace Application
} // namespace Yuni

#endif // __YUNI_APPLICATION_CONSOLE_H__
