#pragma once

#include <iostream>
#include <cstdint>
#include <memory.h>
#include <cassert>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

struct Image
{
#ifndef NDEBUG
	std::string img_name;
#endif
	uint16_t width, height, nchannels;
	uint8_t* data;
};

Image ReadImage(const char* file_name)
{
	Image image;
	int width, height, nchannels;
	image.data = stbi_load(file_name, (int*)&width, (int*)&height, (int*)&nchannels, 1);
	if (!image.data)
	{
		std::cout << "Couldn't read: " << file_name << "\n";
	}
	image.width = width;
	image.height = height;
	image.nchannels = nchannels;
	return image;
}

void WriteImage(const Image& image, const char* file_name)
{
	stbi_write_png(file_name, image.width, image.height, 1, image.data, image.width);
}
