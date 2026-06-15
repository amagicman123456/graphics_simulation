#define true 1
#define false 0

#undef M_PI
#define M_PI 3.14159265358979323846f

/*
	Kernel code passed to the GPU. These comments are funny so I'll put more of them -ryu
*/

/*
	quaternion part
*/

typedef struct quaternion {
    float x, y, z, w; // vector3 & scalar if anyone was wondering
	bool is_normalized;
} quaternion;
inline quaternion conjugate_quaternion(const quaternion* q) {
	if(q == NULL) return (quaternion){.w = 0, .x = 0, .y = 0, .z = 0};
    return (quaternion){.w = q->w, .x = -q->x, .y = -q->y, .z = -q->z};
}
inline quaternion multiply_quaternions(const quaternion* q1, const quaternion* q2) {
    if(q1 == NULL || q2 == NULL) return (quaternion){.w = 0, .x = 0, .y = 0, .z = 0};
	return
	(quaternion){
		.w = q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z,
		.x = q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y,
		.y = q1->w * q2->y - q1->x * q2->z + q1->y * q2->w + q1->z * q2->x,
		.z = q1->w * q2->z + q1->x * q2->y - q1->y * q2->x + q1->z * q2->w
	};
}
inline float squared_magnitude_quaternion(const quaternion* q) {
	if(q == NULL) return 0;
    return q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w;
}
inline float magnitude_quaternion(const quaternion* q) {
	if(q == NULL) return 0;
    return sqrt(squared_magnitude_quaternion(q));
}
inline void normalize_quaternion(quaternion* q) {
	if(q == NULL) return;
	q->is_normalized = true;
    const float magnitude = magnitude_quaternion(q);
    if(magnitude == 0) return;
    q->x /= magnitude, q->y /= magnitude, q->z /= magnitude, q->w /= magnitude;
}
quaternion inverse_quaternion(const quaternion* q) {
	quaternion inv = (quaternion){.w = 0, .x = 0, .y = 0, .z = 0};
	if(q == NULL) return inv;
    float n2 = squared_magnitude_quaternion(q);
    if (n2 > 0.0f) {
        quaternion conj = conjugate_quaternion(q);
        inv.w = conj.w / n2;
        inv.x = conj.x / n2;
        inv.y = conj.y / n2;
        inv.z = conj.z / n2;
    }
    return inv;
}
typedef struct vector3{
	float x, y, z;
	bool is_normalized;
} vector3;
typedef struct hit_val{
    bool first;
    float second;
    uint third;
	uint fourth;
} hit_val;
inline bool is_float_equal(float a, float b) {
    return fabs(a - b) <= 0.00001f;
}
inline bool is_vector_equal(const vector3* a, const vector3* b){
	if(a == NULL || b == NULL) return 0;
	return is_float_equal(a->x, b->x) || is_float_equal(a->x, b->x) || is_float_equal(a->x, b->x);
}
inline float squared_magnitude_vector3(const vector3* v) {
	if(v == NULL) return 0;
    return v->x * v->x + v->y * v->y + v->z * v->z;
}
inline float magnitude_vector3(const vector3* v) {
	if(v == NULL) return 0;
    return sqrt(squared_magnitude_vector3(v));
}
inline void normalize_vector3(vector3* v) {
	if(v == NULL) return;
	v->is_normalized = true;
    const float magnitude = magnitude_vector3(v);
    if(magnitude == 0) return;
    v->x /= magnitude, v->y /= magnitude, v->z /= magnitude;
}
inline vector3 cross_product(const vector3* a, const vector3* b){
	if(a == NULL || b == NULL) return (vector3){.x = 0, .y = 0, .z = 0};
	return (vector3){.x = a->y * b->z - b->y * a->z, .y = a->z * b->x - b->z * a->x, .z = a->x * b->y - b->x * a->y};
}
inline vector3 negated(const vector3* v){
	if(v == NULL) return (vector3){.x = 0, .y = 0, .z = 0};;
	return (vector3){.x = -(v->x), .y = -(v->y), .z = -(v->z)};
}
inline float dot_product(const vector3* a, const vector3* b){
	if(a == NULL || b == NULL) return 0;
	return a->x * b->x + a->y * b->y + a->z * b->z;
}
inline vector3 sum_vector3(const vector3* a, const vector3* b) {
	if(a == NULL || b == NULL) return (vector3){.x = 0, .y = 0, .z = 0};
    return (vector3){a->x + b->x, a->y + b->y, a->z + b->z};
}
inline vector3 difference_vector3(const vector3* a, const vector3* b) {
	if(a == NULL || b == NULL) return (vector3){.x = 0, .y = 0, .z = 0};
    return (vector3){a->x - b->x, a->y - b->y, a->z - b->z};
}
inline vector3 multiply_vector3_by_scalar(const vector3* a, float v) {
    if(a == NULL) return (vector3){.x = 0, .y = 0, .z = 0};
	return (vector3){a->x * v, a->y * v, a->z * v};
}
inline vector3 multiply_vector3_by_vector3(const vector3* a, const vector3* b) {
	if(a == NULL || b == NULL) return (vector3){.x = 0, .y = 0, .z = 0};
    return (vector3){a->x * b->x, a->y * b->y, a->z * b->z};
}

inline void rotate3d(vector3* v, const float theta, const vector3* axis){
	if(v == NULL || axis == NULL) return;

	float half_theta = theta / 2.0f;
	float sine_half_theta = sin(half_theta);

	quaternion v_representation = (quaternion){
		.w = 0,
		.x = v->x,
		.y = v->y,
		.z = v->z
	};
	quaternion rotation = (quaternion){
		.w = cos(half_theta),
		.x = axis->x * sine_half_theta,
		.y = axis->y * sine_half_theta,
		.z = axis->z * sine_half_theta
	};
	normalize_quaternion(&rotation);
	quaternion rotation_inverse = inverse_quaternion(&rotation);
	quaternion first_product  = multiply_quaternions(&rotation, &v_representation);
	quaternion second_product = multiply_quaternions(&first_product, &rotation_inverse);

	/*
	for debugging purposes:

	printf("rotation: %f %f %f %f, axis: %f %f %f, rotation inverse: %f %f %f %f, first: %f %f %f %f, second: %f %f %f %f, v: %f %f %f\n",
		rotation.w, rotation.x, rotation.y, rotation.z,
		axis->x, axis->y, axis->z,
		rotation_inverse.w, rotation_inverse.x, rotation_inverse.y, rotation_inverse.z,
		first_product.w, first_product.x, first_product.y, first_product.z,
		second_product.w, second_product.x, second_product.y, second_product.z,
		v->x, v->y, v->z);*/

	v->x = second_product.x, v->y = second_product.y, v->z = second_product.z;
}
inline void rotate_by_quaternion(vector3* v, const quaternion *rotation){
	if(v == NULL || rotation == NULL) return;

	quaternion v_representation = (quaternion){
		.w = 0,
		.x = v->x,
		.y = v->y,
		.z = v->z
	};
	quaternion rotation_inverse = inverse_quaternion(rotation);
	quaternion first_product  = multiply_quaternions(rotation, &v_representation);
	quaternion second_product = multiply_quaternions(&first_product, &rotation_inverse);

	/*
	also for debugging purposes:
	printf("rotation: %f %f %f %f, rotation inverse: %f %f %f %f, first: %f %f %f %f, second: %f %f %f %f, v: %f %f %f\n",
		rotation->w, rotation->x, rotation->y, rotation->z,
		rotation_inverse.w, rotation_inverse.x, rotation_inverse.y, rotation_inverse.z,
		first_product.w, first_product.x, first_product.y, first_product.z,
		second_product.w, second_product.x, second_product.y, second_product.z,
		v->x, v->y, v->z);*/

	v->x = second_product.x, v->y = second_product.y, v->z = second_product.z;
}

/*
	some functions essentially the same as image.hpp
*/
inline uint index_at(uint row_bytes, uint x, uint y)
{
	return y * row_bytes + 3 * x;
}
inline uint color_at(__global const uchar* data, uint row_bytes, uint x, uint y) {
	uint index = index_at(row_bytes, x, y);
	return (uint)data[index] << 16 |
		   (uint)data[index + 1] << 8 |
		   (uint)data[index + 2];
}

/*
	sphere part
*/
typedef struct sphere{
	const vector3 center;
	const float radius;
	const uint img;
} sphere;
hit_val check_intersection_sphere(vector3* origin, vector3* u, const sphere *sph, const uint img, const uint row_bytes, uint width, uint height){
	if(sph == NULL) return (hit_val){.first = 0, .second = 0, .third = 0, .fourth = 0};

	const vector3 *center = &sph->center;
	const float radius = sph->radius;

	if(!u->is_normalized) normalize_vector3(u);

	vector3 center_negative = difference_vector3(origin, center);
	float dot = dot_product(u, &center_negative);
	//float determinant = dot * dot - (center->x * center->x + center->y * center->y + center->z * center->z - radius * radius);
	float a = dot_product(u, u);
	//vector3 twice = sum_vector3(&center_negative, &center_negative);
	//float b = dot_product(&twice, u);
	float b = 2 * dot;
	float c = dot_product(&center_negative, &center_negative) - radius * radius;
	float determinant = b * b - 4 * a * c;
	if(determinant < 0) return (hit_val){.first = false, .second = 0, .third = 0, .fourth = 0};

	float sq = sqrt(determinant);
	float dist1 = -dot - sq;
	float dist2 = -dot + sq;

	float distance = (dist1 > 0) ? dist1 : (dist2 > 0 ? dist2 : -1);
	if (distance < 0) return (hit_val){.first = 0, .second = 0, .third = 0, .fourth = 0};

	hit_val h = {.first = true, .second = distance, .third = 0, .fourth = 0};

	if(img == 0 || row_bytes == 0 || width == 0 || height == 0) return h;
	vector3 product = multiply_vector3_by_scalar(u, distance);
	product = sum_vector3(origin, &product);

	vector3 hit_spot = difference_vector3(&product, center);
	float magnitude = magnitude_vector3(&hit_spot);
	if(magnitude == 0){
		printf("oh no the magnitude is 0\n");
		return h;
	}
	//normalize_vector3(&hit_spot);
	hit_spot.is_normalized = true;
    hit_spot.x /= magnitude, hit_spot.y /= magnitude, hit_spot.z /= magnitude;
	{
		//avoid shadowing
		float uu = 0.5f + atan2(hit_spot.z, hit_spot.x) / (2 * M_PI);
		float vv = 0.5f + asin(hit_spot.y) / M_PI;

		//clamp is a built in opencl function hooray
		uu = clamp(uu, 0.0f, 1.0f);
		vv = clamp(vv, 0.0f, 1.0f);

		long x = convert_long_rte(uu * width);
		long y = convert_long_rte(vv * height);

		//clamp again for assurance purposes
		x = clamp(x, (long)(0), (long)(width - 1));
		y = clamp(y, (long)(0), (long)(height - 1));

		uint hit_index = index_at(row_bytes, x, y);

		h.third = img;
		h.fourth = hit_index;
		return h;
	}
}

/*
	triangle part
*/
#if 0
typedef struct triangle{
	const vector3 center;
	const vector3 normal;
	const vector3 v1, v2, v3;
	const uint img;
} triangle;
hit_val check_intersection_triangle(vector3* origin, vector3* u, const triangle *tri, const uint img, const uint row_bytes, uint width, uint height){
	if(tri == NULL) return (hit_val){.first = 0, .second = 0, .third = 0, .fourth = 0};

	if(!u->is_normalized) normalize_vector3(u);



	hit_val h = {.first = true, .second = distance, .third = 0, .fourth = 0};

	if(img == 0 || row_bytes == 0 || width == 0 || height == 0) return h;

	vector3 hit_spot;
	float magnitude = magnitude_vector3(&hit_spot);
	if(magnitude == 0){
		printf("oh no the magnitude is 0\n");
		return h;
	}
	//normalize_vector3(&hit_spot);
	hit_spot.is_normalized = true;
    hit_spot.x /= magnitude, hit_spot.y /= magnitude, hit_spot.z /= magnitude;
	{
		float uu, vv;

		//clamp is a built in opencl function hooray
		uu = clamp(uu, 0.0f, 1.0f);
		vv = clamp(vv, 0.0f, 1.0f);

		long x = convert_long_rte(uu * width);
		long y = convert_long_rte(vv * height);

		//clamp again for assurance purposes
		x = clamp(x, (long)(0), (long)(width - 1));
		y = clamp(y, (long)(0), (long)(height - 1));

		uint hit_index = index_at(row_bytes, x, y);

		h.third = img;
		h.fourth = hit_index;
		return h;
	}
}
#endif
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

    	hit_val h[len];

    	for (int i = 0; i < len; i++)
    	{
    		h[i] = check_intersection_sphere(&origin, &pixel, &object_arr[i], object_arr[i].img, row_bytes[object_arr[i].img - 1], img_width[object_arr[i].img - 1], img_height[object_arr[i].img - 1]);
    	}

		float min = -1;
		int min_index = -1;

		for(int i = 0; i < len; ++i){
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
			if (h[min_index].fourth)
			{
				C[index + 1] = h[min_index].fourth;
			}
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