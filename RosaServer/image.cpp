#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdexcept>
#include <cstdlib>

#define ERR_COULD_NOT_LOAD "Could not load image"
#define ERR_COULD_NOT_SAVE "Could not save image"
#define ERR_NO_DATA_LOADED "No image data loaded"
#define ERR_OUT_OF_RANGE "Coordinates out of range"
#define ERR_CHANNELS "Too few channels"

Image::Image()
{
}

Image::~Image()
{
	_free();
}

void Image::_free()
{
	if (data)
	{
		stbi_image_free(data);
		data = nullptr;
		width = 0;
		height = 0;
		numChannels = 0;
	}
}

void Image::loadFromFile(const char* fileName)
{
	_free();

	data = stbi_load(fileName, &width, &height, &numChannels, 0);
	if (!data || !numChannels)
	{
		data = nullptr;
		width = 0;
		height = 0;
		numChannels = 0;
		throw std::runtime_error(ERR_COULD_NOT_LOAD);
	}
}

void Image::loadBlank(unsigned int _width, unsigned int _height, unsigned int _numChannels)
{
	if (_numChannels < 1 || _numChannels > 4)
	{
		throw std::invalid_argument("Invalid channel count");
	}

	if (_width == 0)
	{
		throw std::invalid_argument("width cannot be 0");
	}

	if (_height == 0)
	{
		throw std::invalid_argument("height cannot be 0");
	}

	_free();

	data = (uint8_t*)calloc(_width * _height * _numChannels, sizeof(uint8_t));
	if (!data)
	{
		width = 0;
		height = 0;
		numChannels = 0;
		throw std::runtime_error(ERR_COULD_NOT_LOAD);
	}

	width = _width;
	height = _height;
	numChannels = _numChannels;
}

std::tuple<int, int, int> Image::getRGB(unsigned int x, unsigned int y)
{
	if (!data)
	{
		throw std::runtime_error(ERR_NO_DATA_LOADED);
	}

	if (x >= width || y >= height)
	{
		throw std::invalid_argument(ERR_OUT_OF_RANGE);
	}

	size_t index = ((y * width) + x) * numChannels;
	return std::make_tuple(
		data[index],
		numChannels > 1 ? data[index + 1] : 255,
		numChannels > 2 ? data[index + 2] : 255
	);
}

std::tuple<int, int, int, int> Image::getRGBA(unsigned int x, unsigned int y)
{
	if (!data)
	{
		throw std::runtime_error(ERR_NO_DATA_LOADED);
	}

	if (x >= width || y >= height)
	{
		throw std::invalid_argument(ERR_OUT_OF_RANGE);
	}

	size_t index = ((y * width) + x) * numChannels;
	return std::make_tuple(
		data[index],
		numChannels > 1 ? data[index + 1] : 255,
		numChannels > 2 ? data[index + 2] : 255,
		numChannels > 3 ? data[index + 3] : 255
	);
}

void Image::setRGB(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b)
{
	if (!data)
	{
		throw std::runtime_error(ERR_NO_DATA_LOADED);
	}

	if (x >= width || y >= height)
	{
		throw std::invalid_argument(ERR_OUT_OF_RANGE);
	}

	if (numChannels < 3)
	{
		throw std::invalid_argument(ERR_CHANNELS);
	}

	size_t index = ((y * width) + x) * numChannels;
	data[index] = r;
	data[index + 1] = g;
	data[index + 2] = b;
	if (numChannels > 3)
	{
		data[index + 3] = 255;
	}
}

void Image::setRGBA(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	if (!data)
	{
		throw std::runtime_error(ERR_NO_DATA_LOADED);
	}

	if (x >= width || y >= height)
	{
		throw std::invalid_argument(ERR_OUT_OF_RANGE);
	}

	if (numChannels < 4)
	{
		throw std::invalid_argument(ERR_CHANNELS);
	}

	size_t index = ((y * width) + x) * numChannels;
	data[index] = r;
	data[index + 1] = g;
	data[index + 2] = b;
	data[index + 3] = a;
}

std::string Image::getPNG()
{
	if (!data)
	{
		throw std::runtime_error(ERR_NO_DATA_LOADED);
	}

	int length;
	unsigned char* pngBuffer = stbi_write_png_to_mem(reinterpret_cast<const unsigned char*>(data), width * numChannels, width, height, numChannels, &length);
	if (!pngBuffer)
	{
		throw std::runtime_error(ERR_COULD_NOT_SAVE);
	}

	std::string pngString(reinterpret_cast<char*>(pngBuffer), length);
	STBIW_FREE(pngBuffer);
	return pngString;
}