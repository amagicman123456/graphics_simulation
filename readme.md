```x86_64-w64-mingw32-g++ main.cpp -O3 -march=native -I ./ -lwinmm -L. ./dll/opencl.dll -static -o main```

```x86_64-w64-mingw32-g++ [file.cpp] -O3 -march=native -I [directory] -L. [path to opencl.dll] -static -o [output]```