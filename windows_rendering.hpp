#pragma once
#include <windowsx.h>
#include <windows.h>
#include <fstream>
#include <memory>
#include <cmath>
#include <utility>
#include <functional>
#include "keydown.hpp"
#include "mouseinputs.hpp"

/*
	windows_rendering.hpp, for winapi calls
*/

extern std::function<void()> render, resize;
extern std::function<void(WPARAM)> keydown;
extern std::function<void(LPARAM)> mousemove;
extern std::function<void(LPARAM)> lbuttondown;
extern int width_px, height_px;
extern float width, height, pixel_inc;
uint32_t *framebuf;
bool wm_size = false;
LRESULT CALLBACK winproc(HWND hwnd, UINT msg, WPARAM w, LPARAM l){
	static HDC pdc;
	static HBITMAP old;
	static HBITMAP bitmap;
	for(const int& i : desired_keys){
		if(GetAsyncKeyState(i) & 0x8000) keydown(i);
	}
	switch(msg)
	{
	case WM_CREATE:{
			HDC hdc;
			BITMAPINFO bitmapinfo{};
			hdc = CreateCompatibleDC(0);
			bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bitmapinfo.bmiHeader.biWidth = width_px;
			bitmapinfo.bmiHeader.biHeight = -height_px; // top down is negative
			bitmapinfo.bmiHeader.biPlanes = 1;
			bitmapinfo.bmiHeader.biBitCount = 32;
			bitmapinfo.bmiHeader.biCompression = BI_RGB;
			bitmapinfo.bmiHeader.biClrUsed = 256;
			bitmapinfo.bmiHeader.biClrImportant = 256;
			bitmap = CreateDIBSection(hdc, &bitmapinfo, DIB_RGB_COLORS, (void**)&framebuf, 0, 0);
			pdc = CreateCompatibleDC(0);
			old = (HBITMAP)SelectObject(pdc, bitmap);
			DeleteDC(hdc);
			break;
	}
	case WM_SIZE:{
			if(!wm_size) break;
			width_px = LOWORD(l);
			height_px = HIWORD(l);
			resize();
			SelectObject(pdc, old);
			DeleteDC(pdc);
			DeleteObject(bitmap);
			HDC hdc;
			BITMAPINFO bitmapinfo{};
			hdc = CreateCompatibleDC(nullptr);
			bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bitmapinfo.bmiHeader.biWidth = width_px;
			bitmapinfo.bmiHeader.biHeight = -height_px; // top down is negative
			bitmapinfo.bmiHeader.biPlanes = 1;
			bitmapinfo.bmiHeader.biBitCount = 32;
			bitmapinfo.bmiHeader.biCompression = BI_RGB;
			bitmapinfo.bmiHeader.biClrUsed = 256;
			bitmapinfo.bmiHeader.biClrImportant = 256;
			bitmap = CreateDIBSection(hdc, &bitmapinfo, DIB_RGB_COLORS, (void**)(&framebuf), 0, 0);
			pdc = CreateCompatibleDC(0);
			old = (HBITMAP)SelectObject(pdc, bitmap);
			DeleteDC(hdc);
			break;
	}
		/*
		case WM_KEYDOWN: {
			keydown(w);
			break;
		}
		*/
		case WM_LBUTTONDOWN: {
				lbuttondown(l);
				break;
		}
		case WM_LBUTTONUP:{
			lbuttonup(l);
			break;
		}
	case WM_MOUSEMOVE:{
			mousemove(l);
			break;
	}
		case WM_PAINT:{
			PAINTSTRUCT ps;
			HDC h = BeginPaint(hwnd, &ps);

			//auto start = std::chrono::high_resolution_clock::now();

			render();

			//auto end = std::chrono::high_resolution_clock::now();
			//std::chrono::duration<double> diff = end - start;
			//std::cout << diff.count() << std::endl;

			BitBlt(h, 0, 0, width_px, height_px, pdc, 0, 0, SRCCOPY);
			EndPaint(hwnd, &ps);
			InvalidateRgn(hwnd, nullptr, 0);
			break;
		}
		case WM_DESTROY:
			SelectObject(pdc, old);
			DeleteDC(pdc);
			DeleteObject(bitmap);
			exit(0);
		default:
			{
				//for(const int& i : desired_keys){
				//	if(GetAsyncKeyState(i) & 0x8000) keydown(i);
				//}
				return DefWindowProc(hwnd, msg, w, l);
			}
	}
	return 0;
}
#define window_loop()\
{\
	MSG msg{};\
	while(true){\
		if(GetMessage(&msg, 0, 0, 0)){\
			if (msg.message == WM_QUIT) break;\
			TranslateMessage(&msg);\
			DispatchMessage(&msg);\
		}\
	}\
}
struct window{
	window(const char* name, int width, int height, WNDPROC func, bool resizable){
		wm_size = resizable;
		WNDCLASS wc{};
		wc.lpszClassName = name;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.lpfnWndProc = func;
		RegisterClass(&wc);
		int a = width + 16 - !resizable * 10, b = height + 39 - !resizable * 10;
		CreateWindow(name, name, (resizable ? WS_OVERLAPPEDWINDOW : WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME) | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, a, b, 0, 0, 0, 0);
	}
};