#ifndef __YUNI_CORE_DECORATORS_LOGS_TIME_H__
# define __YUNI_CORE_DECORATORS_LOGS_TIME_H__

# include "../null.h"


namespace Yuni
{
namespace Private
{
namespace LogsDecorator
{

	// Forward declarations
	# if defined(YUNI_OS_MINGW)
	char* WriteCurrentTimestampToBufferMinGW(void);
	# else
	void WriteCurrentTimestampToBuffer(char buffer[32]);
	# endif

} // namespace LogsDecorator
} // namespace Private
} // namespace Yuni


namespace Yuni
{
namespace Logs
{


	template<class LeftType = NullDecorator>
	class Time : public LeftType
	{
	public:
		template<class Handler, class VerbosityType, class O, class StringT>
		void internalDecoratorAddPrefix(O& out, const StringT& s) const
		{
			out.put('[');

			# ifndef YUNI_OS_MINGW
			char asc[32]; // MSDN specifies that the buffer length value must be >= 26 for validity
			Private::LogsDecorator::WriteCurrentTimestampToBuffer(asc);
			out.write(asc, 24);
			# else
			out.write(Private::LogsDecorator::WriteCurrentTimestampToBufferMinGW(), 24);
			# endif

			out.put(']');

			// Transmit the message to the next decorator
			LeftType::template internalDecoratorAddPrefix<Handler, VerbosityType,O,StringT>(out, s);
		}

	}; // class Time




} // namespace Logs
} // namespace Yuni

#endif // __YUNI_CORE_DECORATORS_LOGS_TIME_H__
