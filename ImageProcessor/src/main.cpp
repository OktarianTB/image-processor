#pragma once
#include <iostream>
#include "Image.h"


int main()
{
	Image img("image.jpg");

	if (img.is_valid())
	{
		// Manipulate images here
		img.write("new-image.jpg");
	}

	std::cout << "Finished processing images." << std::endl;
	std::cin.get();
	return 0;
}
