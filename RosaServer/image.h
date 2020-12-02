#pragma once

#include <cstdint>
#include <tuple>
#include <string>

class Image
{
	uint8_t* data = nullptr;
	int width = 0;
	int height = 0;
	int numChannels = 0;
public:
	Image();
	~Image();
	int getWidth() const
	{
		return width;
	}
	int getHeight() const
	{
		return height;
	}
	int getNumChannels() const
	{
		return numChannels;
	}
	void _free();
	void loadFromFile(const char* fileName);
	std::tuple<int, int, int> getRGB(unsigned int x, unsigned int y);
	std::tuple<int, int, int, int> getRGBA(unsigned int x, unsigned int y);
	std::string getPNG();
};