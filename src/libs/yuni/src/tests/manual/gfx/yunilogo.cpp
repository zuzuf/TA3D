/*
 * WARNING: this file uses bare irrlicht calls for now, and until Application::Gfx3D and/or friends implements
 * methods to display something on the devices.
 *
 * This is mainly a sample to enable Loomchild to test his Polygonizers algorithms.
 *
 */
#include <stdexcept>
#include <vector>
#include <yuni/yuni.h>
#include <yuni/application/gfx3d.h>
#include <yuni/gfx/mesh.h>
#include <yuni/gfx/marchingcubes.h>
#include <yuni/gfx/metaball.h>


using namespace Yuni;
using namespace Yuni::Gfx;


class YuniLogo : public Object3D
{
public:
	YuniLogo(ObjectModel::Ptr model)
		: Object3D(model)
	{
		// Create an empty implicit surface
		ImplicitSurface surf;

		// The following coordinates are meant to resemble the Yuni logo, in 2d, like this:
		// I don't know if the result can look good or not, but it sounded like a fun idea.
		/*

		                * *
		    * *      *
		  *    *   *
		       * *
		       *
		      *
		     *

		 */

		// Here's a scaling factor.
# define FACT 4.0f

		// Add the metaballs to the implicit surface
		surf.addSubSurface(new MetaBall(Point3D<float>(-3.5631f * FACT, -1.1995f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(-2.6458f * FACT, -1.6933f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(-2.0f * FACT, -1.8f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(-1.1289f * FACT, -1.6933f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(-0.9172f * FACT, -0.9172f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(-1.1289f * FACT, 0.3528f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(-1.27f * FACT, 1.4464f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(-1.8344f * FACT, 2.3283f * FACT, 0.0f), 0.2f));
		surf.addSubSurface(new MetaBall(Point3D<float>(-2.3989f * FACT, 3.2103f * FACT, 0.0f), 0.2f));
		surf.addSubSurface(new MetaBall(Point3D<float>(-0.0353f * FACT, 0.6703f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(1.1289f * FACT, -0.1764f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(2.2931f * FACT, -1.0231f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(3.4572f * FACT, -1.8697f * FACT, 0.0f), 0.1f));
		surf.addSubSurface(new MetaBall(Point3D<float>(4.7272f * FACT, -2.2931f * FACT, 0.0f), 0.1f));

		// Create a mesh using marchingcubes with an isovalue of 0.05, and a 2.0 mesh size
		Mesh* mesh = MarchingCubes(surf)(0.05f, 0.5f);
		if (!mesh)
			throw std::runtime_error("Polygonization failed.");

		// Create a skeleton that uses this mesh, translate it in Z and rotate the logo properly
		Skeleton* skel = new Skeleton(Mesh::Ptr(mesh), Vector3D<>(0.0f, 0.0f, -30.0f), Vector3D<>(0.0f, 0.0f, 0.0f));
		// Set the skeleton in our ObjectModel
		pModel->setSkeleton(Skeleton::Ptr(skel));
	}
};


class MeshTestApplication : public Application::Gfx3D
{
public:
	MeshTestApplication(int argc, char* argv[])
		:Application::Gfx3D(argc, argv)
	{
		// We connect our own method to the event
		Gfx::Engine::Instance()->onFPSChanged.connect(this, &MeshTestApplication::onFPSChanged);
	}

	virtual ~MeshTestApplication()
	{
		// It is advised to disconnect all events at this stade
		destroyBoundEvents();
	}

	void onFPSChanged(unsigned int fps)
	{
		// The FPS count has changed
		// We will set the application title according the new value
		this->title(String() << "Marchig cubes Yuni Logo - " << fps << " fps");
	}
};




int main(int argc, char* argv[])
{
	YuniLogo logoObject(new ObjectModel());

	/*
	 * Yuni main loop
	 */
	MeshTestApplication app(argc, argv);
	app.execute();
	return app.exitCode();
}
