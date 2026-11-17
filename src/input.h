#pragma once

#include "common.h"

typedef struct {
    bool down[256];
    bool pressed[256];
    bool mouseClicked;
    bool mouseDown;
    int mouseX;
    int mouseY;
    wchar_t chars[64];
    int charCount;
    bool ctrlB;     /* B 按下时 Ctrl 是否处于按下状态 */
    bool quit;
} Input;

void input_init(Input* in);
void input_update(Input* in);
bool input_down(const Input* in, int vk);
bool input_pressed(const Input* in, int vk);
