
#include <yuni/yuni.h>
#include <yuni/gfx/point3D.h>
#include <iostream>


using namespace Yuni;





namespace
{

	void constructor()
	{
		Gfx::Point3D<int> p_int(10, 42, 3);
		std::cout << "Simple values (int): " << p_int.x << "," << p_int.y << "," << p_int.z << std::endl;

		Gfx::Point3D<float> p_ft(23.4f, 83.2894f, -2.69f);
		std::cout << "Simple values (float): " << p_ft.x << "," << p_ft.y << "," << p_ft.z << std::endl;
	}

	void printing()
	{
		std::cout << "Check ostream printing: " << Gfx::Point3D<sint64>(-42, 203923, 23) << std::endl;
	}

	void cond()
	{
		Gfx::Point3D<int> a(12, 45, 65);
		Gfx::Point3D<int> b(-324, 67, 2);
		Gfx::Point3D<int> c(12, 45, 65);
		std::cout << "Must be false (a == b): " << (a == b) << std::endl;
		std::cout << "Must be true (a == c): " << (a == c) << std::endl;
		std::cout << "Must be true (a != b): " << (a != b) << std::endl;
	}

	void assign()
	{
		Gfx::Point3D<sint16> a(1, 2, 1);
		Gfx::Point3D<sint16> b(3, 4, 6);
		b = a;
		std::cout << "Assign (1,2): " << b.x << "," << b.y << "," << b.z << std::endl;
	}

	void convertions()
	{
		Gfx::Point3D<sint8> p(23, 56, 2.3f);
		std::cout << "sint8 from int (23,56): " << (int)p.x << "," << (int)p.y << std::endl;

		Gfx::Point3D<double> d(p);
		std::cout << "double from sint8 : " << d << std::endl;
		d += (int) 42;
		std::cout << "Inc with an int (+42): " << d << std::endl;
		d.reset();
		std::cout << "Reset:  " << d << std::endl;
		d = Gfx::Point3D<int>(10, 12.0f, 1.2);
		d.translate(2, 4.5f, 3);
		std::cout << "Translate:  " << d << std::endl;
	}

	void additions()
	{
		Gfx::Point3D<int> a(1, 2);
		Gfx::Point3D<int> b(2, 4);

		Gfx::Point3D<int> c = a + b;
		std::cout << a << " + " << b << " = " << c << std::endl;
		Gfx::Point3D<float> f = a + b;
		std::cout << "(float) " << a << " + " << b << " = " << f << std::endl;
		f += 3.0f;
		std::cout << "f += 3 " << " = " << f << std::endl;
	}

	void close()
	{
		Gfx::Point3D<float> a(10, 10);
		Gfx::Point3D<float> b(10.1, 9.8);
		std::cout << a << " close to " << b << ", delta: (int)1 = " << a.closeTo(b, 1) << std::endl;
		std::cout << a << " close to " << b << ", delta: (int)0 = " << a.closeTo(b, 0) << std::endl;
		std::cout << a << " close to " << b << ", delta: 0.3f   = " << a.closeTo(b, 0.3f) << std::endl;
		std::cout << a << " close to " << b << ", delta: 0.1f   = " << a.closeTo(b, 0.1f) << std::endl;
	}

} // anonymous namespace




int main(void)
{
	constructor();
	printing();
	cond();
	assign();
	convertions();
	additions();
	close();
	return 0;
}

