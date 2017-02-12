
#include "option.h"




# define YUNI_GETOPT_HELPUSAGE_30CHAR  "                             "



namespace Yuni
{
namespace Private
{
namespace GetOptImpl
{


	namespace
	{

		template<bool Decal, int LengthLimit>
		void PrintLongDescription(std::ostream& out, const String& description)
		{
			String::Size start = 0;
			String::Size end = 0;
			String::Size p = 0;
			do
			{
				// Jump to the next separator
				p = description.find_first_of(" .\r\n\t", p);

				// No separator, aborting
				if (p == String::npos)
					break;

				if (p - start < LengthLimit)
				{
					switch (description.at(p))
					{
						case '\n':
							{
								out.write(description.c_str() + start, static_cast<std::streamsize>(p - start));
								out << '\n';
								if (Decal)
									out << YUNI_GETOPT_HELPUSAGE_30CHAR;
								start = p + 1;
								end = p + 1;
								break;
							}
						default:
							end = p;
					}
				}
				else
				{
					if (!end)
						end = p;
					out.write(description.c_str() + start, static_cast<std::streamsize>(end - start));
					out << '\n';
					if (Decal)
						out << YUNI_GETOPT_HELPUSAGE_30CHAR;
					start = end + 1;
					end = p + 1;
				}
				++p;
			}
			while (true);

			// Display the remaining piece of string
			if (start < description.size())
				out << (description.c_str() + start);
		}

	} // anonymous namespace






	void DisplayHelpForOption(std::ostream& out, const String::Char shortName, const String& longName,
		const String& description, bool requireParameter)
	{
		// Space
		if ('\0' != shortName && ' ' != shortName)
		{
			out << "  -" << shortName;
			if (longName.empty())
			{
				if (requireParameter)
					out << " VALUE";
			}
			else
				out << ", ";
		}
		else
			out << "      ";
		// Long name
		if (longName.empty())
		{
			if (requireParameter)
				out << "              ";
			else
				out << "                    ";
		}
		else
		{
			out << "--" << longName;
			if (requireParameter)
				out << "=VALUE";
			if (30 <= longName.size() + 6 /*-o,*/ + 2 /*--*/ + 1 /*space*/ + (requireParameter ? 6 : 0))
				out << "\n                             ";
			else
			{
				for (unsigned int i = 6 + 2 + 1 + (unsigned int) longName.size() + (requireParameter ? 6 : 0); i < 30; ++i)
					out.put(' ');
			}
		}
		// Description
		if (description.size() <= 50 /* 80 - 30 */)
			out << description;
		else
			PrintLongDescription<true, 50>(out, description);
		out << "\n";
	}


	void DisplayTextParagraph(std::ostream& out, const String& text)
	{
		if (text.size() <= 80)
			out << text;
		else
			PrintLongDescription<false, 80>(out, text);
		out << "\n";
	}




} // namespace GetOptImpl
} // namespace Private
} // namespace Yuni

