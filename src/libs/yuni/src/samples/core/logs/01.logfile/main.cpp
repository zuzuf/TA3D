
#include <yuni/yuni.h>
#include <yuni/core/logs.h>


int main(void)
{
	// To enable the writing into a log file, we have to set the handlers to use
	// We want to write to the stdcout _and_ a log file
	typedef Yuni::Logs::StdCout< Yuni::Logs::File<> >  MyLogHandlers;

	// Our logger
	Yuni::Logs::Logger<MyLogHandlers>  logs;

	// Creating the log file (in the current directory, read-write access required)
	logs.outputFilename("./sample.log");
	if (!logs.logFileIsOpened())
	{
		// An error has occured. That means the log file could not be opened for
		// writing for some reason
		// In this sample, we will continue anyway
		logs.error() << "Impossible to open the log file !";
	}
	else
		logs.info() << "Log file : " << logs.outputFilename();

	// Hello, world !, all standard verbosity levels
	logs.checkpoint() << "Hello, " << "world !";
	logs.notice() << "Hello, " << "world !";
	logs.warning() << "Hello, " << "world !";
	logs.error() << "Hello, " << "world !";
	logs.debug() << "Hello, " << "world !";
	logs.fatal() << "Hello, " << "world !";

	return 0;
}
