#pragma once

#include "common.h"
#include "input.h"

void player_init(Player* p, const wchar_t* name);
void player_update(Player* p, const Input* in, float dt);
void player_move_toward(Player* p, float tx, float ty, float dt);
void player_refresh_size(Player* p);
bool player_invulnerable(const Player* p);
