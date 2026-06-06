#include <iostream>
#include "../image.hpp"
#include <windows.h>
std::string ColorRefToHexString(COLORREF color) {
    // Extract the R, G, B components using Windows macros
    int red = GetRValue(color);
    int green = GetGValue(color);
    int blue = GetBValue(color);

    // Buffer to hold the resulting hex string (including '#' and null terminator)
    char hexColor[8];

    // Format the components into a #RRGGBB string
    // %02X ensures each component is a 2-digit uppercase hex value, padded with a leading zero if necessary
    std::sprintf(hexColor, "#%02X%02X%02X", red, green, blue);

    return std::string(hexColor);
}
int main() {
    image i = read_rgb_image("pixel_beach.bmp");
    std::cout << "[\n";
    for (int y = 0; y < i.height; y++) {
        for (int x = 0; x < i.width; x++) {
            std::cout << "\"" << ColorRefToHexString(i.color_at(x, y)) << "\"" << ',';
        }
    }
    std::cout << "];";
}