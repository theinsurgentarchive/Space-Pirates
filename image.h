#pragma once

#include <memory>

struct Image {
	uint16_t width, height;
	std::unique_ptr<unsigned char[]> data;
	~Image();
	Image(const std::string& file_name);
};
//end - gordon