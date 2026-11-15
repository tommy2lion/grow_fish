#pragma once

#include "game.h"

void render_init(void);
void render_frame(const Game* game);
void render_button(int x, int y, int w, int h, const wchar_t* label, bool hover, bool enabled);
