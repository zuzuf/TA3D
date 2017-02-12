
#include <yuni/yuni.h>
#include <yuni/core/getopt.h>
#include <iostream>


using namespace Yuni;


/*
** How to try the example from the command line :
**
** To Display the help :
** ./getopt_02_remainingargs -h
** ./getopt_02_remainingargs --help
** ./getopt_02_remainingargs /?      (Windows only)
** ./getopt_02_remainingargs /help   (Windows only)
**
** To test all remaining arguments :
** ./getopt_02_remainingargs -t "Lorem Ipsum" /path/to/somewhere  "/another path/to/somewhere"
**
**
** You can try an unknown option :
** ./getopt_02_remainingargs --unknown-option
*/



int main(int argc, char* argv[])
{
	// The command line options parser
	GetOpt::Parser options;

	// A simple option : --title="<X>" or -t "<X>"
	String optText = "A default value";
	options.add(optText, 't', "title", "An example of an option");

	// Catch all remaining arguments (arguments that does not belong to any option,
	// like a list of paths)
	String::Vector remainingArgs;
	options.remainingArguments(remainingArgs);

	// Ask to the parser to parse the command line
	if (!options(argc, argv))
	{
		// The program should not continue here
		return (options.errors() ? 1 /* an error has occured */ : 0);
	}

	// Displaying the value of our variable on the standard output
	std::cout << "Value for `optText` : `" << optText << "`" << std::endl;

	// Displaying all remaining arguments, if any
	if (!remainingArgs.empty())
	{
		const String::Vector::const_iterator end = remainingArgs.end();
		for (String::Vector::const_iterator i = remainingArgs.begin(); i != end; ++i)
			std::cout << " Remains : `" << *i << "`" << std::endl;
	}
	else
	{
		std::cout << "No remaining options" << std::endl;
	}

	return 0;
}
