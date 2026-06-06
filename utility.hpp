#pragma once
#include <windows.h>
#include <unordered_set>  
#include <cstdint>
#include <vector>
#include <memory>
#include <cassert>
#include <any>
#include <atomic>
#include <thread>
#include <chrono>

/*
    utility.hpp, for random hacks that might cause UB :(
*/

using namespace std::chrono_literals;
#include "CL/cl.h"
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

inline float mouse_sensitivity = 0.007f;
inline float key_sensitivity = 1.0f;
inline float current_pitch = 0;

std::atomic<int> framerate;
[[noreturn]] void fps() {
    while(true) {
        std::this_thread::sleep_for(1000ms);
        std::cout << framerate << '\n';
        framerate = 0;
    }
}

extern const char* kernel_src;
extern size_t kernel_length;
template <typename T>
concept is_vector = std::is_same_v<T, std::vector<typename T::value_type>>;

template <typename T>
struct is_pair : std::false_type {};
template <typename U, typename V>
struct is_pair<std::pair<U, V>> : std::true_type {};
template <typename T>
inline constexpr bool is_pair_v = is_pair<T>::value;

//todo used with std::vectors
template<int n, typename ...A> void repeat_calls_with_same_args(const std::pair<bool, std::unordered_set<uint32_t>> pair_repeat, const std::string_view& func, const std::vector<size_t>& bytes, const size_t (&global)[n], int32_t dimensions, A&&... args){
    static cl_int err;
    static cl_context ctx;
    static cl_command_queue queue;
    static std::vector<cl_mem> buffers;
    static cl_program prog;
    static cl_kernel kernel;
    static cl_platform_id platform;
    static cl_device_id device;
    static std::vector<std::any> any_vector;

    bool repeat = pair_repeat.first;
    const std::unordered_set<uint32_t>& indices = pair_repeat.second;
    
    static bool done = [&]() -> bool{
        clGetPlatformIDs(1, &platform, nullptr);
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);

        ctx = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
        queue = clCreateCommandQueueWithProperties(ctx, device, 0, &err);

        //std::cout << "initializing buffers\n";
        uint32_t index_bytes = 0;
        auto add_args1 = [&]<typename T>(T& arg){
            if constexpr (is_vector<T>){
                //std::cout << "emplacing mem into buffer\n";
                buffers.emplace_back(clCreateBuffer(ctx, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, bytes[index_bytes++], arg.data(), &err));
                //std::cout << "buffer created\n";
            }
        };
        (add_args1(args), ...);

        //std::cout << "creating program\n";
        prog = clCreateProgramWithSource(ctx, 1, &kernel_src, &kernel_length, &err);
        err = clBuildProgram(prog, 1, &device,
#ifndef CL_OPTIMIZATION
        nullptr,
#else
        "-cl-fast-relaxed-math",
#endif
        nullptr, nullptr);
        if (err != CL_SUCCESS) {
            size_t log_size = 0;
            clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
            std::string log(log_size, '\0');
            clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
            std::cerr << "build log:\n" << log << "\n";
            return false;
        }
        kernel = clCreateKernel(prog, func.data(), &err);

        //std::cout << "adding args\n";
        uint32_t count = 0, index = 0;
        any_vector.clear();
        auto add_args2 = [&]<typename T>(const T& arg){
            if constexpr (is_vector<T>){
                err = clSetKernelArg(kernel, index, sizeof(cl_mem), &buffers[count]);
                ++count;
            }
            else if constexpr (is_pair_v<T>){
                any_vector.emplace_back(arg.first);
                err = clSetKernelArg(kernel, index, arg.second, any_cast<decltype(arg.first)>(&any_vector.back()));
            }
            else{
                any_vector.emplace_back(arg);
                //std::cout << "emplacing " << arg << '\n';
                err = clSetKernelArg(kernel, index, sizeof(T), any_cast<T>(&any_vector.back()));
            }
            if (err != CL_SUCCESS) {
                size_t log_size = 0;
                clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
                std::string log(log_size, '\0');
                clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
                std::cerr << "build log:\n" << log << "\n";
                return;
            }
            ++index;
        };
        (add_args2(args), ...);
        return true;
    }();
    if (repeat){
        //todo delete this later, and when you do change emplace back in add_args1
        //for(auto& i : buffers){
        //    clReleaseMemObject(i);
        //}
        //buffers.clear();

        uint32_t index_bytes = 0;
        uint32_t total_index = 0;
        /*
        auto add_args1 = [&]<typename T>(T& arg){
            if (indices.empty() || indices.contains(total_index++)){
                if constexpr (is_vector<T>){
                    //std::cout << "emplacing mem into buffer\n";
                    // if the indices is empty let it go

                    //std::cout << "emplacing\n";
                    //if (index_bytes < buffers.size()) clReleaseMemObject(buffers[index_bytes]);

                    //buffers[index_bytes] = clCreateBuffer(ctx, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, bytes[index_bytes++], arg.data(), &err);
                    buffers.emplace_back(clCreateBuffer(ctx, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, bytes[index_bytes++], arg.data(), &err));
                }
            }
        };
        (add_args1(args), ...);
        */

        //std::cout << "adding args\n";
        uint32_t count = 0, index = 0;
        any_vector.clear();
        auto add_args2 = [&]<typename T>(const T& arg){
            // if the indices is empty let it go
            if (!indices.empty() && !indices.contains(index)){/*std::cout << "inc\n";*/ ++index; return;}
            if constexpr (is_vector<T>){
                //std::cout << "vector\n";
                err = clSetKernelArg(kernel, index, sizeof(cl_mem), &buffers[count]);
                ++count;
            }
            else if constexpr (is_pair_v<T>){
                //std::cout << "pair\n";
                any_vector.emplace_back(arg.first);
                err = clSetKernelArg(kernel, index, arg.second, any_cast<decltype(arg.first)>(&any_vector.back()));
            }
            else{
                //std::cout << "another\n";
                any_vector.emplace_back(arg);
                //std::cout << "emplacing " << arg << '\n';
                err = clSetKernelArg(kernel, index, sizeof(T), any_cast<T>(&any_vector.back()));
            }
            if (err != CL_SUCCESS) {
                size_t log_size = 0;
                clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
                std::string log(log_size, '\0');
                clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
                std::cerr << "build log:\n" << err << ' ' << log << "\n";
                return;
            }
            ++index;
        };
        (add_args2(args), ...);
    }
    clEnqueueNDRangeKernel(queue, kernel, dimensions, nullptr, global, nullptr, 0, nullptr, nullptr);

	//todo double sync
    err = clEnqueueReadBuffer(queue, buffers.back(), CL_TRUE, 0, bytes.back(), (args, ...).data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        std::cerr << "enqueue failed: " << err << "\n";
    }
}

inline uint32_t rgb(uint8_t red, uint8_t green, uint8_t blue) {
    return RGB(red, green, blue);
}
std::vector<std::unique_ptr<char[]>> sounds;
void play_sounds(void*){
    for(const auto &sound : sounds)
        PlaySound(TEXT(sound.get()), nullptr, SND_FILENAME | SND_SYNC);
    sounds.clear();
}
typedef uint32_t color;
#define no_color 4278190080
extern inline uint32_t rgb(uint8_t red, uint8_t green, uint8_t blue);
const color background = rgb(255, 255, 255);
struct hit_val{
    bool first;
    double second;
    color third;
    hit_val() : first(0), second(0), third(no_color){}
    hit_val(bool b, double d, color c = no_color) : first(b), second(d), third(c){}
};

/*
struct vector3;
typedef vector3 point;
struct vector3{
	double origin_x{}, origin_y{}, origin_z{};
    double x, y, z;
    bool is_normalized = false;
    vector3(double a, double b, double c, bool normal = false) : x(a), y(b), z(c), is_normalized(normal) {}
    vector3() : x(0), y(0), z(0) {}
    vector3(const vector3& v) {
        *this = v;
    }

    [[nodiscard]] double magnitude() const {
	    if (is_normalized) return 1;
    	return sqrt(x*x + y*y + z*z);
    }
    void normalize() {
    	const double magnitude = this->magnitude();
    	assert(magnitude != 0);
    	this->x /= magnitude, this->y /= magnitude, this->z /= magnitude;
    }
    void set_normalize() {
        normalize();
    	is_normalized = true;
    }
    [[nodiscard]] double dot(vector3 v) const {
        return x*v.x + y*v.y + z*v.z;
    }
    [[nodiscard]] double squared() const {
        return x*x + y*y + z*z;
    }
    [[nodiscard]] vector3 cross(vector3 v) const {
        return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
    }
    vector3 operator+(vector3 v) const {
        return {x + v.x, y + v.y, z + v.z};
    }
    vector3 operator-(vector3 v) const {
        return {x - v.x, y - v.y, z - v.z};
    }
    vector3 operator*(vector3 v) const {
        return {x * v.x, y * v.y, z * v.z};
    }
    vector3 operator/(vector3 v) const {
        return {x / v.x, y / v.y, z / v.z};
    }
    vector3 operator*(double v) const {
        return {x * v, y * v, z * v};
    }
    vector3 operator/(double v) const {
        return {x / v, y / v, z / v};
    }
    vector3 operator-() const {
        return {-x, -y, -z};
    }
    vector3& operator-=(const vector3& g){
        x -= g.x;
        y -= g.y;
        z -= g.z;
        return *this;
    }
    vector3& operator+=(const vector3& o){
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
    vector3& operator=(const vector3& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        is_normalized = x*x + y*y + z*z == 1;
        return *this;
    }
};
inline vector3 cross_product(const vector3& a, const vector3& b){
	return vector3(a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y);
}
inline double dot_product(const vector3& a, const vector3& b){
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
*/
typedef struct quaternion {
    float w, x, y, z; // vector3 & scalar
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

    quaternion v_representation = (quaternion){
        .w = 0,
        .x = v->x,
        .y = v->y,
        .z = v->z
    };
	float half_theta = theta / 2.0f;
	float sine_half_theta = std::sin(half_theta);
	quaternion rotation = (quaternion){
		.w = std::cos(half_theta),
		.x = axis->x * sine_half_theta,
		.y = axis->y * sine_half_theta,
		.z = axis->z * sine_half_theta
	};
	normalize_quaternion(&rotation);
	quaternion rotation_inverse = inverse_quaternion(&rotation);
	//quaternion first_product = multiply_quaternions(&v_representation, &rotation_inverse);
	//quaternion second_product = multiply_quaternions(&rotation, &first_product);
	quaternion first_product  = multiply_quaternions(&rotation, &v_representation);
	quaternion second_product = multiply_quaternions(&first_product, &rotation_inverse);
	//normalize_quaternion(&second_product);

	/*
	printf("rotation: %f %f %f %f, axis: %f %f %f, rotation inverse: %f %f %f %f, first: %f %f %f %f, second: %f %f %f %f, v: %f %f %f\n",
		rotation.w, rotation.x, rotation.y, rotation.z,
		axis->x, axis->y, axis->z,
		rotation_inverse.w, rotation_inverse.x, rotation_inverse.y, rotation_inverse.z,
		first_product.w, first_product.x, first_product.y, first_product.z,
		second_product.w, second_product.x, second_product.y, second_product.z,
		v->x, v->y, v->z);*/

	v->x = second_product.x, v->y = second_product.y, v->z = second_product.z;
}
inline void rotate3d(vector3* v, const quaternion& rotation){
    if(v == NULL) return;

    quaternion v_representation = (quaternion){
        .w = 0,
        .x = v->x,
        .y = v->y,
        .z = v->z
    };

    quaternion rotation_inverse = inverse_quaternion(&rotation);
    //quaternion first_product = multiply_quaternions(&v_representation, &rotation_inverse);
    //quaternion second_product = multiply_quaternions(&rotation, &first_product);
    quaternion first_product  = multiply_quaternions(&rotation, &v_representation);
    quaternion second_product = multiply_quaternions(&first_product, &rotation_inverse);
    //normalize_quaternion(&second_product);

    /*
    printf("rotation: %f %f %f %f, axis: %f %f %f, rotation inverse: %f %f %f %f, first: %f %f %f %f, second: %f %f %f %f, v: %f %f %f\n",
        rotation.w, rotation.x, rotation.y, rotation.z,
        axis->x, axis->y, axis->z,
        rotation_inverse.w, rotation_inverse.x, rotation_inverse.y, rotation_inverse.z,
        first_product.w, first_product.x, first_product.y, first_product.z,
        second_product.w, second_product.x, second_product.y, second_product.z,
        v->x, v->y, v->z);*/

    v->x = second_product.x, v->y = second_product.y, v->z = second_product.z;
}

extern float origin_x, origin_y, origin_z;
inline auto current_rotation = quaternion(1, 0, 0, 0);
//extern float rotation_float;
void rotate_object_direct(auto& obj, quaternion rotation)
{
    obj = multiply_quaternions(&rotation, &obj);
    normalize_quaternion(&obj);
    //std::cout << current_rotation.w << ' ' << current_rotation.x << ' ' << current_rotation.y << ' ' << current_rotation.z << std::endl;
}
inline quaternion quaternion_from_angle_and_axis(float added_rotation, float axis_x, float axis_y, float axis_z)
{
    float half_theta = added_rotation / 2.0f;
    float sine_half_theta = std::sin(half_theta);
    quaternion rotation = (quaternion){
        .w = std::cos(half_theta),
        .x = axis_x * sine_half_theta,
        .y = axis_y * sine_half_theta,
        .z = axis_z * sine_half_theta
    };
    normalize_quaternion(&rotation);
    return rotation;
}
inline void process_rotation(float added_rotation, float axis_x, float axis_y, float axis_z){
    quaternion rotation = quaternion_from_angle_and_axis(added_rotation, axis_x, axis_y, axis_z);
    rotate_object_direct(current_rotation, rotation);
    //just in case
    normalize_quaternion(&current_rotation);
}