
#include <iostream>
#include <yuni/core/uri.h>



static void check(const Yuni::String& s)
{
	Yuni::Uri  uri(s);
	std::cout << "  Test : " << s << std::endl;
	std::cout << "   Got : " << uri << std::endl;
	std::cout << " Valid : " << (uri.isValid() ? "Yes\n" : "No\n");

	if (uri.isValid())
	{
		// Display some informations if the URI is valid

		if (!uri.scheme().empty())
			std::cout << "        Scheme : " << uri.scheme() << std::endl;
		if (!uri.server().empty())
		{
			std::cout << "        Server : " << uri.server();
			if (uri.port() > 0)
				std::cout << " : " << uri.port();
			std::cout << std::endl;
		}
		if (!uri.user().empty())
		{
			std::cout << "          User : " << uri.user();
			if (!uri.password().empty())
				std::cout << ", " << uri.password();
			std::cout << std::endl;
		}
		if (!uri.path().empty())
			std::cout << "          Path : " << uri.path() << std::endl;
		if (!uri.query().empty())
			std::cout << "         Query : " << uri.query() << std::endl;
		if (!uri.fragment().empty())
			std::cout << "      Fragment : " << uri.fragment() << std::endl;
	}

	// Final separator
	std::cout << " -- " << std::endl;
}




static void uriBuildTests()
{
	std::cout << "=== URI Build ===\n" << std::endl;
	check("foo://example.com:8042/over/there?name=ferret#nose");
	check("foo://example.com:8042/over/there?name=ferret");
	check("foo://example.com:8042/over/there#nose");
	check("foo://example.com:8042#nose");
	check("foo://example.com:8042?queryfornopath=1");
	check("foo://example.com:8042?queryfornopath=1#withfragment");
	check("foo://example?queryfornopath=1");
	check("foo://example#fragment");
	check("foo://example.com/over/there?name=ferret#nose");
	check("foo://example.com/");
	check("foo://example.com");
	check("foo://example.com:4242");
	check("urn:example:animal:ferret:nose");
	check("telnet://192.0.2.16:80/");
	check("mailto:John.Doe@example.com");
	check("foo://info.example.com?fred");
	check("foo://info.example.com#fred");
	check("news:comp.infosystems.www.servers.unix");
	check("ftp://ftp.is.co.za/rfc/rfc1808.txt");
	check("http://www.ietf.org/rfc/rfc2396.txt");
	check("tel:+1-816-555-1212");
	check("urn:oasis:names:specification:docbook:dtd:xml:4.1.2");
	check("ldap://[2001:db8::7]/c=GB?objectClass?one");
	check("ldap://[2001:db8::7]:999/c=GB?objectClass?one");
	check("http://user@[2001:db8::7]:999/c=GB?objectClass?one");
	check("http://user:somepass@[2001:db8::7]:999/c=GB?objectClass?one");
	check("ftp://user@ftp.is.co.za/rfc/rfc1808.txt");
	check("ftp://user:dummypass@ftp.is.co.za/rfc/rfc1808.txt");
	check("foo://foouser@example.com:8042/over/there?name=ferret#nose");
	check("foo://baruser:suprapassword@example.com:8042/over/there?name=ferret#nose");
	check("file://relativeisserveractually/path/to/somewhere");
	check("file://./relative/path/to/somewhere");
	check("file:///path/to/somewhere");
	check("proto://server/path/to/somewhere?querywithslashes=/path/to/somewhere/else");
	check("proto://server/path/to/somewhere?querywithslashes=/path/to/somewhere/else?subquery");
	check("proto://server/path/to/somewhere?querywithslashes=/path/to/somewhere/else?subquery#real_fragment");
	check("/path/to/somewhere");
	check("/path/to/somewhere?withastrangequery=foo");
	check("./relative/path/");
	check("missingscheme_for_server/path/to/somewhere");
	check("www.w3.org/Addressing/"); // Suffix reference - http://www.w3.org/Addressing/ ?
	check("www.libyuni.org"); // Suffix reference - http://www.libyuni.org ?
	check("google"); // Suffix reference - http://google.com ?
	check("mysite?query=1");
	check("mysite#fragment");
	check("mysite?query=1#fragment");
	check("#fragment_only");
	check("192.168.0.1");
	check("192.168.0.1/path");
	check("192.168.0.1?query=1");
	check("192.168.0.1#fragment");
	check("[2001:db8::7]");
	check("[2001:db8::7]/path");
	check("[2001:db8::7]?query=1");
	check("[2001:db8::7]#fragment");
	check("file:///remove/././dot/.././segments");
	check("file:///remove/././dot/../dot/./segments");
	check("file://.././../relative/path/.");
	check("file://.");
}



static void simpleTests()
{
	std::cout << "=== Simple test ===\n" << std::endl;
	// Original URI
	Yuni::Uri uri("http://www.example.org/?myquery=foo");
	std::cout << "Original : " << uri << std::endl;

	uri.query("anotherquery=bar");
	std::cout << "   query : " << uri << std::endl;

	uri.user("myuser");
	std::cout << "    user : " << uri << std::endl;

	std::cout << std::endl;
}





int main(void)
{
	simpleTests();
	uriBuildTests();
	return 0;
}
