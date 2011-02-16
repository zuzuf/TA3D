#ifndef __TA3D_GRID_HXX__
#define __TA3D_GRID_HXX__

#include "grid.h"

namespace TA3D
{
	template<class T>
		Grid<T>::Grid() : w(0), h(0), data()
	{
		resize(1, 1);
	}

	template<class T>
		Grid<T>::Grid(int w, int h) : w(0), h(0), data()
	{
		resize(w, h);
	}

	template<class T>
		void Grid<T>::resize(int w, int h)
	{
		this->w = w;
		this->h = h;
		data.resize(w * h);
	}

	template<class T>
		inline void Grid<T>::clear(const T &v)
	{
		for(typename Container::iterator i = data.begin() ; i != data.end() ; ++i)
			*i = v;
	}

	template<class T>
		Grid<T>::~Grid()
	{
		data.clear();
	}

	template<class T>
		inline typename Grid<T>::const_reference Grid<T>::operator()(int x, int y) const
	{
		LOG_ASSERT(x >= 0 && x < w && y >= 0 && y < h);
		return data[x + y * w];
	}

	template<class T>
		inline typename Grid<T>::reference Grid<T>::operator()(int x, int y)
	{
		LOG_ASSERT(x >= 0 && x < w && y >= 0 && y < h);
		return data[x + y * w];
	}

	template<class T>
		inline void Grid<T>::add(const Grid<T> &grid, int x, int y)
	{
		const int j0 = std::max(0, -y);
		const int j1 = std::min(grid.getHeight(), h - y);
		const int i0 = std::max(0, -x);
		const int i1 = std::min(grid.getWidth(), w - x);
		for(int j = j0 ; j < j1 ; ++j)
			for(int i = i0 ; i < i1 ; ++i)
				(*this)(i + x, j + y) += grid(i, j);
	}

	template<class T>
		inline void Grid<T>::sub(const Grid<T> &grid, int x, int y)
	{
		const int j0 = std::max(0, -y);
		const int j1 = std::min(grid.getHeight(), h - y);
		const int i0 = std::max(0, -x);
		const int i1 = std::min(grid.getWidth(), w - x);
		for(int j = j0 ; j < j1 ; ++j)
			for(int i = i0 ; i < i1 ; ++i)
				(*this)(i + x, j + y) -= grid(i, j);
	}
}

#endif
