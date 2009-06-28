#ifndef __TA3D_INSTANCING_H__
#define __TA3D_INSTANCING_H__

#include "mesh.h"
#include <vector>

namespace TA3D
{
    namespace INSTANCING
    {
        extern bool water;
        extern float sealvl;
    }

    class Instance
    {
    public:
        Vector3D	pos;
        uint32	    col;
        float	    angle;

        Instance(const Vector3D &p, const uint32 &c, const float &ang)
            :pos(p), col(c), angle(ang)
        {}
    };

    class RenderQueue
    {
    public:
        std::vector<Instance>	queue;
        uint32				    model_id;

        RenderQueue(const uint32 m_id) :queue(), model_id(m_id) {}
        ~RenderQueue() {}

        void draw_queue();
    };

#define DrawingTable_SIZE		0x100
#define DrawingTable_MASK		0xFF

    class DrawingTable							// Kind of hash table used to speed up rendering of Instances of a mesh
    {
    private:
        std::vector< std::vector< RenderQueue* > >		hash_table;

    public:
        DrawingTable() : hash_table() {hash_table.resize(DrawingTable_SIZE);}
        ~DrawingTable();

        void queue_Instance(uint32 &model_id, Instance instance);
        void draw_all();

    };


    class QUAD
    {
    public:
        Vector3D	pos;
        float	    size_x, size_z;
        uint32	    col;

        QUAD(const Vector3D &P, const float S_x, const float S_z, const uint32 c)
            :pos(P), size_x(S_x), size_z(S_z), col(c)
        {}

    };

    class QUAD_QUEUE
    {
    public:
        std::vector< QUAD > queue;
        GLuint              texture_id;

        QUAD_QUEUE( GLuint t_id ) : queue(), texture_id(t_id) {}

        ~QUAD_QUEUE() {}

        void draw_queue( Vector3D *P, uint32 *C );
    };


    class QUAD_TABLE							// Kind of hash table used to speed up rendering of separated quads
    {
    private:
        std::vector< std::vector< QUAD_QUEUE* > >		hash_table;

    public:
        QUAD_TABLE() : hash_table() {hash_table.resize(DrawingTable_SIZE);}
        ~QUAD_TABLE();

        void queue_quad( GLuint &texture_id, QUAD quad );
        void draw_all();
    };



} // namespace TA3D

#endif
