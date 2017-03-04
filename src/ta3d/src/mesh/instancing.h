#ifndef __TA3D_INSTANCING_H__
#define __TA3D_INSTANCING_H__

#include "mesh.h"
#include <vector>
#include <misc/hash_table.h>

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

        void draw_queue(uint32 model_id) const;
    };

#define DrawingTable_SIZE		0x100
#define DrawingTable_MASK		0xFF

    class DrawingTable							// Kind of hash table used to speed up rendering of Instances of a mesh
    {
    private:
        UTILS::HashMap<RenderQueue, uint32>::Dense hash_table;

    public:
        DrawingTable();
        ~DrawingTable();

		void queue_Instance(uint32 model_id, const Instance &instance);
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

        void draw_queue( const GfxTexture::Ptr &texture_id, Vector3D *P, uint32 *C ) const;
    };


    class QUAD_TABLE							// Kind of hash table used to speed up rendering of separated quads
    {
    private:
        UTILS::HashMap< QUAD_QUEUE, GfxTexture::Ptr >::Dense hash_table;

    public:
        QUAD_TABLE();
        ~QUAD_TABLE();

        void queue_quad(const GfxTexture::Ptr &texture_id, const QUAD &quad );
        void draw_all();
    };



} // namespace TA3D

#endif
