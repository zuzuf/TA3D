
#include <iostream>
#include <yuni/yuni.h>
#include <yuni/gfx/mesh.h>

/*!
** \brief Complex resources/mesh feature test
** This scenario tests the resources manager,
** mesh loading, conversion from irrlicht to a Yuni mesh,
** and executes a simple algorithm 
*/
int main(void)
{
	Yuni::Resources::SetRootDirectory("./");
	int modelID = Yuni::Resources::LoadModel("models/test.md3");
	Yuni::Gfx::Mesh* mesh = Yuni::Resources::GetMesh(modelID);
	if (0 == mesh)
		return 1;
	std::cout << "Triangle count:" << mesh->TriangleCount() << std::endl;
	std::cout << "Vertex count:" << mesh->VertexCount() << std::endl;
	std::cout << "Edge count:" << mesh->EdgeCount() << std::endl;
	return 0;
}
