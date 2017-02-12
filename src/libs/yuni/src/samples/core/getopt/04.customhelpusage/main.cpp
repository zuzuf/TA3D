
#include <yuni/yuni.h>
#include <yuni/core/getopt.h>
#include <iostream>


using namespace Yuni;


/*
** This sample shows how to write its own help usage
** from scratch.
** If you only want to add some text paragraph, please refer to `03.paragraph`
*/


/*
** How to try the example from the command line :
**
** To Display the help :
** ./getopt_04_customhelpusage -h
** ./getopt_04_customhelpusage --help
** ./getopt_04_customhelpusage /?      (Windows only)
** ./getopt_04_customhelpusage /help   (Windows only)
*/



int main(int argc, char* argv[])
{
	// The command line options parser
	GetOpt::Parser options;

	// A simple option : --text
	String optText;
	options.add(optText, 't', "text");

	// For the help, we want to display _our_ help usage
	// For that, we have to override the option `--help`
	bool optHelp = false;
	options.addFlag(optHelp, 'h', "help");

	// But, do not forget to do the same thing for `?` on
	// Windows, for compatibility with other programs.
	// Example: getopt_04_customhelpusage.exe /?
	# ifdef YUNI_OS_WINDOWS
	// It is safe to use the same variable `optFlag`
	options.addFlag(optHelp, '?');
	# endif


	// Ask the parser to parse the command line
	if (!options(argc, argv))
	{
		// The program should not continue here
		return (options.errors() ? 1 /* error */ : 0);
	}


	if (optHelp)
	{
		std::cout <<
			"This is our custom help usage.\n"
			"Here is the list of options :\n\n"
			" -t, --text=VALUE : A text to display\n\n";
		return 0;
	}

	std::cout << "Text : `" << optText << "`" << std::endl;

	return 0;
}
