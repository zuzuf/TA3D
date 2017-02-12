#ifndef __YUNI_NET_MESSAGE_TRANSPORT_LAYER_H__
# define __YUNI_NET_MESSAGE_TRANSPORT_LAYER_H__



namespace Yuni
{
namespace Net
{


	enum TransportLayer
	{
		tlNone = 0,
		tlTCP,
		tlUDP,
		tlCustom,
	};



} // namespace Net
} // namespace Yuni

#endif // __YUNI_NET_MESSAGE_TRANSPORT_LAYER_H__
