#include <stdafx.h>
#include <cstring>
#include "grid.h"
#include "math.h"

#ifdef _OPENMP
#include <omp.h>
#endif

namespace TA3D
{
	void gaussianFilter(Grid<float> &grid, float sigma)
	{
		const int s = 1 + int(3.0f * sigma);

#ifdef _OPENMP
		const int nb_threads = omp_get_max_threads();
		float **backups = new float*[nb_threads];
		memset(backups, 0, sizeof(float*) * nb_threads);
		const int maxSize = Math::Max(grid.getWidth(), grid.getHeight());
#else
		float *backup = new float[Math::Max(grid.getWidth(), grid.getHeight())];
#endif
		float *kernel = new float[2 * s + 1];
		for(int i = -s ; i <= s ; ++i)
			kernel[s + i] = float(exp(-i * i / (2.0 * sigma * sigma)) / (sqrt(2.0 * M_PI) * sigma * sigma));

		// X pass
#pragma omp parallel for
		for(int y = 0 ; y < grid.getHeight() ; ++y)
		{
#ifdef _OPENMP
			const int id = omp_get_thread_num();
			if (backups[id] == NULL)
				backups[id] = new float[maxSize];
			float *backup = backups[id];
#endif
			for(int x = 0 ; x < grid.getWidth() ; ++x)
				backup[x] = grid(x, y);
			for(int x = 0 ; x < grid.getWidth() ; ++x)
			{
				float acc(static_cast<float>((2 * s - (Math::Min(s, grid.getWidth() - 1 - x) - Math::Max(-s, -x))) * 255));
				for(int i = Math::Max(-s, -x) ; i <= Math::Min(s, grid.getWidth() - 1 - x) ; ++i)
					acc += kernel[i + s] * backup[x + i];
				grid(x,y) = acc;
			}
		}

		// Y pass
#pragma omp parallel for
		for(int x = 0 ; x < grid.getWidth() ; ++x)
		{
#ifdef _OPENMP
			const int id = omp_get_thread_num();
			if (backups[id] == NULL)
				backups[id] = new float[maxSize];
			float *backup = backups[id];
#endif
			for(int y = 0 ; y < grid.getHeight() ; ++y)
				backup[y] = grid(x, y);
			for(int y = 0 ; y < grid.getHeight() ; ++y)
			{
				float acc(static_cast<float>((2 * s - (Math::Min(s, grid.getHeight() - 1 - x) - Math::Max(-s, -y))) * 255));
				for(int i = Math::Max(-s, -y) ; i <= Math::Min(s, grid.getHeight() - 1 - y) ; ++i)
					acc += kernel[i + s] * backup[y + i];
				grid(x,y) = acc;
			}
		}

#ifdef _OPENMP
		for(int i = 0 ; i < nb_threads ; ++i)
			if (backups[i])
				delete[] backups[i];
		delete[] backups;
#else
		delete[] backup;
#endif
		delete[] kernel;
	}
}
