#ifndef __TA3D_MISC_BVH_H__
#define __TA3D_MISC_BVH_H__

#include "vector.h"
#include <vector>
#include <deque>
#include "mempool.h"

namespace TA3D
{
	/* This class implements a BVH (Bounding Volume Hierarchie) structure
	 *
	 * The tree is built at object creation from a std::vector<T>.
	 * In order to use this sturcture you must define a TKit class which
	 * supports the following operations:
	 * TKit::Vec TKit::pos(const T &elt);
	 * float TKit::radius(const T &elt);
	 * void TKit::getTopBottom(const std::vector<T> &elts, TKit::Vec &top, TKit::Vec &bottom);
	 * TKit::Vec TKit::getPrincipalDirection(const TKit::Vec &v);
	 *
	 * The TKit class must also provide the following types:
	 * TKit::Vec - a vector type
	 * TKit::Predicate - a TKit::Vec predicate type to be used with std::partition
	 *                    and containing the direction of the projection and a position (the split point)
	 */
	template<typename T, typename TKit>
			class BVH
	{
	public:
		typedef typename TKit::Vec	Vec;
	public:
		inline BVH() : lChild(NULL), rChild(NULL)	{}
		inline ~BVH();

		inline void build(MemoryPool< BVH<T, TKit> > *pool, const typename std::vector<T>::iterator &begin, const typename std::vector<T>::iterator &end, const unsigned int l = 0U);

		inline void boxCollisionQuery(std::deque<T> &result, const Vec &center, const float maxDist) const;

		static inline BVH *create(MemoryPool< BVH<T, TKit> > *pool, const typename std::vector<T>::iterator &begin, const typename std::vector<T>::iterator &end)
		{
			BVH *bvh = pool->alloc();
			bvh->build(pool, begin, end);
			return bvh;
		}

	private:
		typename std::vector<T>::const_iterator elements_begin;
		typename std::vector<T>::const_iterator elements_end;
		Vec bottom, top;
		BVH<T, TKit>	*lChild;
		BVH<T, TKit>	*rChild;
		MemoryPool< BVH<T, TKit> > *pool;
	};
}

#include "bvh.hxx"

#endif
