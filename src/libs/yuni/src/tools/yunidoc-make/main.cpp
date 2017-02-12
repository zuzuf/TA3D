
#include <yuni/yuni.h>
#include "logs.h"
#include "make.h"
#include "job.h"


using namespace Yuni;



int main(int argc, char** argv)
{
	logs.applicationName("yunidoc-make");
	Make make;
	make.parseCommandLine(argc, argv);
	queueService.start();
	make.findAllSourceFiles();
	queueService.wait(60 * 60);
	queueService.stop();
	return 0;
}

