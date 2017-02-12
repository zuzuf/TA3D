#include <stdexcept>
#include <vector>
#include <yuni/yuni.h>
#include <yuni/application/gfx3d.h>
#include <yuni/gfx/mesh.h>
#include <yuni/gfx/marchingcubes.h>
#include <yuni/gfx/metaball.h>

using namespace Yuni;
using namespace Yuni::Gfx;

class MergingMetaballs : public Object3D
{
private:
	//! First metaball
	MetaBall* pMetaball1;
	//! Second metaball
	MetaBall* pMetaball2;

	//! Current distance between metaballs centers
	float pDistance;
	//! Current direction of the movement: true = closer, false = away
	bool pDirection;

	//! Maximum allowed distance between the centers of the 2 metaballs
	static const float MAX_DISTANCE = 14.0f;

public:
	MergingMetaballs(ObjectModel::Ptr model)
		: Object3D(model), pDistance(MAX_DISTANCE), pDirection(true)
	{
		updateMesh();
	}

	virtual void animate()
	{
		// Change direction if we are at maximum or minimum value
		if (distance >= MAX_DISTANCE)
			direction = true;
		if (distance <= 0.0f)
			direction = false;
		// Modify the distance to have the metaballs come closer or farther
		pDistance += pDirection ? -0.5f : 0.5f;
		updateMesh();
	}

private:

	void updateMesh()
	{
		// Actually create the metaballs
		pMetaball1 = new MetaBall(Point3D<float>(-distance / 2.0f, 0.0f, 0.0f), 0.2f);
		pMetaball2 = new MetaBall(Point3D<float>(distance / 2.0f, 0.0f, 0.0f), 0.2f);

		// Create an empty implicit surface
		ImplicitSurface surf;

		// Add the metaballs to the surface
		surf.addSubSurface(pMetaball1);
		surf.addSubSurface(pMetaball2);

		// Create a mesh using marchingcubes with an isovalue of 0.05, and a 1.0 mesh size
		Mesh* mesh2 = MarchingCubes(surf)(0.05f, 1.0f);
		if (!mesh2)
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

	void onFPSChanged(int fps)
	{
		// The FPS count has changed
		// We will set the application title according the new value
		this->title(String() << "Marching cubes Merging metaballs - " << fps << " fps");
	}
};



int main(int argc, char* argv[])
{
	MergingMetaballs* metaballsObj = new MergingMetaballs(new ObjectModel());

	/*
	 * Yuni main loop
	 */
	MeshTestApplication app(argc, argv);
	app.execute();
	return app.exitCode();
}
