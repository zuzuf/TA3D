#include <stdafx.h>
#include <cstring>
#include "grid.h"
#include "math.h"

namespace TA3D
{
	void gaussianFilter(Grid<float> &grid, float sigma)
	{
		int s = 1 + int(3.0f * sigma);

		float *backup = new float[Math::Max(grid.getWidth(), grid.getHeight())];
		float *kernel = new float[2 * s + 1];
		for(int i = -s ; i <= s ; ++i)
			kernel[s + i] = float(exp(-i * i / (2.0 * sigma * sigma)) / (sqrt(2.0 * M_PI) * sigma * sigma));

		// X pass
		for(int y = 0 ; y < grid.getHeight() ; ++y)
		{
			memcpy(backup, &(grid(y, 0)), grid.getWidth() * sizeof(float));
			for(int x = 0 ; x < grid.getWidth() ; ++x)
			{
				float acc((2 * s - (Math::Min(s, grid.getWidth() - 1 - x) - Math::Max(-s, -x))) * 255.0f);
				for(int i = Math::Max(-s, -x) ; i <= Math::Min(s, grid.getWidth() - 1 - x) ; ++i)
					acc += kernel[i + s] * backup[x + i];
				grid(x,y) = acc;
			}
		}

		// Y pass
		for(int x = 0 ; x < grid.getWidth() ; ++x)
		{
			for(int y = 0 ; y < grid.getHeight() ; ++y)
				backup[y] = grid(x, y);
			for(int y = 0 ; y < grid.getHeight() ; ++y)
			{
				float acc((2 * s - (Math::Min(s, grid.getHeight() - 1 - x) - Math::Max(-s, -y))) * 255.0f);
				for(int i = Math::Max(-s, -y) ; i <= Math::Min(s, grid.getHeight() - 1 - y) ; ++i)
					acc += kernel[i + s] * backup[y + i];
				grid(x,y) = acc;
			}
		}

		delete[] kernel;
		delete[] backup;
	}
}
