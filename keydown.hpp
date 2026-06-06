#pragma once
#include "utility.hpp"
#include <cmath>

/*
    keydown.hpp, for when a keydown event happens. don't mind the redundant code, copying and pasting is fun
*/

extern float mouse_sensitivity;
extern float key_sensitivity;
//extern float current_pitch;
inline int desired_keys[] = {VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, 'W', 'S', 'A', 'D'};

#undef M_PI
#define M_PI 3.14159265358979323846f
constexpr float pitch_limit = 89.0f * (M_PI / 180.0f);

std::function<void(WPARAM)> keydown = [](WPARAM w) {

    // REMEMBER: IF SOMETHINGS NOT WORKING ADD IT TO DESIRED KEYS

    float angle = mouse_sensitivity;

    quaternion v = {0, 1, 0, 0};
    quaternion q_inv = inverse_quaternion(&current_rotation);
    quaternion temp = multiply_quaternions(&current_rotation, &v);
    quaternion rotated = multiply_quaternions(&temp, &q_inv);

    vector3 axis = {rotated.x, rotated.y, rotated.z};

    switch(w){
        case VK_UP:
            {
                float new_pitch = current_pitch + mouse_sensitivity;

                if (new_pitch > pitch_limit) {
                    angle = pitch_limit - current_pitch;
                    current_pitch = pitch_limit;
                }
                else if (new_pitch < -pitch_limit) {
                    angle = -pitch_limit - current_pitch;
                    current_pitch = -pitch_limit;
                }
                else {
                    current_pitch = new_pitch;
                }
                quaternion pitch = quaternion_from_angle_and_axis(-angle, axis.x, axis.y, axis.z);
                current_rotation = multiply_quaternions(&pitch, &current_rotation);
                break;
            }
        case VK_DOWN:
            {
                float new_pitch = current_pitch - mouse_sensitivity;

                if (new_pitch > pitch_limit) {
                    angle = pitch_limit - current_pitch;
                    current_pitch = pitch_limit;
                }
                else if (new_pitch < -pitch_limit) {
                    angle = -pitch_limit - current_pitch;
                    current_pitch = -pitch_limit;
                }
                else {
                    current_pitch = new_pitch;
                }
                quaternion pitch = quaternion_from_angle_and_axis(angle, axis.x, axis.y, axis.z);
                current_rotation = multiply_quaternions(&pitch, &current_rotation);
                break;
            }
        case VK_LEFT:
            {
                process_rotation(-angle,0,1,0);
                break;
            }
        case VK_RIGHT:
            {
                process_rotation(angle,0,1,0);
                break;
            }
        case 'W':
			origin_x += key_sensitivity * current_rotation.x;
			origin_y += key_sensitivity * current_rotation.y;
			origin_z += key_sensitivity * current_rotation.z;
            break;
        case 'S':
			origin_x += -key_sensitivity * current_rotation.x;
			origin_y += -key_sensitivity * current_rotation.y;
			origin_z += -key_sensitivity * current_rotation.z;
            break;
        case 'A':
            break;
        case 'D':
            break;
        default:;
    }
};