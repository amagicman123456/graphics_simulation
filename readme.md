<div>

## A graphics project.

This is, at the moment, a learning project to explore the math of 3d rendering.

Control set to be made with WASD and arrow keys.

![image](/examples/img.png)

</div>

Build in directory with

```x86_64-w64-mingw32-g++ main.cpp -O3 -march=native -I ./ -lwinmm -L. ./dll/opencl.dll -static -o main```