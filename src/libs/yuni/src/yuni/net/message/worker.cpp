
#include "worker.h"
#include "queueservice.h"


namespace Yuni
{
namespace Private
{
namespace Net
{
namespace Message
{


	bool Worker::onExecute()
	{
		if (!pTransport)
			return true;
		// The current transport layer
		ITransport& transport = *pTransport;

		// Attach the current thread to the transport layer
		transport.attachedThread(this);
		// Start the transport layer
		const Yuni::Net::Error error = transport();
		// Detach the thread
		transport.attachedThread(nullptr);

		if (error != Yuni::Net::errNone)
			pQueueService.onError(QueueService::stRunning, error);
		return true;
	}





} // namespace Message
} // namespace Net
} // namespace Private
} // namespace Yuni

