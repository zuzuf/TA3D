#ifndef __YUNI_DOCMAKE_MAKE_H__
# define __YUNI_DOCMAKE_MAKE_H__

# include <yuni/yuni.h>
# include <yuni/core/string.h>
# include "sqlite/sqlite3.h"


class Make 
{
public:
	/*!
	** \brief Default constructor
	*/
	Make();

	void parseCommandLine(int argc, char** argv);

	void findAllSourceFiles();

public:
	Yuni::String input;
	Yuni::String htdocs;
	unsigned int nbJobs;
	bool printVersion;
	bool debug;
	bool verbose;
	bool clean;

}; // class Make



#endif // __YUNI_DOCMAKE_MAKE_H__
