#pragma once

#include "common.h"

void fishlist_init(FishList* list);
void fishlist_clear(FishList* list);
void fishlist_append(FishList* list, Fish* fish);
Fish* fishlist_detach(FishList* list, Fish* fish);
void fishlist_remove(FishList* list, Fish* fish);
Fish* fishlist_spawn(FishList* list, float gameW, float gameH);
void fishlist_update(FishList* list, float dt, float gameW, float gameH);
const wchar_t* fish_type_name(FishType type);
