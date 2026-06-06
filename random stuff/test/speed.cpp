#include <chrono>
#include <iostream>
#include <vector>
#include <CL/cl.h>

const char* kernelSrc = R"(
__kernel void matmul(
    __global const float* A,
    __global const float* B,
    __global float* C,
    int N)
{
    int row = get_global_id(0);
    int col = get_global_id(1);

    float sum = 0.0f;
    for (int k = 0; k < N; k++) {
        sum += A[row*N + k] * B[k*N + col];
    }
    C[row*N + col] = sum;
}
)";

void matmul_cpu(const float* A, const float* B, float* C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                sum += A[i*N + k] * B[k*N + j];
            }
            C[i*N + j] = sum;
        }
    }
}

int main() {
    const int N = 512; // 512 by 512
    size_t bytes = N * N * sizeof(float);

    std::vector<float> A(N*N, 1.0f);
    std::vector<float> B(N*N, 2.0f);
    std::vector<float> C_cpu(N*N);
    std::vector<float> C_gpu(N*N);

    // cpu
    auto t1 = std::chrono::high_resolution_clock::now();
    matmul_cpu(A.data(), B.data(), C_cpu.data(), N);
    auto t2 = std::chrono::high_resolution_clock::now();
    double cpu_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

    std::cout << "CPU time: " << cpu_ms << " ms\n";

    // gpu
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    clGetPlatformIDs(1, &platform, nullptr);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);

    cl_context ctx = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    cl_command_queue queue = clCreateCommandQueue(ctx, device, 0, &err);

    cl_mem dA = clCreateBuffer(ctx, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, bytes, A.data(), &err);
    cl_mem dB = clCreateBuffer(ctx, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, bytes, B.data(), &err);
    cl_mem dC = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, bytes, nullptr, &err);

    cl_program prog = clCreateProgramWithSource(ctx, 1, &kernelSrc, nullptr, &err);
    clBuildProgram(prog, 1, &device, nullptr, nullptr, nullptr);
    cl_kernel kernel = clCreateKernel(prog, "matmul", &err);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &dA);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &dB);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &dC);
    clSetKernelArg(kernel, 3, sizeof(int), &N);

    size_t global[2] = { (size_t)N, (size_t)N };

    // gpu time
    auto g1 = std::chrono::high_resolution_clock::now();
    clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, global, nullptr, 0, nullptr, nullptr);
    clFinish(queue);
    auto g2 = std::chrono::high_resolution_clock::now();

    double gpu_ms = std::chrono::duration<double, std::milli>(g2 - g1).count();

    std::cout << "GPU time: " << gpu_ms << " ms\n";

    clEnqueueReadBuffer(queue, dC, CL_TRUE, 0, bytes, C_gpu.data(), 0, nullptr, nullptr);

    return 0;
}
