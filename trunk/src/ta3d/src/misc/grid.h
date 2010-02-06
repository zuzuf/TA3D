#ifndef __TA3D_GRID_H__
#define __TA3D_GRID_H__

#include <logs/logs.h>

namespace TA3D
{
	template<class T>
			class Grid
	{
	public:
		Grid();
		Grid(int w, int h);
		~Grid();
		void resize(int w, int h);

		inline const T& operator()(int x, int y) const;
		inline T& operator()(int x, int y);

		inline int getWidth() const {	return w;	}
		inline int getHeight() const {	return h;	}
		inline void add(const Grid<T> &grid, int x, int y);
		inline void sub(const Grid<T> &grid, int x, int y);
		inline void clear();

	private:
		int w;
		int h;
		T *data;
	};

	void gaussianFilter(Grid<float> &grid, float sigma);
}

#include "grid.hxx"

#endif
