/*
 * WARNING: this file uses bare OpenGL calls for now, and until Application::Gfx3D and/or friends implement
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
#include <yuni/gfx/metabox.h>

using namespace Yuni;
using namespace Yuni::Gfx;

/*
 * We define an Object3D that represents a box
 */
class OurBox: public Object3D
{
public:
	OurBox(ObjectModel::Ptr model)
		: Object3D(model)
	{
		// Create a metabox
		MetaBox* metaBox = new MetaBox(BoundingBox<>(Point3D<>(-8.0, -8.0, -8.0), Point3D<>(8.0, 8.0, 8.0)));

		// Create an implicit surface
		ImplicitSurface surf;
		// Add the box to the surface
		surf.addSubSurface(metaBox);

		// Create a mesh using marchingcubes with an isovalue of 0.05, and a 1.0 mesh size
		Mesh* mesh = MarchingCubes(surf)(0.05f, 1.0f);
		if (!mesh)
			throw std::runtime_error("Polygonization failed.");

		// Create a skeleton that uses this mesh, translate it in Z
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
		// It is advised to disconnect all events at this stage
		destroyBoundEvents();
	}

	void onFPSChanged(unsigned int fps)
	{
		// The FPS count has changed
		// We will set the application title according to the new value
		this->title(String() << "Marching cubes Metabox - " << fps << " fps");
	}
};


int main(int argc, char* argv[])
{
	OurBox* boxObj = new OurBox(new ObjectModel());

	/*
	 * Yuni main loop
	 */
	MeshTestApplication app(argc, argv);
	app.execute();
	return app.exitCode();
}
