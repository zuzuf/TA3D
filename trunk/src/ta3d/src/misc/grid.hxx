#ifndef __TA3D_GRID_HXX__
#define __TA3D_GRID_HXX__

#include "grid.h"

namespace TA3D
{
	template<class T>
		Grid<T>::Grid() : w(0), h(0), data(NULL)
	{
		resize(1, 1);
	}

	template<class T>
		Grid<T>::Grid(int w, int h) : w(0), h(0), data(NULL)
	{
		resize(w, h);
	}

	template<class T>
		void Grid<T>::resize(int w, int h)
	{
		if (data)
			delete[] data;
		this->w = w;
		this->h = h;
		data = new T[w * h];
	}

	template<class T>
		Grid<T>::~Grid()
	{
		if (data)
			delete[] data;
	}

	template<class T>
		const T& Grid<T>::operator()(int x, int y) const
	{
		return data[x + y * w];
	}

	template<class T>
		T& Grid<T>::operator()(int x, int y)
	{
		return data[x + y * w];
	}
}

#endif
