#ifndef __YUNI_NET_MESSAGE_QUEUESERVICE_HXX__
# define __YUNI_NET_MESSAGE_QUEUESERVICE_HXX__


namespace Yuni
{
namespace Net
{
namespace Message
{

	template<class StringT>
	inline Error QueueService::sendAll(const StringT& buffer)
	{
		const unsigned int len = Traits::Length<StringT, unsigned int>::Value(buffer);
		const char* const cstr = Traits::CString<StringT>::Perform(buffer);
		return sendAll(cstr, len);
	}





} // namespace Message
} // namespace Net
} // namespace Yuni

#endif // __YUNI_NET_MESSAGE_QUEUESERVICE_HXX__
