#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Image.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#define BYTE_BOUND(x) x < 0 ? 0 : (x > 255 ? 255 : x)
#define MAP_BLACK_WHITE(x) x < 115 ? 0 : 255;

using namespace std;


Image::Image(const char* filename)
{
	if (read(filename))
	{
		printf("Successfully read %s\n", filename);
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

Image& Image::crop(uint16_t cx, uint16_t cy, uint16_t ch, uint16_t cw)
{
	size = cw * ch * channels;
	uint8_t* croppedImage = new uint8_t[size];
	memset(croppedImage, 0, size);

	for (uint16_t y = 0; y < ch; ++y)
	{
		if (y + cy >= height) break;

		for (uint16_t x = 0; x < cw; ++x)
		{
			if (x + cx >= width) break;

			memcpy(&croppedImage[(x + y * cw) * channels], &data[(x + cx + (cy + y) * width) * channels], channels);
		}
	}

	width = cw;
	height = ch;

	delete[] data;
	data = croppedImage;
	croppedImage = nullptr;

	return *this;
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

Image& Image::colorMask(float r, float g, float b)
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

Image& Image::diffMap(Image& img)
{
	int compare_width = fmin(width, img.width);
	int compare_height = fmin(height, img.height);
	int compare_channels = fmin(channels, img.channels);

	for (uint32_t i = 0; i < compare_height; ++i)
	{
		for (uint32_t j = 0; j < compare_width; ++j)
		{
			for (uint32_t k = 0; k < compare_channels; ++k)
			{
				data[(i * width + j) * channels + k] = BYTE_BOUND(abs(data[(i * width + j) * channels + k] -
					img.data[(i * img.width + j) * img.channels + k]));
			}
		}
	}

	return *this;
}

Image& Image::diffMap_scale(Image& img, uint8_t scale)
{
	int compare_width = fmin(width, img.width);
	int compare_height = fmin(height, img.height);
	int compare_channels = fmin(channels, img.channels);

	uint8_t largest = 0;
	for (uint32_t i = 0; i < compare_height; ++i)
	{
		for (uint32_t j = 0; j < compare_width; ++j)
		{
			for (uint32_t k = 0; k < compare_channels; ++k)
			{
				data[(i * width + j) * channels + k] = BYTE_BOUND(abs(data[(i * width + j) * channels + k] -
					img.data[(i * img.width + j) * img.channels + k]));
				largest = fmax(largest, data[(i * width + j) * channels + k]);
			}
		}
	}

	scale = 255 / fmax(1, fmax(scale, largest));

	for (int i = 0; i < size; ++i)
	{
		data[i] *= scale;
	}

	return *this;
}

Image& Image::overlay(const Image& source, int x, int y)
{
	uint8_t* srcPx;
	uint8_t* dstPx;
	for (int sy = 0; sy < source.height; ++sy)
	{
		if (sy + y < 0) continue;
		else if (sy + y >= height) break;

		for (int sx = 0; sx < source.width; ++sx)
		{
			if (sx + x < 0) continue;
			else if (sx + x >= width) break;

			srcPx = &source.data[(sx + sy * source.width) * source.channels];
			dstPx = &data[(sx + x + (sy + y) * width) * channels];

			float srcAlpha = source.channels < 4 ? 1 : srcPx[3] / 255.f;
			float dstAlpha = channels < 4 ? 1 : dstPx[3] / 255.f;

			if (srcAlpha >= 0.99 && dstAlpha >= 0.99)
			{
				if (source.channels >= channels)
				{
					memcpy(dstPx, srcPx, channels);
				}
				else
				{
					memset(dstPx, srcPx[0], channels);
				}
			}
			else
			{
				float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);
				if (outAlpha < 0.01)
				{
					memset(dstPx, 0, channels);
				}
				else
				{
					for (int chnl = 0; chnl < channels; ++chnl)
					{
						dstPx[chnl] = (uint8_t)BYTE_BOUND((srcPx[chnl] / 255.f * srcAlpha + dstPx[chnl] / 255.f * dstAlpha * (1 - srcAlpha)) / outAlpha * 255.f);
					}

					if (channels > 3)
					{
						dstPx[3] = (uint8_t)BYTE_BOUND(outAlpha * 255.f);
					}
				}
			}
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
	uint8_t* dst = new uint8_t[size];

	memset(temp, 0, size);
	memset(dst, 0, size);

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
				dst[(y * width + x) * channels + channel] = BYTE_BOUND(new_value);
			}
		}
	}

	for (size_t i = 0; i < size; ++i)
	{
		data[i] = dst[i];
	}

	delete[] temp;
	delete[] dst;
	return *this;
}

Image& Image::edge_detection()
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
					int index = (get_border_values(height, y + i) * width + x) * channels + channel;
					sum += sobel_kernel_x[i + 1] * data[index];
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
					int index = (y * width + get_border_values(width, x + i)) * channels + channel;
					sum += sobel_kernel_y[i + 1] * data[index];
				}

				tempY[(y * width + x) * channels + channel] = sum;
			}
		}
	}

	for (size_t i = 0; i < size; ++i)
	{
		data[i] = MAP_BLACK_WHITE(round(sqrt(tempX[i] * tempX[i] + tempY[i] * tempY[i])));
	}

	delete[] tempX;
	delete[] tempY;
	return *this;
}


Image& Image::std_convolve_clamp_to_0(uint8_t channel, uint32_t kernel_width, uint32_t kernel_height, double kernel[], uint32_t cr, uint32_t cc)
{
	uint8_t* new_data = new uint8_t[width * height];
	uint64_t center = (uint64_t)cr * kernel_width + (uint32_t)cc;
	//printf("%" PRIu64 "\n", center);

	for (uint64_t k = channel; k < size; k += channels)
	{
		double c = 0;
		long i = -((long)cr);
		long end = (long)kernel_height - cr;
		//std::cout << "i: " << i << " - " << end << std::endl;
		while (i < end)
		{
			i++;
			long row = ((long)k / channels) / width - i;
			if (row < 0 || row > height - 1)
			{
				continue;
			}

			long j = -((long)cc);
			long inner_end = (long)kernel_width - cc;
			//std::cout << "j: " << j << " - " << inner_end << std::endl;
			while (j < inner_end)
			{
				j++;
				long col = ((long)k / channels) % width - j;
				if (col < 0 || col > width - 1)
				{
					continue;
				}
				
				long k_w = (long)kernel_width;
				long index = center + i * k_w + j;
				//std::cout << "index: " << index << std::endl;

				c += kernel[index] * data[(row * width + col) * channels + channel];
				cout << kernel[index] * data[(row * width + col) * channels + channel] << endl;
			}
		}

		new_data[k / channels] = (uint8_t)BYTE_BOUND(round(c));
		cout << "total: " << round(c) << endl;
		cout << "----------------" << endl;
		//printf("%" PRIu8 "\n", (uint8_t)round(c));
		//printf("New Data:%" PRIu8 "\n", new_data[k / channels]);
		
	}


	for (uint64_t k = channel; k < size; k += channels)
	{
		data[k] = new_data[k / channels];
		//printf("Data: %" PRIu8 "\n", data[k]);
	}

	return *this;
}









