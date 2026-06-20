#pragma once

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