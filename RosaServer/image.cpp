#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdexcept>

Image::Image()
{
}

Image::~Image()
{
	free();
}

void Image::free()
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
	free();

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

static constexpr char channelError[] = "Image data has too few channels";

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