#ifndef __TA3D_MISC_KDTREE_H__
#error You should not include this file directly !
#endif

// This is useless but it tells QtCreator where to find symbols :P
#include "kdtree.h"
#include <algorithm>

#define KDTREE_MAX_SET_SIZE		8

namespace TA3D
{
	template<typename T, class TKit>
		inline KDTree<T, TKit>::KDTree(const typename std::vector<T>::iterator &begin, const typename std::vector<T>::iterator &end) : lChild(NULL), rChild(NULL)
	{
		if ((end - begin) <= KDTREE_MAX_SET_SIZE)
		{
			elements_begin = begin;
			elements_end = end;
			return;
		}
		Vec top, bottom;
		TKit::getTopBottom(begin, end, top, bottom);
		N = TKit::getPrincipalDirection(top - bottom);

		top = 0.5f * (top + bottom);
		const typename std::vector<T>::iterator mid = std::partition(begin, end, typename TKit::Predicate(top, N));
		P = top[N];
		lChild = new KDTree<T, TKit>(mid, end);
		rChild = new KDTree<T, TKit>(begin, mid);
	}

	template<typename T, class TKit>
		inline KDTree<T, TKit>::~KDTree()
	{
		if (lChild)
			delete lChild;
		if (rChild)
			delete rChild;
	}

	template<typename T, class TKit>
		inline void KDTree<T, TKit>::maxDistanceQuery(std::deque<T> &result, const Vec &center, const float maxDist) const
	{
		if (rChild == NULL && lChild == NULL)
		{
			const float dist2 = maxDist * maxDist;
			for(typename std::vector<T>::const_iterator i = elements_begin ; i != elements_end ; ++i)
				if ((TKit::pos(*i) - center).sq() <= dist2)
					result.push_back(*i);
			return;
		}

		const float proj = P - center[N];

		if (proj >= 0.0f)
		{
			rChild->maxDistanceQuery(result, center, maxDist);
			if (proj < maxDist)
				lChild->maxDistanceQuery(result, center, maxDist);
		}
		else
		{
			lChild->maxDistanceQuery(result, center, maxDist);
			if (-proj < maxDist)
				rChild->maxDistanceQuery(result, center, maxDist);
		}
	}
}
