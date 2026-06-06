#include <CL/cl.h>
#include <iostream>

const char* kernelSrc = R"(
__kernel void add1(__global float* data) {
    int id = get_global_id(0);
    data[id] += 1.0f;
}
)";

int main() {
    const int N = 8;
    float data[N] = {0,1,2,3,4,5,6,7};

    cl_int err;

    // Platform
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, nullptr);

    // Device (Intel GPU if available)
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);

    // Context + queue
    cl_context ctx = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    cl_command_queue queue = clCreateCommandQueue(ctx, device, 0, &err);

    // Buffer
    cl_mem buf = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                sizeof(data), data, &err);

    // Program + kernel
    cl_program prog = clCreateProgramWithSource(ctx, 1, &kernelSrc, nullptr, &err);
    clBuildProgram(prog, 1, &device, nullptr, nullptr, nullptr);
    cl_kernel kernel = clCreateKernel(prog, "add1", &err);

    // Set args + run
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf);
    size_t global = N;
    clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, nullptr, 0, nullptr, nullptr);

    // Read back
    clEnqueueReadBuffer(queue, buf, CL_TRUE, 0, sizeof(data), data, 0, nullptr, nullptr);

    // Print result
    for (float x : data) std::cout << x << " ";
    std::cout << "\n";

    return 0;
}

