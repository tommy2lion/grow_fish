#include "game.h"
#include "data.h"

static void game_reload_records(Game* g) {
    data_load(g->records, &g->recordCount);
    g->highScore = data_high_score(g->records, g->recordCount);
}

static void game_spawn_decor(Game* g) {
    fishlist_clear(&g->fish);
    while (g->fish.count < 12) {
        if (!fishlist_spawn(&g->fish, GAME_W, GAME_H)) {
            break;
        }
    }
}

void game_init(Game* g) {
    memset(g, 0, sizeof(Game));
    g->state = STATE_LOGIN;
    player_init(&g->player, L"Player");
    fishlist_init(&g->fish);
    fishlist_init(&g->eaten);
    g->timeLeft = INITIAL_TIME;
    g->targetScore = TARGET_SCORE;
    g->spawnTimer = 0.0f;
    g->animTime = 0.0f;
    g->win = false;
    g->recordSaved = false;
    g->loginName[0] = 0;
    g->loginLen = 0;
    g->loginMessage[0] = 0;
    g->mouseX = WIN_W / 2;
    g->mouseY = WIN_H / 2;
    game_reload_records(g);
}

void game_start_round(Game* g) {
    fishlist_clear(&g->fish);
    fishlist_clear(&g->eaten);
    player_init(&g->player, g->loginName[0] ? g->loginName : L"Player");
    g->timeLeft = INITIAL_TIME;
    g->targetScore = TARGET_SCORE;
    g->spawnTimer = 0.0f;
    g->win = false;
    g->recordSaved = false;
    g->state = STATE_PLAYING;
    while (g->fish.count < 10) {
        if (!fishlist_spawn(&g->fish, GAME_W, GAME_H)) {
            break;
        }
    }
}

void game_to_menu(Game* g) {
    g->state = STATE_MENU;
    g->win = false;
    g->recordSaved = false;
    g->timeLeft = INITIAL_TIME;
    fishlist_clear(&g->eaten);
    player_init(&g->player, g->loginName[0] ? g->loginName : L"Player");
    game_spawn_decor(g);
}

void game_finish(Game* g, bool win) {
    if (g->state != STATE_PLAYING && g->state != STATE_PAUSED) {
        return;
    }
    g->win = win;
    g->state = STATE_OVER;

    if (!g->recordSaved && g->player.score > 0) {
        data_save(g->records, &g->recordCount, g->player.name, g->player.score);
        g->highScore = data_high_score(g->records, g->recordCount);
        g->recordSaved = true;
    }
}

static void game_process_login(Game* g, Input* in) {
    if (input_pressed(in, VK_ESCAPE)) {
        in->quit = true;
        return;
    }
    for (int i = 0; i < in->charCount; i++) {
        wchar_t ch = in->chars[i];
        if (ch == L'\r') {
            if (g->loginLen > 0) {
                g->loginName[g->loginLen] = 0;
                wcsncpy_s(g->player.name, MAX_NAME, g->loginName, _TRUNCATE);
                g->state = STATE_MENU;
                g->loginMessage[0] = 0;
                game_spawn_decor(g);
            } else {
                wcsncpy_s(g->loginMessage, 64, L"请输入玩家名", _TRUNCATE);
            }
        } else if (ch == L'\b') {
            if (g->loginLen > 0) {
                g->loginLen--;
            }
        } else if (ch >= 32 && ch < 127 && g->loginLen < MAX_NAME - 1) {
            g->loginName[g->loginLen++] = ch;
        }
        g->loginName[g->loginLen] = 0;
    }
}

static bool hit_rect(int mx, int my, int x, int y, int w, int h) {
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

static void game_handle_buttons(Game* g, Input* in) {
    if (!in->mouseClicked) {
        return;
    }
    int mx = in->mouseX;
    int my = in->mouseY;

    if (hit_rect(mx, my, BTN_START_X, BTN_Y, BTN_W, BTN_H)) {
        if (g->state == STATE_MENU || g->state == STATE_PLAYING ||
            g->state == STATE_PAUSED || g->state == STATE_OVER) {
            game_start_round(g);
        }
    } else if (hit_rect(mx, my, BTN_PAUSE_X, BTN_Y, BTN_W, BTN_H)) {
        if (g->state == STATE_PLAYING) {
            g->state = STATE_PAUSED;
        } else if (g->state == STATE_PAUSED) {
            g->state = STATE_PLAYING;
        }
    } else if (hit_rect(mx, my, BTN_END_X, BTN_Y, BTN_W, BTN_H)) {
        if (g->state == STATE_PLAYING || g->state == STATE_PAUSED) {
            game_finish(g, false);
        } else if (g->state == STATE_OVER) {
            game_to_menu(g);
        }
    }
}

static bool overlap(float ax, float ay, float ahw, float ahh,
                    float bx, float by, float bhw, float bhh) {
    return fabsf(ax - bx) < (ahw + bhw) && fabsf(ay - by) < (ahh + bhh);
}

static void game_check_collisions(Game* g) {
    Fish* f = g->fish.head;
    while (f) {
        Fish* next = f->next;
        if (overlap(g->player.x, g->player.y, g->player.halfW, g->player.halfH,
                    f->x, f->y, f->halfW, f->halfH)) {
            if (f->type == FISH_SMALL) {
                g->player.score += f->score;
                player_refresh_size(&g->player);
                Fish* eaten = fishlist_detach(&g->fish, f);
                if (eaten) {
                    eaten->alive = false;
                    fishlist_append(&g->eaten, eaten);
                }
                if (g->player.score >= g->targetScore) {
                    game_finish(g, true);
                    return;
                }
            } else if (f->type == FISH_BIG) {
                if (!player_invulnerable(&g->player)) {
                    g->player.lives--;
                    g->player.invulnUntil = GetTickCount() + 1500;
                    if (g->player.lives <= 0) {
                        game_finish(g, false);
                        return;
                    }
                }
                fishlist_remove(&g->fish, f);
            } else if (f->type == FISH_BOMB) {
                g->timeLeft -= 5.0f;
                if (g->timeLeft < 0.0f) {
                    g->timeLeft = 0.0f;
                }
                fishlist_remove(&g->fish, f);
            } else if (f->type == FISH_CLOCK) {
                g->timeLeft += 5.0f;
                fishlist_remove(&g->fish, f);
            }
        }
        f = next;
    }
}

void game_update(Game* g, Input* in, float dt) {
    g->mouseX = in->mouseX;
    g->mouseY = in->mouseY;
    g->animTime += dt;

    if (g->state == STATE_LOGIN) {
        game_process_login(g, in);
        return;
    }

    game_handle_buttons(g, in);

    switch (g->state) {
    case STATE_MENU:
        fishlist_update(&g->fish, dt, GAME_W, GAME_H);
        while (g->fish.count < 12) {
            if (!fishlist_spawn(&g->fish, GAME_W, GAME_H)) {
                break;
            }
        }
        if (input_pressed(in, VK_RETURN)) {
            game_start_round(g);
        }
        if (input_pressed(in, VK_ESCAPE)) {
            in->quit = true;
        }
        break;

    case STATE_PLAYING:
        if (input_pressed(in, 'P')) {
            g->state = STATE_PAUSED;
            break;
        }
        if (input_pressed(in, VK_ESCAPE)) {
            game_finish(g, false);
            break;
        }
        player_update(&g->player, in, dt);
        fishlist_update(&g->fish, dt, GAME_W, GAME_H);
        g->spawnTimer -= dt;
        if (g->spawnTimer <= 0.0f && g->fish.count < MAX_FISH) {
            fishlist_spawn(&g->fish, GAME_W, GAME_H);
            g->spawnTimer = 0.35f + (float)(rand() % 60) / 100.0f;
        }
        game_check_collisions(g);
        if (g->state != STATE_PLAYING) {
            break;
        }
        g->timeLeft -= dt;
        if (g->timeLeft <= 0.0f) {
            g->timeLeft = 0.0f;
            game_finish(g, g->player.score >= g->targetScore);
        }
        break;

    case STATE_PAUSED:
        if (input_pressed(in, 'P') || input_pressed(in, VK_RETURN)) {
            g->state = STATE_PLAYING;
        } else if (input_pressed(in, VK_ESCAPE)) {
            game_finish(g, false);
        }
        break;

    case STATE_OVER:
        fishlist_update(&g->fish, dt, GAME_W, GAME_H);
        while (g->fish.count < 8) {
            if (!fishlist_spawn(&g->fish, GAME_W, GAME_H)) {
                break;
            }
        }
        if (input_pressed(in, VK_RETURN)) {
            game_start_round(g);
        } else if (input_pressed(in, VK_ESCAPE)) {
            game_to_menu(g);
        }
        break;

    default:
        break;
    }
}
