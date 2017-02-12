
#include "tcp.h"


namespace Yuni
{
namespace Net
{
namespace Message
{
namespace Transport
{


	TCP::TCP(Mode m)
		:ITransport(m)
	{}

	TCP::~TCP()
	{}


	Yuni::Net::Error  TCP::onExecute()
	{
		// \internal: The 'client' mode first
		switch (mode)
		{
			case tmClient:  return Yuni::Net::errUnknown;
			case tmServer:  return runServer();
			case tmNone:    break;
		}
		return Yuni::Net::errNone;
	}



	Yuni::Net::Error  TCP::runServer()
	{
		return Yuni::Net::errNone;
	}





} // namespace Transport
} // namespace Server
} // namespace Net
} // namespace Yuni


