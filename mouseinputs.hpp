#pragma once
#include "utility.hpp"
extern int width_px, height_px;
bool mouse_down = false;

/*
    mouseinputs.hpp is for mouse inputs. shocking i know
*/

std::function<void(LPARAM)> mousemove = [](LPARAM l)
{
    static int last_x, last_y;
    static bool init = [&]()
    {
        last_x = GET_X_LPARAM(l);
        last_y = GET_Y_LPARAM(l);
        return true;
    }();
    int x = GET_X_LPARAM(l), y = GET_Y_LPARAM(l);
    int delta_x = x - last_x, delta_y = y - last_y;
    last_x = x, last_y = y;
    if (!mouse_down) return;

    /*
        rotate to local right
    */
    quaternion v = {0, 1, 0, 0};
    quaternion q_inv = inverse_quaternion(&current_rotation);
    quaternion temp = multiply_quaternions(&current_rotation, &v);
    quaternion rotated = multiply_quaternions(&temp, &q_inv);

    vector3 axis = {rotated.x, rotated.y, rotated.z};

    //up
    float angle = static_cast<float>(delta_y) * mouse_sensitivity;
    float new_pitch = current_pitch + angle;

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
    /*
    //down (unnecessary now)
    process_rotation(static_cast<float>(delta_y) * mouse_sensitivity,1,0,0);
    */
    //left (also unnecessary now)
    /*
    process_rotation(-(static_cast<float>(delta_x) * mouse_sensitivity),0,1,0);
    */
    //right
    process_rotation(-static_cast<float>(delta_x) * mouse_sensitivity, 0, 1, 0);
};
std::function<void(LPARAM)> lbuttondown = [](LPARAM l)
{
    mouse_down = true;
    int row = GET_X_LPARAM(l), col = GET_Y_LPARAM(l);
    int index = col * width_px + row;
    if (row < width_px || col < height_px){
        //todo next time for when a user clicks on an object!
    }
};
std::function<void(LPARAM)> lbuttonup = [](LPARAM l) {
    mouse_down = false; // so complicated i know
};