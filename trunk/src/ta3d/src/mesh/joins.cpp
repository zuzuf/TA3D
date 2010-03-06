#include <misc/hash_table.h>
#include "string.h"
#include "mesh.h"
#include "joins.h"

using namespace std;

namespace TA3D
{
	Mesh *Joins::computeStructure(Mesh *mesh)
	{
		if (mesh == NULL)
			return NULL;

		typedef UTILS::HashMap<Mesh*>::Dense ObjectMap;
		typedef UTILS::HashSet<Mesh*>::Sparse ObjectSet;
		typedef UTILS::HashMap<Vector3D, Mesh*>::Dense ObjectPos;
		ObjectMap objects;
		ObjectSet rootObjects;
		ObjectPos objectPos;
		objects.set_empty_key(String());
		rootObjects.set_deleted_key(NULL);
		objectPos.set_empty_key(NULL);

		// Link all objects to their name
		std::vector<Mesh*> queue;
		queue.push_back(mesh);
		while(!queue.empty())
		{
			Mesh *cur = queue.back();
			queue.pop_back();
			while(cur)
			{
				if (!cur->name.empty())
					objects[cur->name] = cur;

				if (cur->child)
					queue.push_back(cur->child);
				cur = cur->next;
			}
		}

		// Break the structure
		for(ObjectMap::iterator i = objects.begin() ; i != objects.end() ; ++i)
		{
			i->second->child = NULL;
			i->second->next = NULL;
			rootObjects.insert(i->second);
			objectPos[i->second].reset();
		}

		// Build the new structure
		for(ObjectMap::iterator i = objects.begin() ; i != objects.end() ; ++i)
		{
			// If we have a "linker", remove it and link its 2 objects
			if (!i->first.contains('_') || i->second == NULL)
				continue;
			String::Vector elts;
			i->first.explode(elts, '_', false, false, true);
			if (elts.size() != 2) // WOW, that's not supposed to happen!
			{
				LOG_ERROR("[mesh] [joins] model structure is broken ! ('" << i->first << "'");
				if (rootObjects.count(i->second) > 0)
					rootObjects.erase(i->second);

				delete i->second;
				i->second = NULL;
				continue;
			}
			Vector3D pos;
			for(int j = 0 ; j < i->second->nb_vtx ; ++j)
				pos += i->second->points[j];
			pos = 1.0f / i->second->nb_vtx  * pos;

			Mesh *parent = objects[elts[0]];
			Mesh *child = objects[elts[1]];

			if (parent == NULL)		// Object missing ?
			{
				LOG_WARNING("[mesh] [joins] object not found : '" << elts[0] << "'");
				continue;
			}
			if (child == NULL)		// Object missing ?
			{
				LOG_WARNING("[mesh] [joins] object not found : '" << elts[1] << "'");
				continue;
			}

			child->pos_from_parent = pos - objectPos[parent];
			objectPos[child] = pos;
			for(int j = 0 ; j < child->nb_vtx ; ++j)
				child->points[j] -= pos;
			Mesh *cur = child->child;
			while(cur)
			{
				cur->pos_from_parent -= pos;
				cur = cur->next;
			}

			if (parent->child)		// Let's make a list
			{
				Mesh *cur = parent->child;
				while(cur->next)
					cur = cur->next;
				cur->next = child;
			}
			else
				parent->child = child;

			if (rootObjects.count(child) > 0)
				rootObjects.erase(child);

			if (rootObjects.count(i->second) > 0)
				rootObjects.erase(i->second);

			delete i->second;
			i->second = NULL;
		}

		Mesh *root = NULL;

		for(ObjectSet::iterator i = rootObjects.begin() ; i != rootObjects.end() ; ++i)
		{
			LOG_DEBUG("[mesh] [joins] root object : '" << (*i)->name << "'");
			if (root == NULL)
			{
				root = *i;
				continue;
			}
			Mesh *cur = root;
			while(cur->next)
				cur = cur->next;
			cur->next = *i;
		}

		return root;
	}

	void Joins::computeSelection(Model *model)
	{
		if (model == NULL || model->mesh == NULL)
			return;
		float s = 0.0f;
		vector<Mesh*> queue;
		vector< Vector3D > pqueue;
		queue.push_back(model->mesh);
		pqueue.push_back(Vector3D());
		while(!queue.empty())
		{
			Mesh *cur = queue.back();
			Vector3D P = pqueue.back();
			queue.pop_back();
			pqueue.pop_back();
			while(cur)
			{
				for(int i = 0 ; i < cur->nb_vtx ; ++i)
				{
					s = Math::Max(s, abs(P.x + cur->pos_from_parent.x + cur->points[i].x));
					s = Math::Max(s, abs(P.z + cur->pos_from_parent.z + cur->points[i].z));
				}
				if (cur->child)
				{
					queue.push_back(cur->child);
					pqueue.push_back(P + cur->pos_from_parent);
				}
				cur = cur->next;
			}
		}

		int pos = model->mesh->nb_vtx << 1;
		Vector3D *np = new Vector3D[(model->mesh->nb_vtx << 1) + 4];
		memcpy(np, model->mesh->points, sizeof(Vector3D) * model->mesh->nb_vtx);
		delete[] model->mesh->points;
		model->mesh->points = np;

		model->mesh->points[pos++] = Vector3D(-s, 1.0f, -s);
		model->mesh->points[pos++] = Vector3D(s, 1.0f, -s);
		model->mesh->points[pos++] = Vector3D(s, 1.0f, s);
		model->mesh->points[pos++] = Vector3D(-s, 1.0f, s);
		model->mesh->sel[0] = pos - 4;
		model->mesh->sel[1] = pos - 3;
		model->mesh->sel[2] = pos - 2;
		model->mesh->sel[3] = pos - 1;
		model->mesh->nb_vtx += 4;
		model->mesh->selprim = 0;
	}
}
