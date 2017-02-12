
#include <yuni/yuni.h>
#include <yuni/gfx/image.h>
#include <yuni/gfx/imagescaler.h>


using namespace Yuni;
using namespace Yuni::Gfx;

int main(void)
{
	// Instantiate the Scale2X algorithm
	ImageScaler scaler;
	// Create an image with values in 0..255 (grey-levels equivalent)
	uint8** data = new uint8*[2];
	data[0] = new uint8[2];
	data[0][0] = 255;
	data[0][1] = 0;
	data[1] = new uint8[2];
	data[1][0] = 255;
	data[1][1] = 0;
	// Call the Scale2X scaling algorithm
	uint8** scaledData = scaler.Scale2X(data, 2, 2);
	scaledData = scaler.Eagle(data, 2, 2);

	delete[] scaledData[0];
	delete[] scaledData[1];
	delete[] scaledData;

	Image<uint8> image(2, 2, data);
	// Call the naive (nearest neighbour) scaling algorithm
	Image<uint8> scaledImage = scaler.NearestNeighbour(image, image.width(), image.height());
	return 0;
}
