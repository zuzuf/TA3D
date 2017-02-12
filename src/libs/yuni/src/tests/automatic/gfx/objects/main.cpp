
#include <yuni/yuni.h>
#include <yuni/application/gfx3d.h>
#include <yuni/core/smartptr/smartptr.h>
#include <yuni/gfx/object3D.h>

using namespace Yuni;

class CubeModel: public Gfx::ObjectModel
{};


class Wheel: public Gfx::Object3D
{
public:
	Wheel(const String& name, const SmartPtr<Gfx::ObjectModel>& model)
		: Gfx::Object3D(name, model)
	{}
};


class Car: public Gfx::Object3D
{
public:
	Car(const String& name, const SmartPtr<Gfx::ObjectModel>& model)
		: Gfx::Object3D(name, model)
	{}
};


class MyAppWithObjects: public Application::Gfx3D
{
public:
	MyAppWithObjects(int argc, char* argv[])
		:Application::Gfx3D(argc, argv)
	{
		Gfx::Mesh::Ptr mesh(new Gfx::Mesh());
		Gfx::Skeleton::Ptr skeleton(new Gfx::Skeleton(mesh, Gfx::Vector3D<float>(), Gfx::Vector3D<float>()));
		Gfx::ObjectModel::Ptr carModel(new Gfx::ObjectModel(skeleton));
		// Use the same skeleton. This is a stupid test.
		Gfx::ObjectModel::Ptr wheelModel(new Gfx::ObjectModel(skeleton));

		Car* c = new Car("Titine", carModel);

		c->append(new Wheel("LeftForwardWheel", wheelModel));
		c->append(new Wheel("RightForwardWheel", wheelModel));
		*c += new Wheel("LeftRearWheel", wheelModel);
		*c << new Wheel("RightRearWheel", wheelModel);
	}
};


int main(int argc, char *argv[])
{
	MyAppWithObjects app(argc, argv);
	app.execute();
	return app.exitCode();
}
