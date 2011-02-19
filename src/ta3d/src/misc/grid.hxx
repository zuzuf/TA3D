#ifndef __TA3D_GRID_HXX__
#define __TA3D_GRID_HXX__

#include "grid.h"

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
		for(typename Container::iterator i = data.begin() ; i != data.end() ; ++i)
			*i = v;
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
}

#endif
