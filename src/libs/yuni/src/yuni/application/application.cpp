
#include "application.h"
#include "../core/io/directory.h"



namespace Yuni
{
namespace Application
{


	IApplication::IApplication(int, char* argv[])
		:pTerminated(false), pExitCode(0)
	{
		String argv0 = argv[0];
		// Find the absolute folder of the application
		if (Core::IO::IsAbsolute(argv[0]))
			Core::IO::ExtractFilePath(pRootFolder, argv0);
		else
		{
			String r;
			Core::IO::Directory::Current::Get(r);
			r << Core::IO::Separator << argv0;
			if (r.notEmpty())
				Core::IO::ExtractFilePath(pRootFolder, r, true);
		}

		// Find The absolution exe name
		if (pRootFolder.empty())
		{
			String r;
			Core::IO::ExtractFileName(r, argv0);
			pExeName += r;
		}
		else
		{
			pExeName << pRootFolder	<< Core::IO::Separator;
			CustomString<20, false> t;
			Core::IO::ExtractFileName(t, argv0);
			pExeName << t;
		}
	}



	void IApplication::arguments(int argc, char** argv)
	{
		GetOpt::Parser parser;
		bool optHelp = false;
		parser.addFlag(optHelp, 'h', "help", "Print this help and exit");

		if (!parser(argc, argv))
		{
			pExitCode = -1;
			pTerminated = true;
			return;
		}

		if (optHelp)
		{
			pTerminated = true;
			parser.helpUsage(argv[0]);
			return;
		}
	}



} // namespace Application
} // namespace Yuni
