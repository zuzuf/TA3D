
#include <yuni/yuni.h>
#include <yuni/core/getopt.h>
#include <iostream>


using namespace Yuni;


/*
** How to try the example from the command line :
**
** To Display the help :
** ./getopt_00_simple -h
** ./getopt_00_simple --help
** ./getopt_00_simple /?      (Windows only)
** ./getopt_00_simple /help   (Windows only)
**
** To test the example of an option :
** ./getopt_00_simple --text="Lorem Ipsum"
** ./getopt_00_simple --text "Lorem Ipsum"
** ./getopt_00_simple -t "Lorem Ipsum"
** ./getopt_00_simple -t "Lorem Ipsum" --verbose
** ./getopt_00_simple -v
** ./getopt_00_simple /text "Lorem Ipsum"  (Windows only)
** ./getopt_00_simple /t "Lorem Ipsum"     (Windows only)
**
** You can try an unknown option :
** ./getopt_00_simple --unknown-option
*/



int main(int argc, char* argv[])
{
	// The command line options parser
	GetOpt::Parser options;

	// A simple option : --title="<X>" or -t "<X>"
	// We have to provide the variable where the given value on the command line
	// will be written
	String optText = "A default value";
	// Adding the option to the parser :
	// <variable>    : the variable where the user data will be written
	// 't'           : The short name of the option (./getopt_00_simple -t "my value")
	// "text"        : The long name of the option  (./getopt_00_simple --text "my value")
	// <description> : The description of the option, that will be displayed in the help usage
	options.add(optText, 't', "title", "An option with a parameter");

	// A simple flag, enabled when the option is present on the command line
	// Example: ./getopt_00_simple --verbose
	bool optVerbose = false;
	options.addFlag(optVerbose, 'v', "verbose", "A simple flag");


	// Ask to the parser to parse the command line
	if (!options(argc, argv))
	{
		// The program should not continue here
		// The user may have requested the help or an error has happened
		// If an error has happened, the exit status should be different from 0
		if (options.errors())
		{
			std::cout << "Abort due to error" << std::endl;
			return 1;
		}
		return 0;
	}

	// Displaying the value of our variable on the standard output
	std::cout << "Value for `optText` : `" << optText << "`" << std::endl;
	std::cout << "Verbose             : " << optVerbose << std::endl;

	return 0;
}
