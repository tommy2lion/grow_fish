#pragma once

#include "common.h"
#include "fish.h"
#include "player.h"
#include "input.h"

typedef struct Game {
    GameState state;
    Player player;
    FishList fish;
    FishList eaten;
    float timeLeft;
    int targetScore;
    int highScore;
    ScoreRecord records[MAX_RECORDS];
    int recordCount;
    float spawnTimer;
    float animTime;
    bool win;
    bool recordSaved;

    wchar_t loginName[MAX_NAME];
    int loginLen;
    wchar_t loginMessage[64];

    int mouseX;
    int mouseY;
} Game;

void game_init(Game* g);
void game_update(Game* g, Input* in, float dt);
void game_start_round(Game* g);
void game_to_menu(Game* g);
void game_finish(Game* g, bool win);
