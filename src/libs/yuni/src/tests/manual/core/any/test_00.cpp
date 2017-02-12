#include <iostream>
#include <string>
#include <complex>
#include <yuni/core/string.h>
#include <yuni/core/any.h>

using namespace Yuni;
using namespace std;

void outputAny(const Any& a, ostream& o)
{
#define IMPLEMENT_OUT(type) \
	if (a.is<type>()) \
	{ \
		o << a.to<type>() << endl; \
		return; \
	}

	IMPLEMENT_OUT(int)
	IMPLEMENT_OUT(char)
	IMPLEMENT_OUT(double)
	IMPLEMENT_OUT(float)
	IMPLEMENT_OUT(String)
	IMPLEMENT_OUT(complex<int>)

	o << "Unexpected type: " << a.type().name() << endl;
	throw 0;
}

void simpleTest()
{
	cout << "simple test of Yuni::Any" << endl;

	cout << "expecting 42    : "; outputAny(42, cout);
	cout << "expecting q     : "; outputAny('q', cout);
	cout << "expecting 3.14  : "; outputAny(3.14, cout);
	cout << "expecting (1,2) : "; outputAny(complex<int>(1,2), cout);
	cout << "expecting piko  : "; outputAny("piko", cout);

	cout << "expecting 42    : "; int n = 42; outputAny(n, cout);
	cout << "expecting q     : "; char c = 'q'; outputAny(c, cout);
	cout << "expecting 3.14  : "; double d = 3.14; outputAny(d, cout);
	cout << "expecting (1,2) : "; complex<int> x(1,2); outputAny(x, cout);
	cout << "expecting piko  : "; const char * s = "piko"; outputAny(s, cout);

	Yuni::Any a;
	cout << "expecting 42    : "; outputAny(a = n, cout);
	cout << "expecting q     : "; outputAny(a = c, cout);
	cout << "expecting 3.14  : "; outputAny(a = d, cout);
	cout << "expecting (1,2) : "; outputAny(a = x, cout);
	cout << "expecting 13    : "; outputAny(a = 13, cout);
	cout << "expecting nyu   : "; outputAny(s = "nyu", cout);

	cout << "Empty tests" << endl;
	cout << "expecting false : " << boolalpha << a.empty() << endl;
	a = 0;
	cout << "expecting false : " << boolalpha << a.empty() << endl;
	a.reset();
	cout << "expecting true  : " << boolalpha << a.empty() << endl;

	try
	{
		cout << a.to<int>() << endl;
		cout << "failure: expected error" << endl;
	}
	catch (...)
	{
		cout << "success: casting error" << endl;
	}
}




int main(void)
{
	simpleTest();
	return 0;
}
