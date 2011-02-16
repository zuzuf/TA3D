#ifndef __TA3D_GRID_H__
#define __TA3D_GRID_H__

#include <logs/logs.h>
#include <vector>

namespace TA3D
{
	template<class T>	struct GridDataType {	typedef T type;	};
	template<>	struct GridDataType<bool> {	typedef unsigned char type;	};

	template<class T>
			class Grid
	{
	public:
		typedef typename GridDataType<T>::type Type;
		typedef typename std::vector<Type> Container;
		typedef Type& reference;
		typedef const Type& const_reference;
	public:
		Grid();
		Grid(int w, int h);
	private:
		Grid(const Grid&)	{}
		Grid &operator=(const Grid&)	{}
	public:
		~Grid();
		void resize(int w, int h);

		inline const_reference operator()(int x, int y) const;
		inline reference operator()(int x, int y);

		inline int getWidth() const {	return w;	}
		inline int getHeight() const {	return h;	}
		inline void add(const Grid<T> &grid, int x, int y);
		inline void sub(const Grid<T> &grid, int x, int y);
		inline void clear(const T &v = T(0));

	private:
		int w;
		int h;
		Container data;
	};

	void gaussianFilter(Grid<float> &grid, float sigma);
}

#include "grid.hxx"

#endif
