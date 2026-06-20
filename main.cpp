/*
	main.cpp, created 5/16/2026 :D
*/

#define CL_TARGET_OPENCL_VERSION 300

#include <windows.h>
#include "CL/cl.h"

#include <chrono>
#include <csignal>
#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>
#include <thread>
#include <filesystem>
namespace fs = std::filesystem;

#include "windows_rendering.hpp"
#include "mouseinputs.hpp"
#include "keydown.hpp"
#include "render_and_resize.hpp"
#include "utility.hpp"

/*
    The original complimation command was
    x86_64-w64-mingw32-g++ main.cpp -DCL_OPTIMIZATION -Ofast -march=native -I ./ -lgdi32 -lwinmm -L. ./dll/opencl.dll -std=c++20 -static -o main
*/

int width_px = 1200, height_px = 600;
float width = 1, height = height_px / (double)width_px,
      pixel_inc = width / width_px;
extern float origin_x, origin_y, origin_z;

/*
    more debugging purposes, see some values before it all comes crashing down
*/
int SEG_ARR[5]{};
bool TEST_SEG = false;
void segfault_handler(int sig) {
    std::cerr << "guys sigsegv " << sig << " oh also " << std::boolalpha << TEST_SEG << '\n';
    std::cerr << "anyway they wanted you to see: ";
    for (const int& i : SEG_ARR)
    {
        std::cout << i << ' ';
    }
    std::cerr << '\n';
    std::fflush(stderr);
    std::_Exit(1); // according to what i read, the safest _Exit (get it ok nobody laughed) from a signal handler
}

int main() {

    std::signal(SIGSEGV, segfault_handler);
    std::thread t(fps);

    /*
    For if you want to play sound files consecutively
    std::string path = std::string("sounds/");
    try{
        for(const auto& file : fs::directory_iterator(path)){
            sounds.emplace_back(std::make_unique<char[]>(256));
            strncpy(sounds.back().get(), file.path().string().c_str(), 255);
        }
        std::thread a(play_sounds, nullptr);
        a.detach();
    }catch(...){std::cout << "playing sound failed\n";}
    */

    if (!PlaySound(TEXT("sounds/saturn.wav"), nullptr, SND_FILENAME | SND_ASYNC | SND_LOOP)) std::cout << "sound failed\n";

    window{"rendering", width_px, height_px, reinterpret_cast<WNDPROC>(winproc), false};
    window_loop();
    return 0;
}
