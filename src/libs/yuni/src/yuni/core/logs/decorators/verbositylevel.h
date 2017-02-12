#ifndef __YUNI_CORE_LOGS_DECORATORS_VERBOSITY_LEVEL_H__
# define __YUNI_CORE_LOGS_DECORATORS_VERBOSITY_LEVEL_H__

# include "../null.h"
# include "../../system/console.h"


namespace Yuni
{
namespace Logs
{


	template<class LeftType = NullDecorator>
	class VerbosityLevel : public LeftType
	{
	public:
		template<class Handler, class VerbosityType, class O, class StringT>
		void internalDecoratorAddPrefix(O& out, const StringT& s) const
		{
			// Write the verbosity to the output
			if (VerbosityType::hasName)
			{
				// Unix Color
				if (Handler::unixColorsAllowed && VerbosityType::color != System::Console::none)
					System::Console::TextColor<VerbosityType::color>::Set(out);
				// The verbosity
				VerbosityType::AppendName(out);
				// Unix Color
				if (Handler::unixColorsAllowed && VerbosityType::color != System::Console::none)
					System::Console::ResetTextColor(out);
			}
			// Transmit the message to the next decorator
			LeftType::template internalDecoratorAddPrefix<Handler, VerbosityType,O,StringT>(out, s);
		}

	}; // class VerbosityLevel





} // namespace Logs
} // namespace Yuni

#endif // __YUNI_CORE_LOGS_DECORATORS_VERBOSITY_LEVEL_H__
