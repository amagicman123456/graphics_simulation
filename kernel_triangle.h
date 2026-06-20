#pragma once

typedef struct triangle{
	const vector3 v1, v2, v3;
	const uint img;

	const vector3 center;
	const vector3 normal;
} triangle;
hit_val check_intersection_triangle(vector3* origin, vector3* u, const triangle *tri, const uint img, const uint row_bytes, uint width, uint height){
	if(!tri) return (hit_val){.first = 0, .second = 0, .third = 0, .fourth = 0};

	if(!u->is_normalized) normalize_vector3(u);

	// moller-trumbore intersection algorithm implementation

	const float epsilon = FLT_EPSILON;

	hit_val h = {.first = 0, .second = 0, .third = 0, .fourth = 0};

	if(img == 0 || row_bytes == 0 || width == 0 || height == 0) return h;

	vector3 first_edge = difference_vector3(&tri->v2, &tri->v1);
	vector3 second_edge = difference_vector3(&tri->v3, &tri->v1);

	vector3 normal = cross_product(&first_edge, &second_edge);
	if(dot_product(&normal, u) > 0) return h;

	vector3 u_cross_second_edge = cross_product(u, &second_edge);
	float determinant = dot_product(&first_edge, &u_cross_second_edge);
	if(fabs(determinant) < epsilon) return h; // check if parallel

	float inverse_det = 1.0f / determinant;
	vector3 s = difference_vector3(origin, &tri->v1);
	float uu = inverse_det * dot_product(&s, &u_cross_second_edge);

	if(uu < -epsilon || uu - 1 > epsilon) return h; // outside second edge

	vector3 s_cross_first_edge = cross_product(&s, &first_edge);
	float v = inverse_det * dot_product(u, &s_cross_first_edge);

	if(v < -epsilon || uu + v - 1 > epsilon) return h; // outside first edge

	float t = inverse_det * dot_product(&second_edge, &s_cross_first_edge);
	if(t <= epsilon) return h;

	vector3 ray_vector_t = multiply_vector3_by_scalar(u, t);
	vector3 hit_spot = sum_vector3(origin, &ray_vector_t);
/*
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
*/
	h.first = true;
	h.second = 1.0f;
	h.third = 255;
	h.fourth = 0;
	return h;
}