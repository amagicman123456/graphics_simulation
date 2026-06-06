#pragma once
#include "image.hpp"
#include "utility.hpp"

/*
	render_and_resize.hpp, for the call to render to the screen, as well as resizing the window (which has not been implemented yet).
*/

extern int SEG_ARR[5];
extern bool TEST_SEG;

inline size_t kernel_length = 0;

inline std::string load_file(const std::string& path){
	if (!fs::exists(path)) return "";
	std::ifstream file(path, std::ios::binary);
	const size_t size = fs::file_size(path);
	std::string buffer(size, '\0');
	file.read(&buffer[0], static_cast<long long>(size));
	kernel_length = size;
	return buffer;
}
/*
bool b = []()
{
	std::cout << load_file("kernel_src.c").c_str();
	return true;
}();
*/

// avoiding UB but also making a global object...
std::string temporary = load_file("kernel_src.c");
inline const char* kernel_src = temporary.c_str();

size_t bytes = 2 * width_px * height_px * sizeof(uint32_t);
std::vector<uint32_t> C_gpu(2 * width_px * height_px);

//std::vector A(width_px * height_px, 1);
//std::vector B(width_px * height_px, 1);

//inline image sphere_image = read_rgb_image("./images/test.bmp");

uint32_t img_number = 1;
std::vector<image> images; /*= {
	read_rgb_image("./images/test.bmp"),
};*/
static bool complete = []()
{
	try {
		for (const auto& entry : fs::directory_iterator(fs::current_path().append("images"))) {
			if (entry.is_regular_file()) {
				std::string path = entry.path().string();
				const char *c = path.c_str();
				images.emplace_back(read_rgb_image(c));
			}
		}
	} catch (const fs::filesystem_error& e) {
		std::cerr << "error: " << e.what() << "\n";
	}
	img_number = images.size();
	return true;
}();

//std::vector<uint8_t> data{images[0].data.get(), images[0].data.get() + 3 * images[0].width * images[0].height - 1};

//float rotation_float = 0.3f;
float origin_x = 0, origin_y = 0, origin_z = 0;
std::function<void()> render =
[]{
	++framerate;

	const size_t global[2] = {static_cast<size_t>(width_px), static_cast<size_t>(height_px)};

	static const std::vector<size_t> byte_sizes = {
		/*sizeof(rotation_float),*/ /*static_cast<size_t>(3 * sphere_image.width * sphere_image.height),*/
		sizeof(img_number) * img_number, sizeof(img_number) * img_number, sizeof(img_number) * img_number, bytes
	};

	static std::vector<uint32_t> row_bytes(img_number);
	static std::vector<uint32_t> widths(img_number);
	static std::vector<uint32_t> heights(img_number);

	static bool init = [&]()
	{
		for (uint32_t i = 0; i < img_number; ++i)
		{
			row_bytes[i] = images[i].row_bytes;
			widths[i] = images[i].width;
			heights[i] = images[i].height;
		}

		repeat_calls_with_same_args({false, {}}, "render", {bytes}, global, 2, width_px, height_px, width, height, origin_x, origin_y, origin_z, current_rotation.w, current_rotation.x, current_rotation.y, current_rotation.z, C_gpu);
		return true;
	}();
	static const std::unordered_set<uint32_t> indices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

	repeat_calls_with_same_args({true, indices}, "render", byte_sizes, global, 2, width_px/*0*/, height_px/*1*/, width/*2*/, height/*3*/, origin_x/*4*/, origin_y/*5*/, origin_z/*6*/, current_rotation.w/*7*/, current_rotation.x/*8*/, current_rotation.y/*9*/, current_rotation.z/*10*/,
#if 0
	data/*11*/
#endif
	img_number,
	//sphere_image.row_bytes/*12*/, sphere_image.width/*13*/, sphere_image.height/*14*/,
	row_bytes, widths, heights,
	C_gpu/*15*/);

	size_t i = 0;

	for (; i < C_gpu.size(); i += 2)
	{
		if (!C_gpu[i + 1])
		{
			framebuf[i / 2] = C_gpu[i];
		}
		else
		{
#ifdef COLOR_FILTER
			if (images[C_gpu[i] - 1].direct_color(C_gpu[i + 1]) != 0) framebuf[i / 2] = RGB(0, 0, 255);
			else framebuf[i / 2] = 0;
#else
			image& img = images[C_gpu[i] - 1];

			uint32_t val = C_gpu[i + 1];
			uint32_t color = img.direct_color(val);
			framebuf[i / 2] = color;
			//framebuf[i / 2] = images[C_gpu[i] - 1].direct_color(C_gpu[i + 1]);
#endif
		}
#ifdef INVERSION
		framebuf[i / 2] = 0xFF000000 | (~framebuf[i / 2] & 0x00FFFFFF);
#endif
	}
}
, resize =
[]{
    bytes = width_px * height_px * sizeof(uint32_t);
    //A = std::vector(width_px * height_px, 1);
    //B = std::vector(width_px * height_px, 1);
    C_gpu = std::vector<uint32_t>(2 * width_px * height_px);
	//vertical_fov = atan(tan(height_px / 2.0) * height_px / (double)width_px);
	height = height_px / (double)width_px;
	pixel_inc = width / width_px;
	std::cout << "width_px: " << width_px << ", height_px: " << height_px << std::endl;
};