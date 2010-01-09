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
		KDTree<T, TKit>::KDTree(std::vector<T> &elts) : lChild(NULL), rChild(NULL)
	{
		if (elts.size() <= KDTREE_MAX_SET_SIZE)
		{
			elements.insert(elements.end(), elts.begin(), elts.end());
			return;
		}
		Vec top, bottom;
		TKit::getTopBottom(elts, top, bottom);
		N = TKit::getPrincipalDirection(top - bottom);
		typename TKit::Comparator cmp;
		cmp.N = N;

		std::sort(elts.begin(), elts.end(), cmp);
		int mid = (elts.size() - 1) / 2;
		P = TKit::pos(elts[mid]);
		std::vector<T> lower(elts.begin(), elts.begin() + mid - 1);
		std::vector<T> upper(elts.begin() + mid, elts.end());

		lChild = new KDTree<T, TKit>(upper);
		rChild = new KDTree<T, TKit>(lower);
	}

	template<typename T, class TKit>
		KDTree<T, TKit>::~KDTree()
	{
		if (lChild)
			delete lChild;
		if (rChild)
			delete rChild;
	}

	template<typename T, class TKit>
		void KDTree<T, TKit>::maxDistanceQuery(std::deque<T> &result, const Vec &center, float maxDist)
	{
		if (rChild == NULL && lChild == NULL)
		{
			float dist2 = maxDist * maxDist;
			for(typename std::vector<T>::iterator i = elements.begin() ; i != elements.end() ; ++i)
				if ((TKit::pos(*i) - center).Sq() <= dist2)
					result.push_back(*i);
			return;
		}

		float proj = (P - center) % N;

		if (proj >= 0.0f || -proj < maxDist)
			lChild->maxDistanceQuery(result, center, maxDist);
		if (proj <= 0.0f || proj < maxDist)
			rChild->maxDistanceQuery(result, center, maxDist);
	}
}
