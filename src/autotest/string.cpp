
#include "../stdafx.h"
#include "../misc/string.h"
#include "../logs/logs.h"


namespace TA3D
{

    template<typename T>
    int stringConvert(const String& section, const T t, const T expected)
    {
        if (t != expected)
        {
            LOG_ERROR("* String convertion failed !");
            LOG_ERROR("     From `String` to `" << section << "`");
            LOG_ERROR("     Expected: `" << expected << "`, got `" << t << "`");
            return 1;
        }
        return 0;
    }

    static int stringCheckKV(const String& t, const String& expectedKey = "", const String& expectedValue = "")
    {
        String k;
        String v;
        TA3D::String::ToKeyValue(t, k, v);
        if (k != expectedKey || v != expectedValue)
        {
            LOG_ERROR("* String Check failed !");
            LOG_ERROR("     On `" << t << "`");
            LOG_ERROR("     Expectd: key=`" << expectedKey << "`,  value=`" << expectedValue << "`");
            LOG_ERROR("     Found: key=`" << k << "`,  value=`" << v << "`");
            return 1;
        }
        return 0;
    }


    int autoTest()
    {
        int ret = 0;
        ret += stringConvert("int32", String(10).toInt32(), 10);
        ret += stringConvert("uint32", String(uint32(-1)).toUInt32(), uint32(-1));
        ret += stringConvert("int64", String(sint64(33554432 * 30)).toInt64(), sint64(33554432 * 30));
        ret += stringConvert("hexa", String("0xFF").toInt32(), (sint32)255);
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
        ret += stringCheckKV(" foo=bar  /   this is not a comment    ", "foo", "bar  /   this is not a comment");
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
        return ret;
    }

} // namespace TA3D





int main(void)
{
    LOG_INFO("* SelfTest: Strings...");
    return (TA3D::autoTest() ? 1 : 0);
}
END_OF_MAIN() /* Allegro stuff */


