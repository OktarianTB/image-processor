#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Image.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#define BYTE_BOUND(x) x < 0 ? 0 : (x > 255 ? 255 : x)
#define MAP_BLACK_WHITE(x, cutoff) x <= cutoff ? 0 : 255;

using namespace std;


Image::Image(const char* filename)
{
	if (read(filename))
	{
		printf("Successfully read %s\n", filename);
		cout << "Image has width = " << width << " and height = " << height << endl;
		size = width * height * channels;
		valid = true;
	}
	else
		printf("Failed to read %s :(\n", filename);
}

Image::Image(int w, int h, int channels) : width(w), height(h), channels(channels)
{
	size = width * height * channels;
	data = new uint8_t[size];
}

Image::Image(const Image& img) : Image(img.width, img.height, img.channels)
{
	memcpy(data, img.data, img.size);
}

Image::~Image()
{
	stbi_image_free(data);
}

bool Image::read(const char* filename)
{
	data = stbi_load(filename, &width, &height, &channels, 0);
	return data != nullptr;
}

bool Image::write(const char* filename)
{
	ImageType type = getFileType(filename);
	int success = 0;

	switch (type)
	{
	case PNG:
		success = stbi_write_png(filename, width, height, channels, data, width * channels);
		break;
	case JPG:
		success = stbi_write_jpg(filename, width, height, channels, data, 100);
		break;
	case BMP:
		success = stbi_write_bmp(filename, width, height, channels, data);
		break;
	case TGA:
		success = stbi_write_tga(filename, width, height, channels, data);
		break;
	}

	return success != 0;
}

ImageType Image::getFileType(const char* filename)
{
	const char* ext = strrchr(filename, '.');
	if (ext)
	{
		if (strcmp(ext, ".png") == 0)
			return PNG;
		if (strcmp(ext, ".jpg") == 0)
			return JPG;
		if (strcmp(ext, ".bmp") == 0)
			return BMP;
		if (strcmp(ext, ".tga") == 0)
			return TGA;
	}

	return PNG;
}

Image& Image::flipX()
{
	uint8_t temp[4];
	uint8_t* px1;
	uint8_t* px2;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width/2; ++x)
		{
			px1 = &data[(x + y * width) * channels];
			px2 = &data[((width - 1 - x) + y * width) * channels];
			memcpy(temp, px1, channels);
			memcpy(px1, px2, channels);
			memcpy(px2, temp, channels);
		}
	}

	return *this;
}

Image& Image::flipY()
{
	uint8_t temp[4];
	uint8_t* px1;
	uint8_t* px2;
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height / 2; ++y)
		{
			px1 = &data[(x + y * width) * channels];
			px2 = &data[(x + (height - 1 - y) * width) * channels];
			memcpy(temp, px1, channels);
			memcpy(px1, px2, channels);
			memcpy(px2, temp, channels);
		}
	}

	return *this;
}

Image& Image::crop(uint16_t start_x, uint16_t start_y, uint16_t new_height, uint16_t new_width)
{
	size = new_width * new_height * channels;
	uint8_t* croppedImage = new uint8_t[size];
	memset(croppedImage, 0, size);

	for (uint16_t y = 0; y < new_height; ++y)
	{
		if (y + start_y >= height) break;

		for (uint16_t x = 0; x < new_width; ++x)
		{
			if (x + start_x >= width) break;

			memcpy(&croppedImage[(x + y * new_width) * channels], &data[(x + start_x + (start_y + y) * width) * channels], channels);
		}
	}

	width = new_width;
	height = new_height;

	delete[] data;
	data = croppedImage;
	croppedImage = nullptr;

	return *this;
}

Image& Image::resize(int new_width, int new_height)
{
	

	double x_ratio = width / (double)new_width;
	double y_ratio = height / (double)new_height;

	int new_size = new_width * new_height * channels;
	uint8_t* dst = new uint8_t[new_size];
	memset(dst, 0, new_size);

	for (int y = 0; y < new_height; ++y)
	{
		for (int x = 0; x < new_width; ++x)
		{
			int px = x * x_ratio;
			int py = y * y_ratio;

			int dstIndex = (y * new_width + x) * channels;
			int srcIndex = (py * width + px) * channels;

			for (int channel = 0; channel < channels; ++channel)
			{
				dst[dstIndex + channel] = data[srcIndex + channel];
			}
		}
	}

	delete[] data;
	data = dst;
	dst = nullptr;

	width = new_width;
	height = new_height;
	size = new_size;

	return *this;
}

Image& Image::scale(double ratio)
{
	int new_width = ratio * width;
	int new_height = ratio * height;

	return resize(new_width, new_height);
}

Image& Image::grayscale_avg()
{
	if (channels < 3)
	{
		printf("Image has less than 3 channels. Probably already grayscaled.");
	}
	else
	{
		for (int i = 0; i < size; i += channels)
		{
			int gray = (data[i] + data[i + 1] + data[i + 2]) / 3;
			memset(data + i, gray, 3);
		}
	}

	return *this;
}

Image& Image::grayscale_lum()
{
	if (channels < 3)
	{
		printf("Image has less than 3 channels. Probably already grayscaled.");
	}
	else
	{
		for (int i = 0; i < size; i += channels)
		{
			int gray = 0.2126 * data[i] + 0.7152 * data[i + 1] + 0.0722 * data[i + 2];
			memset(data + i, gray, 3);
		}
	}

	return *this;
}

Image& Image::color_mask(float r, float g, float b)
{
	if (channels < 3)
	{
		printf("Image has less than 3 channels, a color mask cannot be applied.");
	}
	else
	{
		for (int i = 0; i < size; i += channels)
		{
			data[i] *= r;
			data[i+1] *= g;
			data[i+2] *= b;
		}
	}

	return *this;
}

Image& Image::pixelize(int strength)
{
	int new_width = width - (width % strength);
	int new_height = height - (height % strength);
	int new_size = new_width * new_height * channels;

	uint8_t* dst = new uint8_t[new_size];
	memset(dst, 0, new_size);

	for (uint16_t y = 0; y < new_height; y += strength)
	{
		for (uint16_t x = 0; x < new_width; x += strength)
		{
			for (int channel = 0; channel < channels; ++channel)
			{
				double sum = 0;

				for (int i = y; i < y + strength; ++i)
				{
					for (int j = x; j < x + strength; ++j)
					{
						sum += data[(i * width + j) * channels + channel];
					}
				}
				
				uint8_t new_value = round(sum / (strength * strength));

				for (int i = y; i < y + strength; ++i)
				{
					for (int j = x; j < x + strength; ++j)
					{
						int index = (i * new_width + j) * channels + channel;
						dst[index] = BYTE_BOUND(new_value);
					}
				}
			}
		}
	}

	delete[] data;
	data = dst;
	dst = nullptr;

	width = new_width;
	height = new_height;
	size = new_size;

	return *this;
}

int get_border_values(int M, int x)
{
	if (x < 0)
	{
		return -x - 1;
	}
	if (x >= M)
	{
		return 2 * M - x - 1;
	}
	
	return x;
}

Image& Image::gaussian_blur(int strength)
{
	// Coefficients of a 1-dimensional gaussian kernel with sigma = 1
	std::vector<double> kernel;
	int kernel_length = 3;
	
	switch (strength)
	{
	case 1:
		kernel = { 0.27901,	0.44198, 0.27901 };
		break;
	case 2:
		kernel = { 0.06136,	0.24477, 0.38774, 0.24477, 0.06136 };
		kernel_length = 5;
		break;
	case 3:
		kernel = { 0.00598,	0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598 };
		kernel_length = 7;
		break;
	case 4:
		kernel = { 0.000229, 0.005977, 0.060598, 0.241732, 0.382928, 0.241732, 0.060598, 0.005977, 0.000229 };
		kernel_length = 9;
		break;
	default:
		kernel = { 0.27901,	0.44198, 0.27901 };
	}

	uint8_t* temp = new uint8_t[size];
	memset(temp, 0, size);

	int N = (kernel_length - 1) / 2;

	// Apply blur along Y axis
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int channel = 0; channel < channels; ++channel)
			{
				double sum = 0;

				for (int i = -N; i <= N; i++) {
					int index = (get_border_values(height, y + i) * width + x) * channels + channel;
					sum += kernel[i + N] * data[index];
				}
				
				uint8_t new_value = round(sum);
				temp[(y * width + x) * channels + channel] = BYTE_BOUND(new_value);
			}
		}
	}


	// Apply blur along X axis
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int channel = 0; channel < channels; ++channel)
			{
				double sum = 0;

				for (int i = -N; i <= N; i++) {
					int index = (y * width + get_border_values(width, x + i)) * channels + channel;
					sum += kernel[i + N] * temp[index];
				}

				uint8_t new_value = round(sum);
				data[(y * width + x) * channels + channel] = BYTE_BOUND(new_value);
			}
		}
	}

	delete[] temp;
	return *this;
}

Image& Image::edge_detection(double cutoff)
{
	grayscale_avg();

	int sobel_kernel_x[] = {1, 0, -1, 2, 0, -2, 1, 0, -1};
	int sobel_kernel_y[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

	int8_t* tempX = new int8_t[size];
	int8_t* tempY = new int8_t[size];

	memset(tempX, 0, size);
	memset(tempY, 0, size);

	// Apply edge detection kernel along X axis
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int channel = 0; channel < channels; ++channel)
			{
				int8_t sum = 0;

				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; ++j)
					{
						int kernel_index = 4 + i * 3 + j;
						int index = (get_border_values(height, y + i) * width + get_border_values(width, x + j)) * channels + channel;
						sum += sobel_kernel_x[kernel_index] * data[index];
					}
				}

				tempX[(y * width + x) * channels + channel] = sum;
			}
		}
	}


	// Apply edge detection kernel along Y axis
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int channel = 0; channel < channels; ++channel)
			{
				int8_t sum = 0;

				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; ++j)
					{
						int kernel_index = 4 + i * 3 + j;
						int index = (get_border_values(height, y + i) * width + get_border_values(width, x + j)) * channels + channel;
						sum += sobel_kernel_y[kernel_index] * data[index];
					}
				}

				tempY[(y * width + x) * channels + channel] = sum;
			}
		}
	}

	for (size_t i = 0; i < size; ++i)
	{
		data[i] = MAP_BLACK_WHITE(round(sqrt(tempX[i] * tempX[i] + tempY[i] * tempY[i])), cutoff);
	}

	delete[] tempX;
	delete[] tempY;
	return *this;
}

Image& Image::sharpen()
{
	double kernel[] = { -0.11111, -0.11111, -0.11111, -0.11111, 2, -0.11111, -0.11111, -0.11111, -0.11111 };

	uint8_t* dst = new uint8_t[size];
	memset(dst, 0, size);

	// Apply sharpening kernel
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int channel = 0; channel < channels; ++channel)
			{
				double sum = 0;

				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; ++j)
					{
						int kernel_index = 4 + i * 3 + j;
						int index = (get_border_values(height, y + i) * width + get_border_values(width, x + j)) * channels + channel;
						sum += kernel[kernel_index] * data[index];						
					}
				}

				dst[(y * width + x) * channels + channel] = BYTE_BOUND(round(sum));
			}
		}
	}

	for (size_t i = 0; i < size; ++i)
	{
		data[i] = (dst[i] + data[i]) / 2;
	}

	delete[] dst;
	dst = nullptr;

	return *this;
}
