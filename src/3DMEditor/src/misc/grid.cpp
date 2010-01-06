#include <cstring>
#include <QtCore>
#include "grid.h"
#include "math.h"

void gaussianFilter(Grid<float> &grid, float sigma)
{
	int s = 1 + int(3.0f * sigma);

	float *backup = new float[qMax(grid.getWidth(), grid.getHeight())];
	float *kernel = new float[2 * s + 1];
	for(int i = -s ; i <= s ; ++i)
		kernel[s + i] = float(exp(-i * i / (2.0 * sigma * sigma)) / (sqrt(2.0 * M_PI) * sigma * sigma));

	// X pass
	for(int y = 0 ; y < grid.getHeight() ; ++y)
	{
		for(int x = 0 ; x < grid.getWidth() ; ++x)
			backup[x] = grid(x, y);
		for(int x = 0 ; x < grid.getWidth() ; ++x)
		{
			float acc((2 * s - (qMin(s, grid.getWidth() - 1 - x) - qMax(-s, -x))) * 255.0f);
			for(int i = qMax(-s, -x) ; i <= qMin(s, grid.getWidth() - 1 - x) ; ++i)
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
			float acc((2 * s - (qMin(s, grid.getHeight() - 1 - x) - qMax(-s, -y))) * 255.0f);
			for(int i = qMax(-s, -y) ; i <= qMin(s, grid.getHeight() - 1 - y) ; ++i)
				acc += kernel[i + s] * backup[y + i];
			grid(x,y) = acc;
		}
	}

	delete[] kernel;
	delete[] backup;
}
