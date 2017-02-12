#ifndef __YUNI_CORE_GETOPT_PARSER_H__
# define __YUNI_CORE_GETOPT_PARSER_H__

# include "../../yuni.h"
# include <map>
# include "option.h"
# include <string.h>
# include "../validator/text/default.h"



namespace Yuni
{
namespace GetOpt
{


	/*!
	** \brief A command line options parser
	*/
	class Parser
	{
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		Parser();
		/*!
		** \brief Destructor
		*/
		~Parser();
		//@}

		/*!
		** \brief Remove all options
		*/
		void clear();


		//! \name Adding an option
		//@{
		/*!
		** \brief Add an option
		**
		** \param[in] var The variable where the value(s) will be written
		** \param shortName The short name of the option (a single char)
		** \param visible True if the option is visible from the help usage
		*/
		template<class U>
		void add(U& var, const char shortName, bool visible = true);


		/*!
		** \brief Add an option
		**
		** \param[in] var The variable where the value(s) will be written
		** \param shortName The short name of the option (a single char)
		** \param longName The long name of the option
		** \param visible True if the option is visible from the help usage
		*/
		template<class U, class S>
		void add(U& var, const char shortName, const S& longName, bool visible = true);


		/*!
		** \brief Add an option
		**
		** \param[in] var The variable where the value(s) will be written
		** \param shortName The short name of the option (a single char)
		** \param longName The long name of the option
		** \param description The description of the option (used in the help usage)
		** \param visible True if the option is visible from the help usage
		*/
		template<class U, class S, class D>
		void add(U& var, const char shortName, const S& longName, const D& description, bool visible = true);


		/*!
		** \brief Add an option that does not require an additional parameter
		**
		** \param[in] var The variable where the value(s) will be written
		** \param shortName The short name of the option (a single char)
		** \param visible True if the option is visible from the help usage
		*/
		template<class U>
		void addFlag(U& var, const char shortName, bool visible = true);


		/*!
		** \brief Add an option that does not require an additional parameter
		**
		** \param[in] var The variable where the value(s) will be written
		** \param shortName The short name of the option (a single char)
		** \param longName The long name of the option
		** \param visible True if the option is visible from the help usage
		*/
		template<class U, class S>
		void addFlag(U& var, const char shortName, const S& longName, bool visible = true);


		/*!
		** \brief Add an option that does not require an additional parameter
		**
		** \param[in] var The variable where the value(s) will be written
		** \param shortName The short name of the option (a single char)
		** \param longName The long name of the option
		** \param description The description of the option (used in the help usage)
		** \param visible True if the option is visible from the help usage
		*/
		template<class U, class S, class D>
		void addFlag(U& var, const char shortName, const S& longName, const D& description, bool visible = true);
		//@}

		//! \name Remaining arguments
		//@{
		/*!
		** \brief Set the target variable where remaining arguments will be writtent
		*/
		template<class U> void remainingArguments(U& var);
		//@}


		//! \name Command line parsing
		//@{
		/*!
		** \brief Parse the command line
		**
		** \param argc The count of arguments
		** \param argv The list of arguments
		** \return False if the program should abort
		*/
		bool operator () (int argc, char* argv[]);
		//@}


		//! \name Help usage
		//@{
		/*!
		** \brief Add a text paragraph after the last added option
		**
		** \param text Any text of an arbitrary length
		*/
		template<class U> void addParagraph(const U& text);

		/*!
		** \brief Generate and display an help usage
		**
		** \note If you want your own help usage, add the option ('h', "help")
		** to handle yourself the behavior.
		*/
		void helpUsage(const char* argv0);
		//@}


		//! \name Errors
		//@{
		/*!
		** \brief The count of errors that have been encountered
		*/
		unsigned int errors() const {return pErrors;}
		//@}


	private:
		/*!
		** \brief Predicate to compare two CString
		*/
		struct CStringComparison
		{
			bool operator()(const char* s1, const char* s2) const {return ::strcmp(s1, s2) < 0;}
		};

		//! IOption
		typedef Private::GetOptImpl::IOption IOption;
		//! Option list (order given by the user)
		typedef std::vector<IOption*> OptionList;
		//! All options ordered by their short name
		typedef std::map<char, IOption*> OptionsOrderedByShortName;
		//! All options ordered by their long name
		typedef std::map<const char*, IOption*, CStringComparison> OptionsOrderedByLongName;

	private:
		//! All existing options
		OptionList pAllOptions;
		//! All options ordered by their short name
		OptionsOrderedByShortName pShortNames;
		//! All options ordered by their long name
		OptionsOrderedByLongName pLongNames;

		//! Options for remaining arguments
		IOption* pRemains;

		//! Count of error
		unsigned int pErrors;

		// A friend
		friend class Private::GetOptImpl::Context;

	}; // class Parser





} // namespace GetOpt
} // namespace Yuni

# include "parser.hxx"

#endif // __YUNI_CORE_GETOPT_PARSER_H__
