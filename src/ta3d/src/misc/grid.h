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
		Grid(unsigned int w, unsigned int h);
	private:
		Grid(const Grid&)	{}
		Grid &operator=(const Grid&)	{}
	public:
		~Grid();
		void resize(unsigned int w, unsigned int h);

		inline const_reference operator()(unsigned int x, unsigned int y) const;
		inline reference operator()(unsigned int x, unsigned int y);

		inline int getWidth() const {	return w;	}
		inline int getHeight() const {	return h;	}
		inline void add(const Grid<T> &grid, int x, int y);
		inline void sub(const Grid<T> &grid, int x, int y);
		inline void clear(const T &v = T(0));

		inline void *getData()	{	return &(data.front());	}
		inline unsigned int getSize() const	{	return (unsigned int)(data.size() * sizeof(T));	}

		inline void hline(const int x0, const int x1, const int y, const T& col);
		inline void circlefill(const int x, const int y, const int r, const T& col);

	private:
		unsigned int w;
		unsigned int h;
		Container data;
	};

	void gaussianFilter(Grid<float> &grid, float sigma);
}

#include "grid.hxx"

#endif
