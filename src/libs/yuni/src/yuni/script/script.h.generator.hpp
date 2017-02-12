#ifndef __YUNI_SCRIPT_SCRIPT_H__
# define __YUNI_SCRIPT_SCRIPT_H__

<%
require File.dirname(__FILE__) + '/../../tools/generators/commons.rb'
generator = Generator.new()
%>
<%=generator.thisHeaderHasBeenGenerated("script.h.generator.hpp")%>


# include "../yuni.h"
# include "../core/string.h"
# include "../core/event.h"
# include "../core/any.h"
# include "../core/bind.h"
# include "../core/static/remove.h"

// Defines complex macros used to declare call() and bind().
# include "script.defines.h"


namespace Yuni
{
namespace Script
{

	// Adding a language ?
	// Insert here the declaration of your language's class.
	class Lua;

} // namespace Script
} // namespace Yuni

// Include Bindings in private namespace
# include "private.h"



namespace Yuni
{
namespace Script
{

	/*!
	** \brief All the supported languages.
	**
	** \todo This enum must be dynamic to account only for the
	** \todo built-in languages.
	** \ingroup Script
	*/
	enum Language
	{
		//! This language is unknown
		slUnknown = 0,
		//! This is the Lua language
		slLua
	};



	/*!
	** \brief Script Interface (abstract)
	** \ingroup Script
	*/
	class AScript : public Policy::ObjectLevelLockable<AScript>
	{
	public:
		//! The Threading policy
		typedef Policy::ObjectLevelLockable<AScript> ThreadingPolicy;

	public:
		//! \name Contructor & Destructor
		//@{
		//! Default constructor
		AScript() {}
		//! Destructor
		virtual ~AScript();
		//@}

		//! \name Language
		//@{
		//! Returns the script language
		virtual Language language() const = 0;
		//@}


		//! \name Script load & save operations
		//@{
		/*!
		** \brief Loads the specified file a fresh script context.
		**
		** This method is equivalent to calling reset() just before appendFromFile().
		**
		** \param[in] file The file to load
		** \return True if the script was at least parsed correcty.
		*/
		virtual bool loadFromFile(const String& file);

		/*!
		** \brief Loads the specified file in the current context.
		**
		** The specified file will be parsed and may or may not be evaluated
		** immediately, depending on the capacities of the underlying script
		** engine. You may call this method several times to sequentially load several
		** files in the same script context. The files will be parsed (and evaluated
		** on subsequent calls to call() or prepare()) in the same order.
		** sequentially in this case (FIFO).
		**
		** \param[in] file The file to load
		** \return True if the script was at least parsed correcty.
		** \see run(), call()
		*/
		virtual bool appendFromFile(const String& file) = 0;

		/*!
		** \brief Loads the specified string in a fresh script context
		**
		** Behaves like loadFromFile().
		**
		** \param[in] script The string to load
		** \return True if the script was at least parsed correcty.
		** \see loadFromFile()
		*/
		virtual bool loadFromString(const String& script);

		/*!
		** \brief Loads the specified string in the current context
		**
		** Behaves like appendFromFile().
		**
		** \param[in] script The string to load
		** \return True if the script was at least parsed correcty.
		** \see appendFromFile()
		*/
		virtual bool appendFromString(const String& script) = 0;

		/*!
		** \brief Loads the specified buffer a fresh context
		**
		** Behaves like loadFromFile().
		**
		** \param[in] scriptBuf The buffer to load
		** \param[in] scriptSize The size of the data in scriptBuf.
		** \return True if the script was at least parsed correcty.
		** \see loadFromFile()
		*/
		virtual bool loadFromBuffer(const char *scriptBuf, const unsigned int scriptSize);

		/*!
		** \brief Loads the specified buffer in the current context
		**
		** Behaves like appendFromFile().
		**
		** \param[in] scriptBuf The buffer to load
		** \param[in] scriptSize The size of the data in scriptBuf.
		** \return True if the script was at least parsed correcty.
		** \see appendFromFile()
		*/
		virtual bool appendFromBuffer(const char* scriptBuf, const unsigned int scriptSize) = 0;

		/*!
		** \brief Restarts the script engine
		**
		** Almost equivalent to destroying and re-creating the script object. The
		** script engine is initialized again.
		*/
		virtual void reset() = 0;
		//@}


		//! \name Execution control
		//@{
		/*!
		** \brief Parses and run any pending script or script chunk loaded with
		** loadFromString() or loadFromFile().
		**
		** Once the scripts are run, they are removed from the queue, and subsequent
		** calls to run() won't do anything. If you want to repeat a particular action,
		** consider creating a script function doing what you want, and call() it.
		**
		** In case of error while parsing scripts, the parsing stops at the first problem
		** and this function returns false.
		**
		** \return True if there was no error returned by the script engine, false otherwise.
		*/
		virtual bool prepare() = 0;

		/*!
		** \brief Call the default entry point of the script
		**
		** This method is a convenient shortcut for the "main" function in
		** the current script. It is equivalent to: call("main");
		**
		** \return True if the "main" function was successfully called.
		*/
		bool run();

		/*!
		** \brief Returns true if the named function is bound.
		**
		** \param[in] functionName The function name
		*/
		template<typename T> bool isBound(const T& functionName) const;

		/*!
		** \brief Clears the bindings associated with the script.
		**
		** Call this method to unbind any function that may be bound
		** with the AScript underlying object.
		*/
		void clear();

		/*!
		** \brief This family of functions calls the specified function.
		**
		** It exists in 9 different versions, from 0 to 8 arguments.
		**
		** This function supports a restricted set of value types. If a type mean nothing in the
		** current script language, a default neutral value will be used.
		**
		** \code
		**
		** Any ret;
		** Script::AScript *sc = ...;
		** std::vector<int> intVector;
		**
		** [...]
		**
		** if (!sc->call("luaMethod", 42, 55.42, intVector, "Some string"))
		** {
		**	 // The call has failed
		** }
		** else
		** {
		**	 // Do something with ret.
		** }
		**
		** \endcode
		**
		** \param[out] retValues A variant containing the (or a list of) the return
		**			   values. You may pass NULL there if you do
		**			   not wish to do anything with the return value.
		** \param[in] method The method to call in the script namespace
		** \param[in] arg1 The first argument (and so on.) You can pass an empty Any as a
		**			  value to mean the same thing as "nil" in Lua.
		** \return True if the call was made without any runtime fatal error.	
		*/
		YUNI_SCRIPT_SCRIPT_DECLARE_CALL_WITH() = 0;
		YUNI_SCRIPT_SCRIPT_DECLARE_CALL_WITH(YUNI_SCRIPT_SCRIPT_1_ANY) = 0;
		YUNI_SCRIPT_SCRIPT_DECLARE_CALL_WITH(YUNI_SCRIPT_SCRIPT_2_ANYS) = 0;
		YUNI_SCRIPT_SCRIPT_DECLARE_CALL_WITH(YUNI_SCRIPT_SCRIPT_3_ANYS) = 0;
		YUNI_SCRIPT_SCRIPT_DECLARE_CALL_WITH(YUNI_SCRIPT_SCRIPT_4_ANYS) = 0;
		YUNI_SCRIPT_SCRIPT_DECLARE_CALL_WITH(YUNI_SCRIPT_SCRIPT_5_ANYS) = 0;
		YUNI_SCRIPT_SCRIPT_DECLARE_CALL_WITH(YUNI_SCRIPT_SCRIPT_6_ANYS) = 0;
		YUNI_SCRIPT_SCRIPT_DECLARE_CALL_WITH(YUNI_SCRIPT_SCRIPT_7_ANYS) = 0;
		YUNI_SCRIPT_SCRIPT_DECLARE_CALL_WITH(YUNI_SCRIPT_SCRIPT_8_ANYS) = 0;


		/*!
		** \brief Binds a C++ function in the script context.
		**
		** This family of functions bind a C++ function in the script context,
		** with the specified function name.
		**
		** Lua code:
		** \code
		** -- Print hello world !
		** myPrint_function("hello, world !");
		** \endcode
		**
		** C++ code:
		** \code
		**
		** bool myPrint(String toPrint)
		** {
		**	 std::cout << toPrint << std::endl;
		**	 return true;
		** }
		**
		** Script::AScript *sc = ...;
		**
		** if (!sc->bind("myPrint_function", &(::myPrint)))
		** {
		**		// The function could not be bound
		** }
		** else
		** {
		**		// The function was bound !
		**		// You can call it from the script.
		** }
		**
		** \endcode
		**
		** \param[in] method The function name in the script namespace
		** \param[in] callback A pointer to the C++ function to bind in the script.
		** \return True if the function was successfully bound.
		*/
		template <class FunctionT, typename U>
		bool bind(const U& functionName, FunctionT funcPtr);


		template <class ClassT, class MemberT, typename U>
		bool bind(const U& functionName, ClassT* object, MemberT member);


		/*!
		** \brief Unbinds a function by name
		** \param[in] functionName The function to unbind
		*/
		template<typename T> bool unbind(const T& functionName);
		//@}


	public:
		//! Type for script run-time error events
		typedef Event<void(Language, const String& /* file */, unsigned int /* line */,
				unsigned int /* position */, const String& /*errorString */)> ScriptErrorEvent;

		/*!
		** \brief Event for script errors
		**
		** This event will be emitted on every script error reported
		** by the underlying script engine.
		*/
		ScriptErrorEvent onError;

	protected:
		/*!
		** \brief Language-specific function binder
		** \param[in] name Function name inside the script.
		** \param[in] toBind Pointer to the generic function wrapper to bind.
		**
		** This function is implemented by language specific inheriting classes
		** to do the specific work required to really bind the function.
		** This involves, for example, lua_pushcclosure for Lua.
		*/
		virtual bool internalBindWL(const char* name, Private::ScriptImpl::Bind::IBinding* toBind) = 0;

		/*!
		** \brief Language-specific function unbinder
		** \param[in] toUnbind Name of the function to unbind.
		** \see internalBindWL
		**
		** Reverse operation of internalBindWL.
		*/
		virtual void internalUnbindWL(const char* toUnbind) = 0;

	private:
		/*!
		** \brief Delete bound functions without unbinding them
		**
		** This operation is useful when the script context is going
		** to be destroyed and we want to delete the function objects
		** quickly.
		** The script must not be run again before the context is
		** destroyed.
		*/
		void clearWithoutUnbindWL();

	private:
		//! Type for dictionnary of currently bound functions
		typedef std::map<String, Private::ScriptImpl::Bind::IBinding*> BoundFunctions;

		//! Dictionnary of currently bound functions
		BoundFunctions pBoundFunctions;

	}; // class AScript





} // namespace Script
} // namespace Yuni


// Cleans up complex macros used to declare call() and bind().
# include "script.undefs.h"

// Template methods definitions
# include "script.hxx"
# include "args.hxx"

#endif // __YUNI_SCRIPT_SCRIPT_H__
