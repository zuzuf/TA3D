#ifndef __YUNI_CORE_LOGS_DECORATORS_MESSAGE_H__
# define __YUNI_CORE_LOGS_DECORATORS_MESSAGE_H__

# include "../null.h"
# include "../../system/console.h"


namespace Yuni
{
namespace Logs
{


	template<class LeftType = NullDecorator>
	class Message : public LeftType
	{
	public:
		template<class Handler, class VerbosityType, class O, class StringT>
		void internalDecoratorAddPrefix(O& out, const StringT& s) const
		{
			// Write the message itself
			out.put(' ');

			// Color
			if (VerbosityType::messageColor != System::Console::none && Handler::unixColorsAllowed)
				System::Console::TextColor<VerbosityType::messageColor>::Set(out);

			// The message
			out.write(s.c_str(), (size_t)s.sizeInBytes());

			// Resetting the color
			if (VerbosityType::messageColor != System::Console::none && Handler::unixColorsAllowed)
				System::Console::ResetTextColor(out);

			// Transmit the message to the next handler
			LeftType::template internalDecoratorAddPrefix<Handler, VerbosityType,O,StringT>(out, s);
		}

	}; // class VerbosityLevel





} // namespace Logs
} // namespace Yuni

#endif // __YUNI_CORE_LOGS_DECORATORS_MESSAGE_H__
