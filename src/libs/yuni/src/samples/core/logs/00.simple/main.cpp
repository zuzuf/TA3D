
#include <yuni/yuni.h>
#include <yuni/core/logs.h>


int main(void)
{
	// A simple logger, which only display on std::cout/cerr
	// Please note that std::cerr can not be used on Windows (the messages would not be displayed)
	Yuni::Logs::Logger<>  logs;


	// Hello, world !, all standard verbosity levels
	logs.checkpoint() << "Hello, " << "world !";
	logs.notice() << "Hello, " << "world !";
	logs.warning() << "Hello, " << "world !";
	logs.error() << "Hello, " << "world !";
	logs.debug() << "Hello, " << "world !";
	logs.fatal() << "Hello, " << "world !";

	return 0;
}
