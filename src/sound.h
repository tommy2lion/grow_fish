#pragma once

#include "common.h"

void sound_init(void);
void sound_update(void);
void sound_shutdown(void);
void sound_play(const wchar_t* name);
void sound_set_enabled(bool enabled);
bool sound_is_enabled(void);
