#include "input.h"

void input_init(Input* in) {
    memset(in, 0, sizeof(Input));
    in->mouseX = WIN_W / 2;
    in->mouseY = WIN_H / 2;
}

bool input_down(const Input* in, int vk) {
    return vk >= 0 && vk < 256 && in->down[vk];
}

bool input_pressed(const Input* in, int vk) {
    return vk >= 0 && vk < 256 && in->pressed[vk];
}

void input_update(Input* in) {
    memset(in->pressed, 0, sizeof(in->pressed));
    in->mouseClicked = false;
    in->charCount = 0;
    in->ctrlB = false;

    ExMessage msg;
    while (peekmessage(&msg, (BYTE)-1)) {
        switch (msg.message) {
        case WM_KEYDOWN:
            if (!in->down[msg.vkcode]) {
                in->pressed[msg.vkcode] = true;
            }
            if (msg.vkcode == 'B' &&
                (in->down[VK_CONTROL] || in->down[VK_LCONTROL] || in->down[VK_RCONTROL])) {
                in->ctrlB = true;
            }
            in->down[msg.vkcode] = true;
            break;

        case WM_KEYUP:
            in->down[msg.vkcode] = false;
            break;

        case WM_CHAR:
            if (in->charCount < (int)(sizeof(in->chars) / sizeof(in->chars[0]) - 1)) {
                in->chars[in->charCount++] = msg.ch;
            }
            break;

        case WM_MOUSEMOVE:
            in->mouseX = msg.x;
            in->mouseY = msg.y;
            break;

        case WM_LBUTTONDOWN:
            in->mouseDown = true;
            in->mouseClicked = true;
            in->mouseX = msg.x;
            in->mouseY = msg.y;
            break;

        case WM_LBUTTONUP:
            in->mouseDown = false;
            break;

        case WM_CLOSE:
            in->quit = true;
            break;

        default:
            break;
        }
    }
}
