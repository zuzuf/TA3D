#ifndef __YUNI_APPLICATION_APPLICATION_H__
# define __YUNI_APPLICATION_APPLICATION_H__

# include "../yuni.h"
# include "../thread/policy.h"
# include "../core/getopt.h"
# include "../core/getopt/parser.h"


namespace Yuni
{
namespace Application
{


	class IApplication : public Policy::ObjectLevelLockable<IApplication>
	{
	public:
		//! The Threading Policy
		typedef Policy::ObjectLevelLockable<IApplication>  ThreadingPolicy;
		//! Pointer
		typedef IApplication* Ptr;


	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Constructor
		*/
		IApplication(int argc, char* argv[]);
		//! Destructor
		virtual ~IApplication();
		//@}


		//! \name Paths & Filenames
		//@{
		/*!
		** \brief The absolute filename of the application
		*/
		const String& exeName() const;

		/*!
		** \brief The absolute root folder of the application
		*/
		const String& rootFolder() const;
		//@}


		//! \name Exit status
		//@{
		/*!
		** \brief Get if the application should stop as soon as possible
		*/
		bool terminated() const;

		/*!
		** \brief Indicate that the application should stop as soon as possible
		**
		** \param ex The new exit code value
		*/
		void terminate(const int ex = 0);

		/*!
		** \brief Get the exit code to use
		*/
		int exitCode() const;
		//@}


		//! \name Execute
		//@{
		/*!
		** \brief Execute the application
		*/
		void execute();
		//@}


		/*!
		** \brief Default command line options
		**
		** ReImplement this method to configure your own command line options.
		** \param parser A GetOpt parser
		** \param argc The number of arguments
		** \param argv A null-terminated list of arguments (in UTF8)
		*/
		virtual void arguments(int argc, char** argv);

		virtual void onExecute() = 0;

	private:
		//! The private default constructor
		IApplication();
		//! The private copy constructor
		IApplication(const IApplication&);
		//! The private copy operator
		IApplication& operator = (const IApplication&);

	private:
		//! Indicates if the application should stop as soon as possible
		ThreadingPolicy::Volatile<bool>::Type pTerminated;
		//! The full filename of the application
		// This variable won't be modified after the constructor
		String pExeName;
		//! The root folder of the application
		// This variable won't be modified after the constructor
		String pRootFolder;
		//! Exit code
		int pExitCode;

	}; // class AApplication






} // namespace Application
} // namespace Yuni

# include "application.hxx"

#endif // __YUNI_APPLICATION_APPLICATION_H__
