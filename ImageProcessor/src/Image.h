#pragma once
#include <cstdint>
#include <inttypes.h>
#include <cmath>
#include <complex>
#include <vector>
#define _USE_MATH_DEFINES

enum ImageType
{
	PNG, JPG, BMP, TGA
};


struct Image
{
	uint8_t* data = nullptr;
	size_t size = 0;
	int width;
	int height;
	int channels;
	bool valid = false;

	Image(const char* filename);
	Image(int w, int h, int channels);
	Image(const Image& img);
	~Image();

	bool read(const char* filename);
	bool write(const char* filename);
	inline bool is_valid() { return valid; }

	ImageType getFileType(const char* filename);

	Image& flipX();
	Image& flipY();

	Image& crop(uint16_t start_x, uint16_t start_y, uint16_t new_height, uint16_t new_width);
	Image& resize(int new_width, int new_height);
	Image& scale(double ratio);

	Image& grayscale_avg();
	Image& grayscale_lum();

	Image& color_mask(float r, float g, float b);

	Image& pixelize(int strength = 2);

	Image& gaussian_blur(int strength = 2);
	Image& edge_detection(double cutoff = 115);
	Image& sharpen();
};


