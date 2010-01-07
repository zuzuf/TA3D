#include "mesh.h"

struct Object
{
	int VersionSignature;
	int NumberOfVertexes;
	int NumberOfPrimitives;
	int OffsetToselectionPrimitive;
	int XFromParent;
	int YFromParent;
	int ZFromParent;
	int OffsetToObjectName;
	int Always_0;
	int OffsetToVertexArray;
	int OffsetToPrimitiveArray;
	int OffsetToSiblingObject;
	int OffsetToChildObject;
};

struct Primitive
{
	int ColorIndex;
	int NumberOfVertexIndexes;
	int Always_0;
	int OffsetToVertexIndexArray;
	int OffsetToTextureName;
	int Unknown_1;
	int Unknown_2;
	int IsColored;
};

void Mesh::load3DOrec(QFile &file)
{
	destroy();

	if (file.atEnd())
		return;

	type = MESH_TRIANGLES;

	Object header;
	file.read((char*)&header, sizeof(header));

	file.seek(header.OffsetToObjectName);
	name.clear();
	char c;
	do
	{
		file.getChar(&c);
		if (c)
			name.append(c);
	} while(c);

	double convFactor = 1.0 / 65536.0;

	pos = Vec(convFactor * header.XFromParent,
			  convFactor * header.YFromParent,
			  convFactor * header.ZFromParent);

	file.seek(header.OffsetToVertexArray);
	vertex.clear();
	for(int i = 0 ; i < header.NumberOfVertexes ; ++i)
	{
		int x, y, z;
		file.read((char*)&x, sizeof(x));
		file.read((char*)&y, sizeof(y));
		file.read((char*)&z, sizeof(z));
		vertex.append(Vec(convFactor * x, convFactor * y, convFactor * z));
	}

	index.clear();
	for(int i = 0 ; i < header.NumberOfPrimitives; ++i)
	{
		file.seek(header.OffsetToPrimitiveArray + i * sizeof(Primitive));

		Primitive primitive;
		file.read((char*)&primitive, sizeof(Primitive));

		file.seek(primitive.OffsetToVertexIndexArray);
		short *idx = new short[primitive.NumberOfVertexIndexes];
		file.read((char*)idx, sizeof(short) * primitive.NumberOfVertexIndexes);

		for(int e = 2 ; e < primitive.NumberOfVertexIndexes ; ++e)
		{
			index.push_back(idx[0]);
			index.push_back(idx[e - 1]);
			index.push_back(idx[e]);
		}

		delete[] idx;
	}

	computeNormals();

	defaultAnim = ANIMATION();

	if (header.OffsetToChildObject)
	{
		child = new Mesh;
		file.seek(header.OffsetToChildObject);
		child->load3DOrec(file);
	}
	else
		child = NULL;

	if (header.OffsetToSiblingObject)
	{
		next = new Mesh;
		file.seek(header.OffsetToSiblingObject);
		next->load3DOrec(file);
	}
	else
		next = NULL;
}

