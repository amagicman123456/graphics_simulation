#define true 1
#define false 0

#undef M_PI
#define M_PI 3.14159265358979323846f

/*
	kernel code passed to the GPU.
*/

#include "kernel_vectors_and_quaternions.h"
#include "kernel_image_copy.h"
#include "kernel_sphere.h"
#include "kernel_triangle.h"
/*
	big rendering function, sorry.
*/
inline uint color(uint r, uint g, uint b){
	return (r << 16) | (g << 8) | b;
}
__kernel void render(
    int width_px,
    int height_px,
	float width,
	float height,
	float origin_x, float origin_y, float origin_z,
	float rotation_w, float rotation_x, float rotation_y, float rotation_z,
	uint img,
	__global uint* row_bytes,
	__global uint* img_width,
	__global uint* img_height,
    __global uint* C)
{
	vector3 origin = (vector3){.x = origin_x, .y = origin_y, .z = origin_z};
	
	quaternion rotation = (quaternion){.w = rotation_w, .x = rotation_x, .y = rotation_y, .z = rotation_z};

	/*
	//unit test
	{
		vector3 test_vector = (vector3){.x = 1.0f, .y = 1.0f, .z = 1.0f}, test_vector2 = test_vector;
		vector3 axis = (vector3){.x = 0.0f, .y = 1.0f, .z = 0.0f};

		rotate3d(&test_vector, 2 * M_PI, &axis);
		if(!is_vector_equal(&test_vector, &test_vector2)) printf("not equal!!!\n");

		rotate3d(&test_vector, M_PI, &axis);
		vector3 test_vector3 = negated(&test_vector2);
		if(!is_vector_equal(&test_vector, &test_vector3)) printf("not equal!!!\n");
	}
	*/

    int row = get_global_id(0);
    int col = get_global_id(1);

	if(width_px == 0 || height_px == 0 || width == 0 || height == 0 || img == 0 || row_bytes == 0 || img_width == 0 || img_height == 0 || C == NULL)
		printf("oh no somethings up guys!\n");

    uint bg_rgb = color(255, 255, 255);
	uint sphere_rgb = color(255, 0, 0);

	#ifndef horizontal_fov
	    #define horizontal_fov 90
	#endif
	const float convert_horizontal_fov_to_radians = //todo: maybe just make it a bool and ternary the 0.0175 in calculating for z
	#ifdef in_radians
	//1 / 0.0175f //to cancel out the 0.0175
	1
	#else
	//1
	0.0175f
	#endif
	;
	float z = width / (2.0f * tan(horizontal_fov * convert_horizontal_fov_to_radians / 2.0f));

	float pixel_inc = width / width_px;

	/*
		this is where injector.cpp is used
	*/
	//INJECTOR.CPP CALL
    const int len = 3;
    const sphere object_arr[len] = {
        {.center = (vector3){.x = 0, .y = 0, .z = 10}, .radius = 3.0f, .img = 3},
        {.center = (vector3){.x = 0, .y = 25, .z = 2000}, .radius = 350, .img = 5},
		{.center = (vector3){.x = 0, .y = 0, .z = 0}, .radius = 1000, .img = 6},
        //{.center = (vector3){.x = 0, .y = -700, .z = 0}, .radius = 300, .img = 3},
        //{.center = (vector3){.x = 0, .y = 25, .z = 200}, .radius = sqrt(10000.0f), .img = 5}
    };
	triangle tri = {.v1 = {.x = 0, .y = 10, .z = 10}, .v2 = {.x = 10, .y = -10, .z = 10}, .v3 = {.x = -10, .y = -10, .z = 10}, .img = 3};

    //END INJECTOR.CPP CALL

	/*
	const sphere s1 = {.center = (vector3){.x = 0, .y = 50, .z = 2000}, .radius = sqrt(100000.0f)};
	const sphere s2 = {.center = (vector3){.x = 0, .y = 0, .z = 0}, .radius = sqrt(1000.0f)};
	*/
    if(row < width_px && col < height_px){
        int index = (col * width_px + row) * 2;

		/* for rotations later
		vector3 row_temp = multiply_vector3_by_scalar(&row_inc, row);
		vector3 col_temp = multiply_vector3_by_scalar(&column_dec, col);
		vector3 pixel = difference_vector3(&row_temp, &col_temp);
		*/

		vector3 pixel = (vector3){.x = -width / 2 + (row * pixel_inc), .y = height / 2 - (col * pixel_inc), .z = z};

		//pixel.x += origin_x, pixel.y += origin_y, pixel.z += origin_z;

		rotate_by_quaternion(&pixel, &rotation);
		normalize_vector3(&pixel);

    	hit_val h[len + 1];

    	for (int i = 0; i < len; i++)
    	{
    		h[i] = check_intersection_sphere(&origin, &pixel, &object_arr[i], object_arr[i].img, row_bytes[object_arr[i].img - 1], img_width[object_arr[i].img - 1], img_height[object_arr[i].img - 1]);
    	}
    	h[len] = check_intersection_triangle(&origin, &pixel, &tri, tri.img, row_bytes[tri.img - 1], img_width[tri.img - 1], img_height[tri.img - 1]);

		float min = -1;
		int min_index = -1;

		for(int i = 0; i < len + 1; ++i){
			if(!h[i].first) continue;
			if(h[i].first && min_index == -1){min = h[i].second; min_index = i; continue;}
			if(h[i].second < min){
				min_index = i;
				min = h[i].second;
			}
		}
		if(min_index == -1){
			C[index] = bg_rgb;
			C[index + 1] = 0;
		}else
		{
			C[index] = h[min_index].third;
			C[index + 1] = !!(h[min_index].fourth) * h[min_index].fourth;
		}

/*
		old principle:

		if(h1.first){
			//printf("got it\n");
			C[index] = h1.third;
		}else if(h2.first){
			C[index] = h2.third;
		}else C[index] = bg_rgb;
*/
    }
}