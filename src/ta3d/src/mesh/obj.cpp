#include "obj.h"
#include "joins.h"

namespace TA3D
{
	REGISTER_MESH_TYPE(MeshOBJ);

	using namespace MOBJ;
	using namespace std;

	MeshOBJ::MeshOBJ()
	{
		Color = 0xFFFFFFFF;
	}

	/*!
* \brief fill the Mesh with gathered data
*/
	void MeshOBJ::obj_finalize(const QString &, const vector<int> &face, const vector<Vector3D> &vertex, const vector<Vector2D> &tcoord, Material* mtl)
	{
		nb_vtx = short(face.size() >> 1);
		nb_t_index = short(face.size() >> 1);
		this->t_index = new GLushort[nb_t_index];
		this->points = new Vector3D[nb_vtx * 2];
		this->tcoord = new float[2 * nb_vtx];
		type = MESH_TYPE_TRIANGLES;

		for (int i = 0 ; i < nb_t_index ; i++)
		{
			this->points[i] = vertex[ face[i * 2] ];
			if (face[i * 2 + 1] == -1)
			{
				this->tcoord[i * 2] = 0.0f;
				this->tcoord[i * 2 + 1] = 0.0f;
			}
			else
			{
				this->tcoord[i * 2] = tcoord[ face[i * 2 + 1]].x;
				this->tcoord[i * 2 + 1] = 1.0f - tcoord[face[i * 2 + 1]].y;
			}
			this->t_index[i] = GLushort(i);
		}

		N = new Vector3D[nb_vtx << 1]; // Compute normals
		if (nb_t_index > 0 && t_index != NULL)
		{
			F_N = new Vector3D[nb_t_index / 3];
			for (int i = 0; i < nb_vtx; ++i)
				N[i].reset();
			int e = 0;
			for (int i = 0 ; i < nb_t_index ; i += 3)
			{
				Vector3D AB, AC, Normal;
				AB = points[t_index[i+1]] - points[t_index[i]];
				AC = points[t_index[i+2]] - points[t_index[i]];
				Normal = AB * AC;
				Normal.unit();
				F_N[e++] = Normal;
				for (byte e = 0; e < 3; ++e)
					N[t_index[i + e]] = N[t_index[i + e]] + Normal;
			}
			for (int i = 0; i < nb_vtx; ++i)
				N[i].unit();
		}

		Flag = SURFACE_ADVANCED | SURFACE_LIGHTED | SURFACE_GOURAUD;
		Color = 0xFFFFFFFF;
		if (mtl)
		{
			tex_cache_name.push_back(mtl->textureName);
			if (mtl->name == "team")				// The magic team material
				Flag |= SURFACE_PLAYER_COLOR;
		}
	}

	void MeshOBJ::load_texture_id(int id)
	{
		if (id < 0 || id >= (int)tex_cache_name.size())
			return;
		bool useAlpha(false);
		LOG_DEBUG(LOG_PREFIX_OBJ << "loading texture : '" << tex_cache_name[id] << "'");
		gltex.push_back( gfx->load_texture( tex_cache_name[id], FILTER_TRILINEAR, NULL, NULL, true, 0, &useAlpha, true ) );
		if (gltex.back())
		{
			Flag |= SURFACE_TEXTURED;
			if (useAlpha)
				Flag |= SURFACE_BLENDED;
		}
		else
			LOG_ERROR(LOG_PREFIX_OBJ << "could not load texture ! (" << tex_cache_name[id] << ')');
	}

	void MeshOBJ::load(File *file, const QString &filename)
	{
		destroy3DM();

		MeshOBJ *cur = this;
		name = "default";
		bool firstObject = true;
		vector<Vector3D>	lVertex;
		vector<Vector2D>	lTcoord;
		vector<int>			face;
		HashMap<Material>::Dense	mtllib;
		Material                    currentMtl;

		while (!file->eof()) // Reads the whole file
		{
			QString line;
			file->readLine(line);
            line = line.trimmed();
            QStringList args = line.split(' ', QString::SkipEmptyParts);
            if (!args.isEmpty())
			{
				if ( (args[0] == "o" || args[0] == "g") && args.size() > 1)      // Creates a new object
				{
                    if (!face.empty())
					{
                        if (firstObject && cur->name.isEmpty())
							cur->name = "default";
						cur->obj_finalize( filename, face, lVertex, lTcoord, &currentMtl );
						face.clear();
						cur->child = new MeshOBJ();
						cur = static_cast<MeshOBJ*>(cur->child);
					}
					firstObject = false;
					cur->name = args[1];
				}
				else if (args[0] == "mtllib" && args.size() > 1)        // Read given material libraries
				{
					for(QStringList::iterator s = args.begin() + 1 ; s != args.end() ; ++s)
					{
                        File *src_mtl = VFS::Instance()->readFile("objects3d/" + *s);
						if (!src_mtl)
							continue;
						Material mtl;
						while (!src_mtl->eof())
						{
							QString line0;
							src_mtl->readLine(line0);
                            line0 = line0.trimmed();
                            const QStringList &args0 = line0.split(' ', QString::SkipEmptyParts);
                            if (!args0.isEmpty())
							{
								if (args0[0] == "newmtl")
									mtl.name = args0[1];
								else
								{
									if (args0[0] == "map_Kd")
									{
                                        mtl.textureName = "textures/" + args0[1];
										mtllib[mtl.name] = mtl;
									}
								}
							}
						}
						delete src_mtl;
					}
				}
				else
				{
					if (args[0] == "usemtl" && args.size() > 1)        // Change current material
					{
						if (mtllib.count(args[1]))
							currentMtl = mtllib[args[1]];
						else
							currentMtl.textureName.clear();
					}
					else if (args[0] == "v" && args.size() > 3)  // Add a vertex to current object
                        lVertex.push_back( Vector3D(args[1].toFloat(), args[2].toFloat(), args[3].toFloat()));
					else if (args[0] == "vn" && args.size() > 3)  // Add a normal vector to current object
					{}
					else if (args[0] == "vt" && args.size() > 2)  // Add a texture coordinate vector to current object
                        lTcoord.push_back( Vector2D( args[1].toFloat(), args[2].toFloat()));
					else if (args[0] == "f" && args.size() > 1)  // Add a face to current object
					{
						vector<int>  vertex_idx;
						vector<int>  tcoord_idx;
						bool first_string = true;
                        for(QString &s : args)
						{
							// The first string is crap if we read it as vertex data !!
							if (first_string)
							{
								first_string = false;
								continue;
							}

                            s = s.trimmed();
                            const QStringList &data = s.split('/', QString::SkipEmptyParts);

                            if (!data.isEmpty())
							{
                                vertex_idx.push_back( data[0].toInt() - 1);
								if (vertex_idx.back() < 0)
                                {	LOG_DEBUG(LOG_PREFIX_OBJ << "parser : " << line << " -> " << s << " -> " << vertex_idx.back());	}
								if (data.size() >= 2)
                                    tcoord_idx.push_back(data[1].toInt() - 1);
								else
									tcoord_idx.push_back(-1);
							}
						}

						for (uint32 i = 2; i < vertex_idx.size(); ++i) // Make triangles (FAN method)
						{
							face.push_back(vertex_idx[0]);
							face.push_back(tcoord_idx[0]);

							face.push_back(vertex_idx[i-1]);
							face.push_back(tcoord_idx[i-1]);

							face.push_back(vertex_idx[i]);
							face.push_back(tcoord_idx[i]);
						}
					}
				}
			}
		}

		cur->obj_finalize(filename, face, lVertex, lTcoord, &currentMtl);
	}

	Model *MeshOBJ::load(const QString &filename)
	{
		File *file = VFS::Instance()->readFile(filename);
		if (!file)
		{
			LOG_ERROR(LOG_PREFIX_OBJ << "could not read file '" << filename << "'");
			return NULL;
		}

		MeshOBJ *mesh = new MeshOBJ;
		mesh->load(file, filename);
		delete file;

		Model *model = new Model;
		model->mesh = Joins::computeStructure(mesh, filename);
		model->postLoadComputations();
		Joins::computeSelection(model);
		return model;
	}

	const char *MeshOBJ::getExt()
	{
		return ".obj";
	}
}
