#ifndef __TA3D_MISC_BVH_H__
#error You should not include this file directly !
#endif

// This is useless but it tells QtCreator where to find symbols :P
#include "bvh.h"
#include "math.h"
#include <algorithm>

#define BVH_MAX_SET_SIZE		8
#define BVH_MAX_DEPTH			48

namespace TA3D
{
	template<typename T, class TKit>
		inline void BVH<T, TKit>::build(MemoryPool<BVH<T, TKit> > *pool, const typename std::vector<T>::iterator &begin, const typename std::vector<T>::iterator &end, const unsigned int l)
	{
		this->pool = pool;
		TKit::getTopBottom(begin, end, top, bottom);
		if ((end - begin) <= BVH_MAX_SET_SIZE || l >= BVH_MAX_DEPTH)
		{
			elements_begin = begin;
			elements_end = end;
			return;
		}
		const unsigned int N = TKit::getPrincipalDirection(top - bottom);

		const Vec M = 0.5f * (top + bottom);
		const typename std::vector<T>::iterator mid = std::partition(begin, end, typename TKit::Predicate(M, N));
		lChild = pool->alloc();
		lChild->build(pool, mid, end, l + 1U);
		rChild = pool->alloc(),
		rChild->build(pool, begin, mid, l + 1U);
	}

	template<typename T, class TKit>
		inline BVH<T, TKit>::~BVH()
	{
		if (lChild)
			pool->release(lChild);
		if (rChild)
			pool->release(rChild);
	}

	template<typename T, class TKit>
		inline void BVH<T, TKit>::boxCollisionQuery(std::deque<T> &result, const Vec &center, const float maxDist) const
	{
		const Vector3D L(maxDist, maxDist, maxDist);
		if ((Math::Max(top, center + L) - Math::Min(bottom, center - L) - (top - bottom + L + L)).max() > 0.0f)
			return;

		if (rChild == NULL && lChild == NULL)
		{
			for(typename std::vector<T>::const_iterator i = elements_begin ; i != elements_end ; ++i)
				if ((TKit::pos(*i) - center).norm() <= maxDist + TKit::radius(*i))
					result.push_back(*i);
			return;
		}

		rChild->boxCollisionQuery(result, center, maxDist);
		lChild->boxCollisionQuery(result, center, maxDist);
	}
}

#undef BVH_MAX_SET_SIZE
#undef BVH_MAX_DEPTH
