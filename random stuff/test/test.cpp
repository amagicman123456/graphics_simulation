#include <iostream>
#include <vector>
#include <omp.h>
#include <cstdint>
int main() {
    const uint64_t n = 1 << 30; // 1 million
    std::vector<float> a(n, 1.0f);
    std::vector<float> b(n, 2.0f);
    std::vector<float> c(n);

    // Parallel + SIMD
    #pragma omp parallel
    {
        #pragma omp for simd schedule(static)
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }

    // Verify
    bool ok = true;
    for (int i = 0; i < n; i++) {
        if (c[i] != 3.0f) {
            ok = false;
            break;
        }
    }

    std::cout << (ok ? "Success!" : "Error!") << std::endl;
    return 0;
}
