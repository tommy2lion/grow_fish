#pragma once

#include "common.h"

typedef enum {
    SPR_PLAYER,
    SPR_SMALL0,
    SPR_SMALL1,
    SPR_SMALL2,
    SPR_BIG0,
    SPR_BIG1,
    SPR_COUNT
    /* SPR_BIG0 = 河豚，SPR_BIG1 = 红色大鱼 */
} SpriteId;

typedef enum {
    BG_SEAWEED0,
    BG_SEAWEED1,
    BG_SEAWEED2,
    BG_ROCK0,
    BG_ROCK1,
    BG_BUBBLE0,
    BG_BUBBLE1,
    BG_BUBBLE2,
    BG_COUNT
} BackgroundSprite;

void assets_init(void);
bool assets_ok(void);
IMAGE* assets_get(SpriteId id, bool facingLeft);
IMAGE* assets_bg(BackgroundSprite id);
bool assets_find(const wchar_t* rel, wchar_t* out, int cap);
