#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdexcept>

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
		throw std::runtime_error("Could not load image");
	}
}

std::tuple<int, int, int> Image::getRGB(unsigned int x, unsigned int y)
{
	if (!data)
	{
		throw std::runtime_error("No image data loaded");
	}

	if (x >= width || y >= height)
	{
		throw std::runtime_error("Coordinates out of range");
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
		throw std::runtime_error("No image data loaded");
	}

	if (x >= width || y >= height)
	{
		throw std::runtime_error("Coordinates out of range");
	}

	size_t index = ((y * width) + x) * numChannels;
	return std::make_tuple(
		data[index],
		numChannels > 1 ? data[index + 1] : 255,
		numChannels > 2 ? data[index + 2] : 255,
		numChannels > 3 ? data[index + 3] : 255
	);
}

std::string Image::getPNG()
{
	if (!data)
	{
		throw std::runtime_error("No image data loaded");
	}

	int length;
	unsigned char* pngBuffer = stbi_write_png_to_mem(reinterpret_cast<const unsigned char*>(data), width * numChannels, width, height, numChannels, &length);
	if (!pngBuffer)
	{
		throw std::runtime_error("Could not get PNG");
	}

	std::string pngString(reinterpret_cast<char*>(pngBuffer), length);
	STBIW_FREE(pngBuffer);
	return pngString;
}