#ifndef __LIBYUNI_CONFIG_PROGRAM_H__
# define __LIBYUNI_CONFIG_PROGRAM_H__

# ifndef YUNI_NO_THREAD_SAFE
#	define YUNI_NO_THREAD_SAFE // disabling thread-safety
# endif
# include <yuni/yuni.h>
# include <yuni/core/string.h>
# include <yuni/core/getopt.h>
# include <iostream>
# include <list>
# include <set>
# include "versions.h"
# include <yuni/core/io/directory.h>
# include <algorithm>



namespace Yuni
{

	class LibConfigProgram
	{
	public:
		LibConfigProgram();

		int execute(int argc, char** argv);

		bool debug() const {return pOptDebug;}

	private:
		void findRootPath(const char* a0);

		bool parseCommandLine(int argc, char** argv);

		void cleanPrefixPaths();

		void initializeSystemPathList();

		void normalizeCompiler();

		void expandModuleNames();

		bool displayInformations();

		void displayInformationAboutYuniVersion();

		void computeDependencies(LibConfig::VersionInfo::Settings& version);

		void printModulesDependencies() const;

		bool isCoreModule(const String& name) const;

		bool checkForDependencies(LibConfig::VersionInfo::Settings& version);

		bool displayInformationAboutYuniVersion(LibConfig::VersionInfo::Settings& version);

		void createArguments(LibConfig::VersionInfo::Settings& version) const;


	private:
		//! Exit status
		int pExitStatus;
		//! The Root path
		String pRootPath;

		//! Flag: Print the version and exit
		bool pOptVersion;
		//! Flag: Print all available versions of libyuni (with info) and exit
		bool pOptList;
		//! Flag: Print all available versions of libyuni and exit
		bool pOptListOnlyVersions;
		//! Flag: Do not use default paths
		bool pOptNoDefaultPath;
		//! Flag: Print all available modules of the selected versions
		bool pOptModuleList;
		//! Flag: Print all default paths and exit
		bool pOptDefaultPathList;
		//! Flag: Print cxx flags
		bool pOptCxxFlags;
		//! Flag: Print lib flags
		bool pOptLibFlags;
		//! Flag: Print the default compiler
		bool pOptPrintCompilerByDefault;
		//! Flag: Verbose
		bool pOptPrintErrors;
		//! Flag: Print all modules and exit
		bool pOptPrintModulesDeps;
		//! Flag: Debug
		bool pOptDebug;

		//! List of required modules
		String::List pOptModules;
		//! List of given prefix
		String::List pOptPrefix;
		//! List of default paths
		String::List pDefaultPathList;
		//! The complete list of known libyuni versions
		LibConfig::VersionInfo::List pVersionList;
		//! The compiler to use
		String pOptCompiler;

	}; // class Options





} // namespace Yuni

# include "program.hxx"

#endif // __LIBYUNI_CONFIG_PROGRAM_H__
