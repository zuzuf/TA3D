#ifndef __YUNI_CORE_SYSTEM_CONSOLE_CONSOLE_HXX__
# define __YUNI_CORE_SYSTEM_CONSOLE_CONSOLE_HXX__

# include "../windows.hdr.h"


namespace Yuni
{
namespace System
{
namespace Console
{


	# ifdef YUNI_OS_WINDOWS

	template<class U> inline void ResetTextColor(U&)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}

	template<> struct TextColor<black>
	{
		template<class U> static void Set(U&)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0);
		}
	};

	template<> struct TextColor<none>
	{
		template<class U> static void Set(U&) {}
	};

	template<> struct TextColor<white>
	{
		template<class U> static void Set(U&)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
	};

	template<> struct TextColor<red>
	{
		template<class U> static void Set(U&)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		}
	};

	template<> struct TextColor<green>
	{
		template<class U> static void Set(U&)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		}
	};

	template<> struct TextColor<yellow>
	{
		template<class U> static void Set(U&)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED);
		}
	};

	template<> struct TextColor<blue>
	{
		template<class U> static void Set(U&)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_BLUE);
		}
	};

	template<> struct TextColor<purple>
	{
		template<class U> static void Set(U&)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_RED);
		}
	};

	template<> struct TextColor<lightblue>
	{
		template<class U> static void Set(U&)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN);
		}
	};

	template<> struct TextColor<gray>
	{
		template<class U> static void Set(U&)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
	};


	# else


	template<> struct TextColor<none> { template<class U> static void Set(U&) {} };


	template<> struct TextColor<black>
	{ template<class U> static void Set(U& out) { out << "[1;30m"; } };

	template<> struct TextColor<red>
	{ template<class U> static void Set(U& out) { out << "[1;31m"; } };

	template<> struct TextColor<green>
	{ template<class U> static void Set(U& out) { out << "[1;32m"; } };

	template<> struct TextColor<blue>
	{ template<class U> static void Set(U& out) { out << "[1;34m"; } };

	template<> struct TextColor<yellow>
	{ template<class U> static void Set(U& out) { out << "[1;33m"; } };

	template<> struct TextColor<purple>
	{ template<class U> static void Set(U& out) { out << "[1;35m"; } };

	template<> struct TextColor<lightblue>
	{ template<class U> static void Set(U& out) { out << "[1;36m"; } };

	template<> struct TextColor<gray>
	{ template<class U> static void Set(U& out) { out << "[1;37m"; } };

	template<> struct TextColor<white>
	{ template<class U> static void Set(U& out) { out << "[1;37m[1m"; } };


	template<class U> inline void ResetTextColor(U& out)
	{
		out << "[0m";
	}

	# endif




	template<class U> inline void SetTextColor(U& out, const Color color)
	{
		switch (color)
		{
			case black:     TextColor<black>::Set(out); break;
			case red:       TextColor<red>::Set(out); break;
			case green:     TextColor<green>::Set(out); break;
			case yellow:    TextColor<yellow>::Set(out); break;
			case blue:      TextColor<blue>::Set(out); break;
			case purple:    TextColor<purple>::Set(out); break;
			case lightblue: TextColor<lightblue>::Set(out); break;
			case gray:      TextColor<gray>::Set(out); break;
			case white:     TextColor<white>::Set(out); break;
			case none:      break;
		}
	}




} // namespace Console
} // namespace System
} // namespace Yuni

#endif // __YUNI_CORE_SYSTEM_CONSOLE_CONSOLE_H__
