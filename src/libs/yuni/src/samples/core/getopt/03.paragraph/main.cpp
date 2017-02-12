
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
** ./getopt_00_simple --width=1024 --height=768
** ./getopt_00_simple --width=1024 --height=768 --fullscreen=yes
** ./getopt_00_simple --width=1024 --height=768 --fullscreen=true
** ./getopt_00_simple --width=1024 --height=768 --fullscreen=true --language=ru
** ./getopt_00_simple -l fr
** ./getopt_00_simple -lf fr
**
** You can try an unknown option :
** ./getopt_00_simple --unknown-option
*/



int main(int argc, char* argv[])
{
	// The command line options parser
	GetOpt::Parser options;

	options.addParagraph("  This sample shows you how to add custom text paragraph where you want. "
		"In any cases, like for the options, the text will be formated to 80 columns.\n"
		"\n"
		"A long explanation :\n  Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer nec odio. Praesent "
		"libero. Sed cursus ante dapibus diam. Sed nisi. Nulla quis sem at nibh elementum imperdiet. "
		"Duis sagittis ipsum. Praesent mauris. Fusce nec tellus sed augue semper porta. Mauris massa. "
		"Vestibulum lacinia arcu eget nulla.\n");

	// Screen Settings
	options.addParagraph("Screen Settings:");
	// --width
	unsigned int optWidth = 800;
	options.add(optWidth, ' ', "width",  "Width of the screen");
	// --height
	unsigned int optHeight = 600;
	options.add(optHeight, ' ', "height", "Height of the screen");
	// --depth
	unsigned int optDepth = 32;
	options.add(optDepth, ' ', "depth", "Depth");
	// --fullscreen
	bool optFullscreen = false;
	options.addFlag(optFullscreen, 'f', "fullscreen", "Toggle the fullscreen mode");

	// Languages
	options.addParagraph("\nLanguages:");
	// --language
	String optLang = "en";
	options.add(optLang, 'l', "language", "Set the language to VALUE");
	// --language-list
	bool optLangList = false;
	options.addFlag(optLangList, ' ', "language-list", "Print the list of all languages and exit");

	// Help
	options.addParagraph("\nHelp");
	// Actually, when the option `help` is not overridden (means you did not add an
	// option with a long name `help`), the description of the --help option
	// is automatically added to the help usage.
	// If you want to make your own help usage, see the sample `04.customhelpusage`


	// Ask the parser to parse the command line
	if (!options(argc, argv))
	{
		// The program should not continue here
		return (options.errors() ? 1 /* Error */ : 0);
	}

	if (optLangList)
	{
		// As written in the help usage, display the list of languages
		// and exit
		std::cout << "Example: All languages : en, fr, ru, jp" << std::endl;
		return 0;
	}

	// Displaying the value of our variable on the standard output
	std::cout << "Width      : `" << optWidth << "`" << std::endl;
	std::cout << "Height     : `" << optHeight << "`" << std::endl;
	std::cout << "Depth      : `" << optDepth << "`" << std::endl;
	std::cout << "Fullscren  : `" << optFullscreen << "`" << std::endl;
	std::cout << "Language   : `" << optLang << "`" << std::endl;

	return 0;
}

