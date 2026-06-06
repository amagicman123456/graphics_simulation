#pragma once
#include <utility>
#include <fstream>
#include <cmath>
#include <memory>

/*
	image.hpp, custom image handler. reinventing the wheel for learning purposes. - ryu
*/

typedef std::shared_ptr<uint8_t[]> data_ptr;
struct image{
	image() : width(0), row_bytes(0), height(0), data(nullptr){}
	image(uint32_t a, int32_t b, data_ptr c, uint32_t sz = 0) : width(a), row_bytes((3 * a + 3) & -4) /* round up to multiple of 4 */, height(b), data(c), sz(sz)
	{
		if (sz == 0)
		{
			this->sz = 3 * a * std::abs(b);
		}
	}
	uint32_t width;
	uint32_t row_bytes;
	int32_t height;
	data_ptr data;

	uint32_t sz;

	uint32_t direct_color(uint32_t index) const{
		if (index + 2 >= sz)
		{
			return 0;
		}
		return (uint32_t)data[index] << 16 |
			   (uint32_t)data[index + 1] << 8 |
			   (uint32_t)data[index + 2];
	}
	uint32_t color_at(uint32_t x, uint32_t y) const{
		uint32_t index = y * row_bytes + 3 * x;
		return direct_color(index);
	}
};

image read_rgb_image(const char* const path){
	std::ifstream bitmap{path, std::ios::in | std::ios::binary};
	if(!bitmap) return image(0, 0, data_ptr(nullptr));
	char buf[26];
	bitmap.read(buf, 26);
	uint32_t width = *reinterpret_cast<uint32_t*>(buf + 18);
	int32_t height = *reinterpret_cast<int32_t*>(buf + 22);
	bool sign = height > 0;
	uint32_t row_bytes = (3 * width + 3) & -4;
	uint32_t size = row_bytes * (sign ? height : -height);
	data_ptr data = std::make_unique<uint8_t[]>(size);
	bitmap.seekg(*reinterpret_cast<uint32_t*>(buf + 10), std::ios_base::beg);
	bitmap.read(reinterpret_cast<char*>(data.get()), size);
	if(sign)
		for(uint32_t i = 0; i < size; i += 3){
			uint8_t temp = data[i];
			data[i] = data[i + 2];
			data[i + 2] = temp;
		}
	return image(width, height, std::move(data), size);
}