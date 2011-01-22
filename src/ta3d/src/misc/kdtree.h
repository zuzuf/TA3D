#ifndef __TA3D_MISC_KDTREE_H__
#define __TA3D_MISC_KDTREE_H__

#include "vector.h"
#include <vector>
#include <deque>

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
	 * TKit::Comparator - a TKit::Vec comparator type to be used with std::sort
	 *                    and containing a "TKit::Vec N" normal vector (the direction of the projection)
	 */
	template<typename T, typename TKit>
			class KDTree
	{
	public:
		typedef typename TKit::Vec	Vec;
	public:
		KDTree(std::vector<T> &elts);
		virtual ~KDTree();

		void maxDistanceQuery(std::deque<T> &result, const Vec &center, float maxDist);

	private:
		std::vector<T> elements;
		Vec P;
		unsigned int N;
		KDTree<T, TKit>	*lChild;
		KDTree<T, TKit>	*rChild;
	};
}

#include "kdtree.hxx"

#endif
