#pragma once

/* easyx.h 默认把 WINVER 压到 0x0400，这里先提升到 Windows 10，  */
/* 以便使用 SetProcessDPIAware / ImmDisableIME 等较新的 API。      */
#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <math.h>
#include <time.h>
#include <windows.h>

/* 窗口与布局 */
#define WIN_W 1024
#define WIN_H 720

#define TOPBAR_H 60
#define GAME_X 0
#define GAME_Y TOPBAR_H
#define GAME_W 760
#define GAME_H (WIN_H - TOPBAR_H)

#define INFO_X GAME_W
#define INFO_Y 0
#define INFO_W (WIN_W - GAME_W)
#define INFO_H WIN_H

/* 按钮 */
#define BTN_W 120
#define BTN_H 40
#define BTN_Y 10
#define BTN_START_X 20
#define BTN_PAUSE_X 160
#define BTN_END_X 300

/* 游戏参数 */
#define TARGET_SCORE 20
#define INITIAL_TIME 60.0f
#define INITIAL_LIVES 3
#define MAX_FISH 24
#define MAX_RECORDS 10
#define MAX_NAME 32
#define MAX_POPUPS 24

typedef enum {
    FISH_SMALL,
    FISH_BIG,
    FISH_BOMB,
    FISH_CLOCK
} FishType;

typedef enum {
    STATE_LOGIN,
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_OVER
} GameState;

typedef enum {
    MODE_TIME,
    MODE_LIFE
} GameMode;

typedef struct Fish {
    FishType type;
    float x, y;
    float vx, vy;
    float halfW, halfH;
    int size;
    int score;
    int variant;
    int turnsLeft;
    bool justTurned;
    bool alive;
    struct Fish* next;
} Fish;

typedef struct {
    Fish* head;
    Fish* tail;
    int count;
} FishList;

typedef struct {
    wchar_t name[MAX_NAME];
    float x, y;
    float halfW, halfH;
    int size;
    int dir;            /* 1 向右，-1 向左 */
    int lives;
    int score;
    bool alive;
    DWORD invulnUntil;
} Player;

typedef struct {
    wchar_t name[MAX_NAME];
    int score;
} ScoreRecord;

typedef struct {
    float x, y;
    wchar_t text[32];
    COLORREF color;
    DWORD born;
} Popup;

static inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
