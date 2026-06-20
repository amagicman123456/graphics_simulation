#pragma once

/*
    some functions essentially the same as image.hpp
*/
inline uint index_at(uint row_bytes, uint x, uint y)
{
    return y * row_bytes + 3 * x;
}
inline uint color_at(__global const uchar* data, uint row_bytes, uint x, uint y) {
    uint index = index_at(row_bytes, x, y);
    return (uint)data[index] << 16 |
           (uint)data[index + 1] << 8 |
           (uint)data[index + 2];
}