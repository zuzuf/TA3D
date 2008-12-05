
#include "obj.h"
#include "../misc/paths.h"


namespace TA3D
{
namespace Converters
{



    class Material
    {
    public:
        String  name;
        String  textureName;

        Material() : name(), textureName()  {}
        Material(int a) : name(), textureName() {}
    };


    namespace
    {

        /*!
         * \brief fill the OBJECT with gathered data
         */
        void finalize_object( OBJECT *cur, std::vector<int> &face, std::vector<Vector3D> &vertex,
                              std::vector<Vector3D> &normal, std::vector<Vector2D> &tcoord, Material* mtl = NULL)
        {
            cur->nb_vtx = face.size() >> 1;
            cur->nb_t_index = face.size() >> 1;
            cur->t_index = new GLushort[cur->nb_t_index];
            cur->points = new Vector3D[cur->nb_vtx];
            cur->N = new Vector3D[cur->nb_vtx];
            cur->tcoord = new float[2 * cur->nb_vtx];

            for (int i = 0 ; i < cur->nb_t_index ; i++)
            {
                cur->points[i] = vertex[ face[i * 2]];
                cur->tcoord[i * 2] = tcoord[ face[i * 2 + 1]].x;
                cur->tcoord[i * 2 + 1] = 1.0f - tcoord[face[i * 2 + 1]].y;
                cur->t_index[i] = i;
            }

            for (int i = 0 ; i < cur->nb_vtx; ++i)
                cur->N[i].x = cur->N[i].y = cur->N[i].z = 0.0f;
            for (int i = 0 ; i < cur->nb_vtx; i += 3)
            {
                Vector3D AB = cur->points[i + 1] - cur->points[i];
                Vector3D AC = cur->points[i + 2] - cur->points[i];
                Vector3D N = AB * AC;
                N.unit();
                cur->N[i] = cur->N[i] + N;
                cur->N[i+1] = cur->N[i+1] + N;
                cur->N[i+2] = cur->N[i+2] + N;
            }
            for (int i = 0 ; i < cur->nb_vtx ; i++)
                cur->N[i].unit();

            cur->surface.Flag = SURFACE_ADVANCED | SURFACE_LIGHTED | SURFACE_GOURAUD;
            if (mtl)
            {
                cur->surface.gltex[0] = gfx->load_texture( mtl->textureName );
                if (cur->surface.gltex[0])
                {
                    cur->surface.NbTex = 1;
                    cur->surface.Flag |= SURFACE_TEXTURED;
                }
            }
        }

    } // anonymous namespace




    MODEL* OBJ::ToModel(const String& filename, const float scale)
    {
        FILE *src_obj = fopen( filename.c_str(), "r");
        MODEL *model_obj = new MODEL();
        if (src_obj)
        {
            char buf[1024];

            OBJECT *cur = &(model_obj->obj);
            bool firstObject = true;
            std::vector<Vector3D>  vertex;
            std::vector<Vector3D>  normal;
            std::vector<Vector2D>  tcoord;
            std::vector<int>       face;
            cHashTable<Material>   mtllib;
            Material               currentMtl;

            while (fgets( buf, 1024, src_obj)) // Reads the while file
            {
                String::Vector args;
                String(buf).split(args, " ");
                if (!args.empty())
                {
                    if ( (args[0] == "o" || args[0] == "g") && args.size() > 1)      // Creates a new object
                    {
                        if (!face.empty())
                        {
                            if (firstObject && cur->name.empty())
                                cur->name = "default";
                            finalize_object( cur, face, vertex, normal, tcoord, &currentMtl );
                            face.clear();
                            cur->child = new OBJECT();
                            cur = cur->child;
                        }
                        firstObject = false;
                        cur->name = args[1];
                        printf("[obj] new object '%s'\n", args[1].c_str());
                    }
                    else
                        if (args[0] == "mtllib" && args.size() > 1)        // Read given material libraries
                        {
                            for (String::Vector::const_iterator i = args.begin(); i != args.end(); ++i)
                            {
                                FILE *src_mtl = fopen((TA3D::Paths::ExtractFilePath(filename) + *i).c_str(), "r");
                                if (src_mtl)
                                {
                                    Material mtl;
                                    while (fgets(buf, 1024, src_mtl))
                                    {
                                        String::Vector args0;
                                        String(buf).split(args0, " ");
                                        if (args0.size() > 0)
                                        {
                                            if (args0[0] == "newmtl")
                                                mtl.name = args0[1];
                                            else
                                            {
                                                if (args0[0] == "map_Kd")
                                                {
                                                    mtl.textureName = TA3D::Paths::ExtractFilePath(filename) + args0[1];
                                                    mtllib.insertOrUpdate(mtl.name, mtl);
                                                }
                                            }
                                        }
                                    }
                                    fclose(src_mtl);
                                }
                            }
                        }
                        else
                        {
                            if (args[0] == "usemtl" && args.size() > 1)        // Change current material
                            {
                                if (mtllib.exists(args[1]))
                                    currentMtl = mtllib.find(args[1]);
                                else
                                    currentMtl.textureName.clear();
                            }
                            else if (args[0] == "v" && args.size() > 3)  // Add a vertex to current object
                                vertex.push_back( Vector3D(args[1].toFloat(), args[2].toFloat(), args[3].toFloat()));
                            else if (args[0] == "vn" && args.size() > 3)  // Add a normal vector to current object
                                normal.push_back( Vector3D(args[1].toFloat(), args[2].toFloat(), args[3].toFloat()));
                            else if (args[0] == "vt" && args.size() > 2)  // Add a texture coordinate vector to current object
                                tcoord.push_back( Vector2D( args[1].toFloat(), args[2].toFloat()));
                            else if (args[0] == "f" && args.size() > 1)  // Add a face to current object
                            {
                                std::vector<int>  vertex_idx;
                                std::vector<int>  tcoord_idx;
                                std::vector<int>  normal_idx;
                                for (String::Vector::const_iterator i = args.begin(); i != args.end(); ++i)
                                {
                                    // The first string is crap if we read it as vertex data !!
                                    if (i == args.begin())
                                        continue;

                                    String::Vector data;
                                    i->split(data, "/");

                                    if (data.size() > 0 )
                                    {
                                        vertex_idx.push_back( data[0].toInt32() - 1);
                                        if (data.size() == 3)
                                        {
                                            if (data[1].empty())
                                                tcoord_idx.push_back(-1);
                                            else
                                                tcoord_idx.push_back(data[1].toInt32() - 1);

                                            if (data[2].empty())
                                                normal_idx.push_back(-1);
                                            else
                                                normal_idx.push_back(data[2].toInt32() - 1);
                                        }
                                        else
                                        {
                                            tcoord_idx.push_back(-1);
                                            normal_idx.push_back(-1);
                                        }
                                    }
                                }

                                for (unsigned int i = 2; i < vertex_idx.size(); ++i) // Make triangles (FAN method)
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

            if (!firstObject)
                finalize_object(cur, face, vertex, normal, tcoord);

            fclose(src_obj);

            model_obj->nb_obj = model_obj->obj.set_obj_id(0);

            Vector3D O;
            int coef(0);
            model_obj->center.reset();
            model_obj->obj.compute_center(&model_obj->center, O, &coef);
            model_obj->center = (1.0f/coef) * model_obj->center;
            model_obj->size = 2.0f * model_obj->obj.compute_size_sq(model_obj->center); // On garde le carré pour les comparaisons et on prend une marge en multipliant par 2.0f
            model_obj->size2 = sqrtf(0.5f*model_obj->size);
            model_obj->obj.compute_emitter();
            model_obj->compute_topbottom();
        }
        return model_obj;

    }


} // namespace Converters
} // namespace TA3D


