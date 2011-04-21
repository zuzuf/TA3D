#ifndef __TA3D_MISC_KDTREE_H__
#define __TA3D_MISC_KDTREE_H__

#include "vector.h"
#include <vector>
#include <deque>
#include "mempool.h"

namespace TA3D
{
	/* This class implements a Kd Tree structure
	 *
	 * The tree is built at object creation from a std::vector<T>.
	 * In order to use this sturcture you must define a TKit class which
	 * supports the following operations:
	 * TKit::Vec TKit::pos(const T &elt);
	 * void TKit::getTopBottom(const std::vector<T> &elts, TKit::Vec &top, TKit::Vec &bottom);
	 * TKit::Vec TKit::getPrincipalDirection(const TKit::Vec &v);
	 *
	 * The TKit class must also provide the following types:
	 * TKit::Vec - a vector type
	 * TKit::Predicate - a TKit::Vec predicate type to be used with std::partition
	 *                    and containing the direction of the projection and a position (the split point)
	 */
	template<typename T, typename TKit>
			class KDTree
	{
	public:
		typedef typename TKit::Vec	Vec;
	public:
		inline KDTree() : lChild(NULL), rChild(NULL)	{}

		inline void build(MemoryPool< KDTree<T, TKit> > *pool, const typename std::vector<T>::iterator &begin, const typename std::vector<T>::iterator &end, const unsigned int l = 0U);
		inline ~KDTree();

		inline void maxDistanceQuery(std::deque<T> &result, const Vec &center, const float maxDist) const;

		static inline KDTree *create(MemoryPool< KDTree<T, TKit> > *pool, const typename std::vector<T>::iterator &begin, const typename std::vector<T>::iterator &end)
		{
			KDTree *tree = pool->alloc();
			tree->build(pool, begin, end);
			return tree;
		}

	private:
		typename std::vector<T>::const_iterator elements_begin;
		typename std::vector<T>::const_iterator elements_end;
		float P;
		unsigned int N;
		KDTree<T, TKit>	*lChild;
		KDTree<T, TKit>	*rChild;
		MemoryPool< KDTree<T, TKit> > *pool;
	};
}

#include "kdtree.hxx"

#endif
