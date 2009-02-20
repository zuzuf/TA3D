#include "../stdafx.h"
#include "../logs/logs.h"

namespace TA3D
{
namespace
{

    int parserExpected(const String& tag, TDFParser& p, const String& key, const String& expectedValue)
    {
        String t = p.pullAsString(key);
        if (t != expectedValue)
        {
            LOG_ERROR("* TDF Parser error:");
            LOG_ERROR("      On " << tag << " for key `" << key << "`");
            LOG_ERROR("      Expected `" << expectedValue << "`, got `" << t << "`");
            return 1;
        }
        return 0;
    }

    int TDFParserSimpleTest()
    {
        int ret = 0;
        String data;
        data << "[root]\n"
            << "    { \n"
            << "       foo = bar;\n"
            << "      bar = foo ;\n"
            << "  semicolon = here is\\;two\\;semicolons;"
            << "  }";
        TDFParser p;
        p.loadFromMemory("Memory", data.c_str(), uint64(data.length()));
        ret += parserExpected("Simple Test", p, "root.foo", "bar");
        ret += parserExpected("Simple Test", p, "root.bar", "foo");
        ret += parserExpected("Simple Test", p, "root.semicolon", "here is;two;semicolons");
        return ret;
    }

    int TDFParserNestedTest()
    {
        int ret = 0;
        String data;
        data << "[root]\n"
            << "    { \n"
            << "       foo = bar;\n"
            << "      bar = foo ;\n"
            << "  [ piko   ] \n"
            << "       { \n"
            << "   sub foo = 42\n" // The semicolon is missing, this is a mistake but it should work
            << "   nyo = ta3d ; // here is a comment\n"
            << "      [gema] \n"
            << "            {\n"
            << "       sub-sub = lvl3\n"
            << "            }\n"
            << "           }\n"
            << "  }\n\n"
            << " // A comment outside a section !\n"
            << " [ s ] \n"
            // << "  {"  `{` missing but it will work too
            << "  url = http://www.ta3d.org ; // Our website ! \n"
            << " } \n";
        TDFParser p;
        p.loadFromMemory("Memory", data.c_str(), uint64(data.length()));
        ret += parserExpected("Nested", p, "root.foo", "bar");
        ret += parserExpected("Nested", p, "root.bar", "foo");
        ret += parserExpected("Nested", p, "root.piko.sub foo", "42");
        ret += parserExpected("Nested", p, "root.piko.nyo", "ta3d");
        ret += parserExpected("Nested", p, "root.piko.gema.sub-sub", "lvl3");
        ret += parserExpected("Nested", p, "s.url", "http://www.ta3d.org");
        return ret;
    }


    int TDFParserGadgetModeTest()
    {
        int ret = 0;
        String data;
        data << "[root]\n"
            << "    { \n"
            << "       foo = bar;\n"
            << "      bar = foo ;\n"
            << "  [ piko   ] \n"
            << "       { \n"
            << "   sub foo = 42\n" // The semicolon is missing, this is a mistake but it should work
            << "   nyo = ta3d ; // here is a comment\n"
            << "      [gema] \n"
            << "            {\n"
            << "       sub-sub = lvl3\n"
            << "            }\n"
            << "           }\n"
            << "  }\n\n"
            << " // A comment outside a section !\n"
            << " [ s ] \n"
            // << "  {"  `{` missing but it will work too
            << "  url = http://www.ta3d.org ; // Our website ! \n"
            << " } \n";
        TDFParser p;
        p.loadFromMemory("Memory", data.c_str(), uint64(data.length()), false, false, true);
        ret += parserExpected("Nested", p, "gadget0", "root");
        ret += parserExpected("Nested", p, "gadget1", "s");
        ret += parserExpected("Nested", p, "gadget0.foo", "bar");
        ret += parserExpected("Nested", p, "gadget0.bar", "foo");
        ret += parserExpected("Nested", p, "gadget0.piko.sub foo", "42");
        ret += parserExpected("Nested", p, "gadget0.piko.nyo", "ta3d");
        ret += parserExpected("Nested", p, "gadget0.piko.gema.sub-sub", "lvl3");
        ret += parserExpected("Nested", p, "gadget1.url", "http://www.ta3d.org");
        return ret;
    }

} // unnamed namespace
} // namespace TA3D

int main(void)
{
    int ret = 0;
    LOG_INFO("AutoTest: TDFParser: Starting...");
    ret += TDFParserSimpleTest(); 
    ret += TDFParserNestedTest();
    ret += TDFParserGadgetModeTest();
    LOG_INFO("AutoTest: TDFParser: Done (" << ret << " error(s))");
    return (ret ? 1 : 0);

}
END_OF_MAIN() /* Allegro stuff */
