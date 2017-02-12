
#include <iostream>
#include <yuni/yuni.h>
#include <yuni/core/string/string.h>


namespace Yuni
{

	template<typename T>
	int stringCompare(const String& t1, const T t2, bool equal)
	{
		if ((t1 == t2) != equal)
		{
			std::cerr << "* String comparison failed !" << std::endl
				<< "	 `" << t1 << "` should be "
				<< (equal ? "equal to `" : "different from `")
				<< String(t2) << "`" << std::endl;
			return 1;
		}
		return 0;
	}

	template<typename T>
	int stringConvert(const String& section, const T t, const T expected)
	{
		if (t != expected)
		{
			std::cerr << "* String conversion failed !" << std::endl
				<< "	 From `String` to `" << section << "`" << std::endl
				<< "	 Expected: `" << expected << "`, got `"
				<< t << "`" << std::endl;
			return 1;
		}
		return 0;
	}

	template<typename T>
	int stringCheck(const String& method, const T found, const T expected)
	{
		if (found != expected)
		{
			std::cerr << "* String conversion failed !" << std::endl
				<< "	 While checking String::" << method << std::endl
				<< "	 Expected: `" << expected << "`, got `"
				<< found << "`" << std::endl;
			return 1;
		}
		return 0;
	}

	int stringCheckKV(const String& t, const String& expectedKey = "", const String& expectedValue = "")
	{
		String k;
		String v;
		String::ExtractKeyValue(t, k, v);
		if (k != expectedKey || v != expectedValue)
		{
			std::cerr << "* String Check failed !" << std::endl
				<< "	 On `" << t << "`" << std::endl
				<< "	 Expected: key=`" << expectedKey << "`,  value=`" << expectedValue << "`" << std::endl
				<< "	 Found: key=`" << k << "`,  value=`" << v << "`" << std::endl;
			return 1;
		}
		return 0;
	}


	int autoTest()
	{
		std::cout << "* SelfTest: Strings..." << std::endl;
		int ret = 0;
		ret += stringCompare(String(""), "", true);
		ret += stringCompare(String(""), "\0\0\0\0", true);
		ret += stringCompare(String(""), (const char*)0, false);
		ret += stringCompare(String(""), "abc", false);
		ret += stringCompare(String("abc"), "", false);
		const char* test = "test";
		ret += stringCompare(String(test), test, true);
		ret += stringCompare(String(test), "test", true);
		ret += stringCompare(String(test), "test ", false);
		ret += stringCompare(String(test), "testtest", false);
		ret += stringCompare(String("abc"), "abc", true);
		ret += stringCompare(String("abc"), String("abc"), true);
		ret += stringCompare(String("abc"), "abc_extended", false);
		ret += stringCompare(String("abc"), String("abc_extended"), false);
		ret += stringConvert("int32", String(10).to<sint32>(), 10);
		ret += stringConvert("uint32", String(uint32(-1)).to<uint32>(), uint32(-1));
		ret += stringConvert("int64", String(sint64(33554432 * 30)).to<sint64>(), sint64(33554432 * 30));
		ret += stringConvert("hexa", String("0xFF").to<sint32>(), (sint32)255);
		ret += stringConvert("int16", String(12).to<sint16>(), sint16(12));
		ret += stringConvert("uint16", String("65536").to<uint16>(), uint16(0));
		ret += stringCheck("Append", String("") + "yop\t", String("yop\t"));
		ret += stringCheck("Append", String("abcdefg") + "", String("abcdefg"));
		ret += stringCheck("Append", String("abcd", 2) + "sence", String("absence"));
		//ret += stringCheck("Append", StringBase<char, 1>(StringBase<char, 1>("voit:") + ":ure"), StringBase<char, 1>("voit::ure")); // Does not compile. Shouldn't it?
		ret += stringCheckKV("a=b;", "a", "b");
		ret += stringCheckKV(" a=b;", "a", "b");
		ret += stringCheckKV("a= b;", "a", "b");
		ret += stringCheckKV("a=b; ", "a", "b");
		ret += stringCheckKV(" a = b ;", "a", "b");
		ret += stringCheckKV(" !(*^&$% = b ;", "!(*^&$%", "b");
		ret += stringCheckKV(" ab c = 1 2 3 4 5 ;", "ab c", "1 2 3 4 5");
		ret += stringCheckKV(" foo=bar; // comments here; ", "foo", "bar");
		ret += stringCheckKV(" foo=bar comments//here ", "foo", "bar comments");
		ret += stringCheckKV(" foo=bar comments // here ", "foo", "bar comments");
		ret += stringCheckKV(" foo=bar  /   this is not a comment	", "foo", "bar  /   this is not a comment");
		ret += stringCheckKV("foo=http://www.example.org;// comment", "foo", "http://www.example.org");
		ret += stringCheckKV("[ini] ", "[", "ini");
		ret += stringCheckKV("  [Example of Section] ", "[", "Example of Section");
		ret += stringCheckKV("  [ foo section  ] ", "[", "foo section");
		ret += stringCheckKV("  [ bad section ", "[", "bad section");
		ret += stringCheckKV("  [ bad section // comments", "[", "bad section // comments");
		ret += stringCheckKV("[ bad section // comments  ", "[", "bad section // comments");
		ret += stringCheckKV("  [Example of Section] // Comments", "[", "Example of Section");
		ret += stringCheckKV("  [Example of Section]//Comments", "[", "Example of Section");
		ret += stringCheckKV("  // Here is a comment ");
		ret += stringCheckKV("  nyo // Here is a comment ", "nyo");
		ret += stringCheckKV("  // a = b; ");
		ret += stringCheckKV("  / a = b; ", "/ a", "b");
		ret += stringCheckKV("  piko//li = b; ", "piko", "");
		ret += stringCheckKV("  piko/li = b; ", "piko/li", "b");
		ret += stringCheckKV("  piko   //li = b; ", "piko", "");
		ret += stringCheckKV("{ ", "{");
		ret += stringCheckKV("  }", "}");
		ret += stringCheckKV("   { // Start of a new block", "{");
		ret += stringCheckKV(" } // end of block", "}");
		ret += stringCheckKV(" a = a full text with \\n cariage return", "a", "a full text with \n cariage return");
		ret += stringCheckKV(" a = semicolon\\;test", "a", "semicolon;test");
		ret += stringCheckKV(" a = semicolon\\; test", "a", "semicolon; test");
		ret += stringCheckKV(" a = semicolon \\;test", "a", "semicolon ;test");
		ret += stringCheckKV(" a = semicolon \\;  test", "a", "semicolon ;  test");
		if (ret > 0)
			std::cerr << ret << " tests failed." << std::endl;
		return ret;
	}

} // unnamed namespace



int main(void)
{
	return Yuni::autoTest() ? 1 : 0;
}

