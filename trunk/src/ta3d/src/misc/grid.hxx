#ifndef __TA3D_GRID_HXX__
#define __TA3D_GRID_HXX__

#include "grid.h"
#include <algorithm>

namespace TA3D
{
	template<class T>
		Grid<T>::Grid() : w(0U), h(0U), data()
	{
		resize(1U, 1U);
	}

	template<class T>
		Grid<T>::Grid(unsigned int w, unsigned int h) : w(0U), h(0U), data()
	{
		resize(w, h);
	}

	template<class T>
		void Grid<T>::resize(unsigned int w, unsigned int h)
	{
		this->w = w;
		this->h = h;
		data.resize(w * h);
	}

	template<class T>
		inline void Grid<T>::clear(const T &v)
	{
		std::fill(data.begin(), data.end(), v);
	}

	template<class T>
		Grid<T>::~Grid()
	{
		data.clear();
	}

	template<class T>
		inline typename Grid<T>::const_reference Grid<T>::operator()(const unsigned int x, const unsigned int y) const
	{
		LOG_ASSERT(x < w && y < h);
		return data[x + y * w];
	}

	template<class T>
		inline typename Grid<T>::reference Grid<T>::operator()(const unsigned int x, const unsigned int y)
	{
		LOG_ASSERT(x < w && y < h);
		return data[x + y * w];
	}

	template<class T>
		inline void Grid<T>::add(const Grid<T> &grid, const int x, const int y)
	{
		const unsigned int j0 = std::max(0, -y);
		const unsigned int j1 = std::min<unsigned int>(grid.getHeight(), h - y);
		const unsigned int i0 = std::max(0, -x);
		const unsigned int i1 = std::min<unsigned int>(grid.getWidth(), w - x);
		for(unsigned int j = j0 ; j < j1 ; ++j)
			for(unsigned int i = i0 ; i < i1 ; ++i)
				(*this)(i + x, j + y) += grid(i, j);
	}

	template<class T>
		inline void Grid<T>::sub(const Grid<T> &grid, const int x, const int y)
	{
		const unsigned int j0 = std::max(0, -y);
		const unsigned int j1 = std::min<unsigned int>(grid.getHeight(), h - y);
		const unsigned int i0 = std::max(0, -x);
		const unsigned int i1 = std::min<unsigned int>(grid.getWidth(), w - x);
		for(unsigned int j = j0 ; j < j1 ; ++j)
			for(unsigned int i = i0 ; i < i1 ; ++i)
				(*this)(i + x, j + y) -= grid(i, j);
	}

	template<class T>
		inline void Grid<T>::hline(const int x0, const int x1, const int y, const T &col)
		{
			std::fill(data.begin() + (y * w + x0), data.begin() + (y * w + x1 + 1), col);
		}

	template<class T>
		inline void Grid<T>::circlefill(const int x, const int y, const int r, const T& col)
	{
		const int r2 = r * r;
		const int my = std::max<int>(-r, -y);
		const int My = std::min<int>(r, h - 1 - y);
		for (int sy = my ; sy <= My ; ++sy)
		{
			const int dx = int(std::sqrt(float(r2 - sy * sy)));
			const int ax = std::max<int>(x - dx, 0);
			const int bx = std::min<int>(x + dx, w - 1);
			hline(ax, bx, y + sy, col);
		}
	}
}

#endif
