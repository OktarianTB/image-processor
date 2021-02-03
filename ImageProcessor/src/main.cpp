#pragma once
#include <iostream>
#include "Image.h"


int main()
{
	Image test("color.png");

	test.gaussian_blur();
	test.write("color-blurred3.png");

	std::cout << "Finished processing images." << std::endl;
	std::cin.get();
	return 0;
}


	/*double kernel[] = { 1 / 16.0, 2 / 16.0, 1 / 16.0,
					2 / 16.0, 4 / 16.0, 2 / 16.0,
					1 / 16.0, 2 / 16.0, 1 / 16.0 };

	test.std_convolve_clamp_to_0(0, 3, 3, kernel, 1, 1);
	test.std_convolve_clamp_to_0(1, 3, 3, kernel, 1, 1);
	test.std_convolve_clamp_to_0(2, 3, 3, kernel, 1, 1);*/