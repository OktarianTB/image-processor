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

	Image& crop(uint16_t cx, uint16_t cy, uint16_t ch, uint16_t cw);
	Image& resize(int new_width, int new_height);
	Image& scale(double ratio);

	Image& grayscale_avg();
	Image& grayscale_lum();

	Image& colorMask(float r, float g, float b);

	Image& diffMap(Image& img);
	Image& diffMap_scale(Image& img, uint8_t scale = 0);

	Image& overlay(const Image& source, int x, int y);

	Image& pixelize(int strength = 2);

	Image& gaussian_blur(int strength = 2);
	Image& edge_detection();
	Image& sharpen();

	Image& std_convolve_clamp_to_0(uint8_t channel, uint32_t kernel_width, 
		uint32_t kernel_height, double kernel[], uint32_t cr, uint32_t cc);
};


