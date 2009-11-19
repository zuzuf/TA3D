#ifndef __TA3D_GRID_H__
#define __TA3D_GRID_H__

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

		const T& operator()(int x, int y) const;
		T& operator()(int x, int y);

		int getWidth() {	return w;	}
		int getHeight() {	return h;	}

	private:
		int w;
		int h;
		T *data;
	};

	void gaussianFilter(Grid<float> &grid, float sigma);
}

#include "grid.hxx"

#endif
