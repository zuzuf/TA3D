#ifndef __YUNI_NET_MESSAGE_WORKER_HXX__
# define __YUNI_NET_MESSAGE_WORKER_HXX__


namespace Yuni
{
namespace Private
{
namespace Net
{
namespace Message
{

	inline Worker::Worker(QueueService& queueservice, ITransport::Ptr transport) :
		pTransport(transport),
		pQueueService(queueservice)
	{}


	inline Worker::~Worker()
	{}





} // namespace Message
} // namespace Net
} // namespace Private
} // namespace Yuni

#endif // __YUNI_NET_MESSAGE_QUEUESERVICE_WORKER_HXX__
