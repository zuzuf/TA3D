/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

/*-----------------------------------------------------------------------------------\
|                                     instancing.cpp                                 |
|  This module contains objects used to render a set of similar meshes or quads      |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include "mesh.h"
#include "instancing.h"


namespace TA3D
{
    namespace INSTANCING
    {
        bool water = false;
        float sealvl = 0.0f;
    }

    DrawingTable::~DrawingTable()
    {
		for (std::vector< std::vector< RenderQueue* > >::iterator i = hash_table.begin() ; i != hash_table.end() ; ++i)
			for (std::vector< RenderQueue* >::iterator e = i->begin(); e != i->end(); ++e)
				delete *e;
        hash_table.clear();
    }


	void DrawingTable::queue_Instance( uint32 model_id, const Instance &instance )
    {
        uint32	hash = model_id & DrawingTable_MASK;
        for (std::vector< RenderQueue* >::iterator i = hash_table[ hash ].begin(); i != hash_table[hash].end(); ++i)
        {
            if ((*i)->model_id == model_id)// We found an already existing render queue
            {
                (*i)->queue.push_back( instance );
                return;
            }
        }
        RenderQueue *renderQueue = new RenderQueue( model_id );
        hash_table[ hash ].push_back( renderQueue );
        renderQueue->queue.push_back( instance );
    }


    void DrawingTable::draw_all()
    {
		for (std::vector< std::vector< RenderQueue* > >::iterator i = hash_table.begin() ; i != hash_table.end() ; ++i)
			for (std::vector< RenderQueue* >::iterator e = i->begin() ; e != i->end() ; ++e )
                (*e)->draw_queue();
    }


    void RenderQueue::draw_queue()
    {
        if (queue.empty())
            return;
        glPushMatrix();

		Model *model = model_manager.model[ model_id ];

		if (model->from_2d)
            glEnable(GL_ALPHA_TEST);

        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
		if (!model->dlist)	// Build the display list if necessary
        {
            model->check_textures();
            model->dlist = glGenLists (1);
            glNewList (model->dlist, GL_COMPILE);
            model->mesh->draw_nodl();
            glEndList();
        }

        for (std::vector< Instance >::iterator i = queue.begin() ; i != queue.end() ; ++i)
        {
            glPopMatrix();
            glPushMatrix();
            glTranslatef( i->pos.x, i->pos.y, i->pos.z );
            glRotatef( i->angle, 0.0f, 1.0f, 0.0f );
            glColor4ubv( (GLubyte*) &i->col );
            glCallList( model->dlist );
            if (lp_CONFIG->underwater_bright && INSTANCING::water && i->pos.y < INSTANCING::sealvl)
            {
                glEnable( GL_BLEND );
                glBlendFunc( GL_ONE, GL_ONE );
                glDepthFunc( GL_EQUAL );
                glColor4ub( 0x7F, 0x7F, 0x7F, 0x7F );
                model->draw(0.0f, NULL, false, true, false, 0, NULL, NULL, NULL, 0.0f, NULL, false, 0, false);
                glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
                glDepthFunc( GL_LESS );
                glDisable( GL_BLEND );
            }
        }

        if (model->from_2d)
            glDisable(GL_ALPHA_TEST);

        glPopMatrix();
    }


    QUAD_TABLE::~QUAD_TABLE()
    {
		for (int i = 0 ; i < hash_table.size() ; ++i)
        {
            for (std::vector< QUAD_QUEUE* >::iterator e = hash_table[ i ].begin() ; e != hash_table[ i ].end() ; ++e)
				delete *e;
        }
        hash_table.clear();
    }


	void QUAD_TABLE::queue_quad(GLuint& texture_id, const QUAD &quad)
    {
        uint32	hash = texture_id & DrawingTable_MASK;
        for (std::vector< QUAD_QUEUE* >::iterator i = hash_table[ hash ].begin() ; i != hash_table[ hash ].end() ; ++i)
        {
            if ((*i)->texture_id == texture_id ) // We found an already existing render queue
            {
                (*i)->queue.push_back( quad );
                return;
            }
        }
        QUAD_QUEUE *quad_queue = new QUAD_QUEUE(texture_id);
        hash_table[ hash ].push_back(quad_queue);
        quad_queue->queue.push_back(quad);
    }


    void QUAD_TABLE::draw_all()
    {
        uint32	max_size = 0;
        for (uint16 i = 0; i < DrawingTable_SIZE ; ++i)
        {
            for (std::vector< QUAD_QUEUE* >::iterator e = hash_table[i].begin(); e != hash_table[i].end(); ++e)
                max_size = Math::Max( max_size, (uint32)(*e)->queue.size());
        }

        Vector3D	*P = new Vector3D[ max_size << 2 ];
        uint32	*C = new uint32[ max_size << 2 ];
        GLfloat	*T = new GLfloat[ max_size << 3 ];

        int e = 0;
        for (unsigned int i = 0 ; i < max_size; ++i)
        {
            T[e << 1] = 0.0f;  T[(e<<1)+1] = 0.0f;
            ++e;

            T[e << 1] = 1.0f;  T[(e<<1)+1] = 0.0f;
            ++e;

            T[e << 1] = 1.0f;  T[(e<<1)+1] = 1.0f;
            ++e;

            T[e << 1] = 0.0f;  T[(e<<1)+1] = 1.0f;
            ++e;
        }

        glEnableClientState(GL_VERTEX_ARRAY);		// vertex coordinates
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnable( GL_TEXTURE_2D );
        glColorPointer(4,GL_UNSIGNED_BYTE,0,C);
        glVertexPointer( 3, GL_FLOAT, 0, P);
        glTexCoordPointer(2, GL_FLOAT, 0, T);

        for (uint16 i = 0; i < DrawingTable_SIZE; ++i)
        {
            for (std::vector< QUAD_QUEUE* >::iterator e = hash_table[i].begin(); e != hash_table[i].end(); ++e)
                (*e)->draw_queue(P, C);
        }

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		DELETE_ARRAY(P);
		DELETE_ARRAY(C);
		DELETE_ARRAY(T);
    }


    void QUAD_QUEUE::draw_queue(Vector3D *P, uint32 *C)
    {
        if (queue.empty())
            return;

        Vector3D *p = P;
        uint32 *c = C;
        for (std::vector<QUAD>::iterator e = queue.begin(); e != queue.end(); ++e)
        {
            p->x = e->pos.x - e->size_x;
            p->y = e->pos.y;
            p->z = e->pos.z - e->size_z;
            *c++ = e->col;
            ++p;

            p->x = e->pos.x + e->size_x;
            p->y = e->pos.y;
            p->z = e->pos.z - e->size_z;
            *c++ = e->col;
            ++p;

            p->x = e->pos.x + e->size_x;
            p->y = e->pos.y;
            p->z = e->pos.z + e->size_z;
            *c++ = e->col;
            ++p;

            p->x = e->pos.x - e->size_x;
            p->y = e->pos.y;
            p->z = e->pos.z + e->size_z;
            *c++ = e->col;
            ++p;
        }
        glBindTexture( GL_TEXTURE_2D, texture_id );
        glDrawArrays(GL_QUADS, 0, queue.size()<<2);		// draw those quads

        if (lp_CONFIG->underwater_bright && INSTANCING::water)
        {
            p = P;
            int i = 0;
            for (std::vector<QUAD>::iterator e = queue.begin(); e != queue.end(); ++e)
            {
                if (e->pos.y >= INSTANCING::sealvl) continue;
                p->x = e->pos.x - e->size_x;
                p->y = e->pos.y;
                p->z = e->pos.z - e->size_z;
                ++p;

                p->x = e->pos.x + e->size_x;
                p->y = e->pos.y;
                p->z = e->pos.z - e->size_z;
                ++p;

                p->x = e->pos.x + e->size_x;
                p->y = e->pos.y;
                p->z = e->pos.z + e->size_z;
                ++p;

                p->x = e->pos.x - e->size_x;
                p->y = e->pos.y;
                p->z = e->pos.z + e->size_z;
                ++p;
                ++i;
            }
            memset(C, 0x7F, i * 16);

            if (i > 0)
            {
                glEnable( GL_BLEND );
                glDisable( GL_TEXTURE_2D );
                glBlendFunc( GL_ONE, GL_ONE );
                glDepthFunc( GL_EQUAL );
                glDrawArrays(GL_QUADS, 0, i << 2);		// draw those quads
                glDepthFunc( GL_LESS );
                glEnable( GL_TEXTURE_2D );
                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            }
        }
    }

} // namespace TA3D

