#ifndef __YUNI_CORE_LOGS_DECORATORS_APPLICATION_NAME_H__
# define __YUNI_CORE_LOGS_DECORATORS_APPLICATION_NAME_H__

# include "../null.h"


namespace Yuni
{
namespace Logs
{


	template<class LeftType = NullDecorator>
	class ApplicationName : public LeftType
	{
	public:
		ApplicationName()
			:pAppName("noname")
		{}


		//! \name Apllication Name
		//@{
		const String& applicationName() const {return pAppName;}

		/*!
		** \brief Set the Application name
		**
		** \warning This method is not thread-safe and should only be used after the creation
		** of the logger
		*/
		template<class U> void applicationName(const U& s) {pAppName = s;}
		//@}


		template<class Handler, class VerbosityType, class O, class StringT>
		void internalDecoratorAddPrefix(O& out, const StringT& s) const
		{
			// Write the verbosity to the output
			out.put('[');
			out.write(pAppName.c_str(), pAppName.size());
			out.put(']');
			// Transmit the message to the next handler
			LeftType::template internalDecoratorAddPrefix<Handler, VerbosityType,O,StringT>(out, s);
		}

	private:
		//! The Application name
		String pAppName;

	}; // class ApplicationName





} // namespace Logs
} // namespace Yuni

#endif // __YUNI_CORE_LOGS_DECORATORS_APPLICATION_NAME_H__
