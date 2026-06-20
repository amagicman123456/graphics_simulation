#pragma once

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

	// using the quadratic formula directly without simplification

	vector3 center_negative = difference_vector3(origin, center);
	float dot = dot_product(u, &center_negative);
	float a = dot_product(u, u);
	float b = 2 * dot;
	float c = dot_product(&center_negative, &center_negative) - radius * radius;
	float determinant = b * b - 4 * a * c;
	if(determinant < 0) return (hit_val){.first = false, .second = 0, .third = 0, .fourth = 0};

	float sq = sqrt(determinant);
	float dist1 = (-b - sq) / (2 * a);
	float dist2 = (-b + sq) / (2 * a);

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