#include "player.h"

void player_refresh_size(Player* p) {
    int size = 1 + p->score / 5;
    if (size > 5) {
        size = 5;
    }
    p->size = size;
    p->halfW = 22.0f + (float)(size - 1) * 5.0f;
    p->halfH = p->halfW * 0.62f;
}

void player_init(Player* p, const wchar_t* name) {
    memset(p, 0, sizeof(Player));
    wcsncpy_s(p->name, MAX_NAME, (name && name[0]) ? name : L"Player", _TRUNCATE);
    p->x = GAME_W * 0.5f;
    p->y = GAME_H * 0.5f;
    p->size = 1;
    p->dir = 1;
    p->halfW = 22.0f;
    p->halfH = p->halfW * 0.62f;
    p->lives = INITIAL_LIVES;
    p->score = 0;
    p->alive = true;
    p->invulnUntil = 0;
}

void player_update(Player* p, const Input* in, float dt) {
    const float speed = 330.0f;
    float dx = 0.0f;
    float dy = 0.0f;

    if (input_down(in, VK_LEFT) || input_down(in, 'A')) {
        dx -= 1.0f;
    }
    if (input_down(in, VK_RIGHT) || input_down(in, 'D')) {
        dx += 1.0f;
    }
    if (input_down(in, VK_UP) || input_down(in, 'W')) {
        dy -= 1.0f;
    }
    if (input_down(in, VK_DOWN) || input_down(in, 'S')) {
        dy += 1.0f;
    }

    if (dx != 0.0f && dy != 0.0f) {
        dx *= 0.7071f;
        dy *= 0.7071f;
    }
    if (dx < 0.0f) {
        p->dir = -1;
    } else if (dx > 0.0f) {
        p->dir = 1;
    }

    p->x += dx * speed * dt;
    p->y += dy * speed * dt;
    p->x = clampf(p->x, p->halfW + 4.0f, GAME_W - p->halfW - 4.0f);
    p->y = clampf(p->y, p->halfH + 4.0f, GAME_H - p->halfH - 4.0f);
}

bool player_invulnerable(const Player* p) {
    return GetTickCount() < p->invulnUntil;
}
