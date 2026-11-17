#include "game.h"
#include "data.h"
#include "sound.h"

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

static void popup_add(Game* g, float x, float y, const wchar_t* text, COLORREF color) {
    if (g->popupCount >= MAX_POPUPS) {
        for (int i = 1; i < MAX_POPUPS; i++) {
            g->popups[i - 1] = g->popups[i];
        }
        g->popupCount = MAX_POPUPS - 1;
    }
    Popup* p = &g->popups[g->popupCount++];
    p->x = x;
    p->y = y;
    p->color = color;
    p->born = GetTickCount();
    wcsncpy_s(p->text, 32, text, _TRUNCATE);
}

static void popup_update(Game* g) {
    DWORD now = GetTickCount();
    int w = 0;
    for (int i = 0; i < g->popupCount; i++) {
        if (now - g->popups[i].born < 900) {
            g->popups[w++] = g->popups[i];
        }
    }
    g->popupCount = w;
}

static void consume_turns(Game* g, bool playSound) {
    for (Fish* f = g->fish.head; f; f = f->next) {
        if (f->justTurned) {
            if (playSound) {
                sound_play(L"turn.wav");
            }
            f->justTurned = false;
        }
    }
}

void game_init(Game* g, int viewW, int viewH) {
    memset(g, 0, sizeof(Game));
    g->viewW = viewW > 0 ? viewW : WIN_W;
    g->viewH = viewH > 0 ? viewH : WIN_H;
    g->state = STATE_LOGIN;
    g->mode = MODE_TIME;
    g->popupCount = 0;
    player_init(&g->player, L"Player");
    fishlist_init(&g->fish);
    fishlist_init(&g->eaten);
    g->timeLeft = INITIAL_TIME;
    g->targetScore = TARGET_SCORE;
    g->spawnTimer = 0.0f;
    g->animTime = 0.0f;
    g->win = false;
    g->recordSaved = false;
    g->bugMode = false;
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
    g->popupCount = 0;
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
    g->popupCount = 0;
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
    sound_play(win ? L"win.wav" : L"lose.wav");
    data_save_last_game(g->player.name, g->player.score, win, (int)g->mode);

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
    int mx = g->mouseX;
    int my = g->mouseY;

    if (hit_rect(mx, my, BTN_START_X, BTN_Y, BTN_W, BTN_H)) {
        if (g->state == STATE_MENU || g->state == STATE_PLAYING ||
            g->state == STATE_PAUSED || g->state == STATE_OVER) {
            sound_play(L"click.wav");
            game_start_round(g);
        }
    } else if (hit_rect(mx, my, BTN_PAUSE_X, BTN_Y, BTN_W, BTN_H)) {
        if (g->state == STATE_PLAYING) {
            sound_play(L"click.wav");
            g->state = STATE_PAUSED;
        } else if (g->state == STATE_PAUSED) {
            sound_play(L"click.wav");
            g->state = STATE_PLAYING;
        }
    } else if (hit_rect(mx, my, BTN_END_X, BTN_Y, BTN_W, BTN_H)) {
        if (g->state == STATE_PLAYING || g->state == STATE_PAUSED) {
            sound_play(L"click.wav");
            game_finish(g, false);
        } else if (g->state == STATE_OVER) {
            sound_play(L"click.wav");
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
                wchar_t buf[32];
                g->player.score += f->score;
                player_refresh_size(&g->player);
                sound_play(L"eat.wav");
                swprintf_s(buf, 32, L"+%d", f->score);
                popup_add(g, f->x, f->y, buf, RGB(255, 230, 120));
                Fish* eaten = fishlist_detach(&g->fish, f);
                if (eaten) {
                    eaten->alive = false;
                    fishlist_append(&g->eaten, eaten);
                }
                if (!g->bugMode && g->player.score >= g->targetScore) {
                    game_finish(g, true);
                    return;
                }
            } else if (f->type == FISH_BIG) {
                if (g->bugMode) {
                    /* BUG 模式：大鱼不造成伤害，玩家可以直接穿过 */
                } else if (!player_invulnerable(&g->player)) {
                    g->player.lives--;
                    g->player.invulnUntil = GetTickCount() + 1500;
                    sound_play(L"hit.wav");
                    popup_add(g, f->x, f->y, L"-1 生命", RGB(255, 130, 130));
                    if (g->player.lives <= 0) {
                        fishlist_remove(&g->fish, f);
                        game_finish(g, false);
                        return;
                    }
                }
                if (!g->bugMode) {
                    fishlist_remove(&g->fish, f);
                }
            } else if (f->type == FISH_BOMB) {
                sound_play(L"bomb.wav");
                if (g->bugMode) {
                    popup_add(g, f->x, f->y, L"BUG 免疫", RGB(255, 180, 120));
                } else if (g->mode == MODE_TIME) {
                    g->timeLeft -= 5.0f;
                    if (g->timeLeft < 0.0f) {
                        g->timeLeft = 0.0f;
                    }
                    popup_add(g, f->x, f->y, L"-5 s", RGB(255, 140, 90));
                } else {
                    g->player.lives--;
                    popup_add(g, f->x, f->y, L"-1 生命", RGB(255, 130, 130));
                }
                fishlist_remove(&g->fish, f);
                if (!g->bugMode && g->mode == MODE_LIFE && g->player.lives <= 0) {
                    game_finish(g, false);
                    return;
                }
            } else if (f->type == FISH_CLOCK) {
                sound_play(L"clock.wav");
                if (g->mode == MODE_TIME) {
                    g->timeLeft += 5.0f;
                    popup_add(g, f->x, f->y, L"+5 s", RGB(140, 240, 160));
                } else {
                    if (g->player.lives < 9) {
                        g->player.lives++;
                    }
                    popup_add(g, f->x, f->y, L"+1 生命", RGB(140, 240, 160));
                }
                fishlist_remove(&g->fish, f);
            }
        }
        f = next;
    }
}

void game_update(Game* g, Input* in, float dt) {
    g->mouseX = (int)((float)in->mouseX * (float)WIN_W / (float)g->viewW);
    g->mouseY = (int)((float)in->mouseY * (float)WIN_H / (float)g->viewH);
    g->mouseX = (int)clampf((float)g->mouseX, 0.0f, (float)(WIN_W - 1));
    g->mouseY = (int)clampf((float)g->mouseY, 0.0f, (float)(WIN_H - 1));
    g->animTime += dt;

    if (g->state == STATE_LOGIN) {
        game_process_login(g, in);
        return;
    }

    if (input_pressed(in, 'S')) {
        bool on = !sound_is_enabled();
        sound_set_enabled(on);
        if (on) {
            sound_play(L"click.wav");
        }
    }

    if (in->ctrlB) {
        g->bugMode = !g->bugMode;
        sound_play(L"click.wav");
        wchar_t msg[32];
        swprintf_s(msg, 32, g->bugMode ? L"BUG 模式：开" : L"BUG 模式：关");
        popup_add(g, g->player.x, g->player.y - 30.0f, msg,
                  g->bugMode ? RGB(255, 120, 120) : RGB(190, 210, 230));
    }

    game_handle_buttons(g, in);

    switch (g->state) {
    case STATE_MENU:
        fishlist_update(&g->fish, dt, GAME_W, GAME_H);
        consume_turns(g, false);
        while (g->fish.count < 12) {
            if (!fishlist_spawn(&g->fish, GAME_W, GAME_H)) {
                break;
            }
        }
        if (input_pressed(in, 'M')) {
            g->mode = (g->mode == MODE_TIME) ? MODE_LIFE : MODE_TIME;
            sound_play(L"click.wav");
        }
        if (input_pressed(in, VK_RETURN)) {
            sound_play(L"click.wav");
            game_start_round(g);
        }
        if (input_pressed(in, VK_ESCAPE)) {
            in->quit = true;
        }
        break;

    case STATE_PLAYING:
        if (input_pressed(in, 'P')) {
            sound_play(L"click.wav");
            g->state = STATE_PAUSED;
            break;
        }
        if (input_pressed(in, VK_ESCAPE)) {
            game_finish(g, false);
            break;
        }
        player_update(&g->player, in, dt);
        if (in->mouseDown && g->mouseX < GAME_W && g->mouseY >= GAME_Y) {
            player_move_toward(&g->player, (float)g->mouseX,
                               (float)(g->mouseY - GAME_Y), dt);
        }
        fishlist_update(&g->fish, dt, GAME_W, GAME_H);
        consume_turns(g, true);
        g->spawnTimer -= dt;
        if (g->spawnTimer <= 0.0f && g->fish.count < MAX_FISH) {
            fishlist_spawn(&g->fish, GAME_W, GAME_H);
            g->spawnTimer = 0.35f + (float)(rand() % 60) / 100.0f;
        }
        game_check_collisions(g);
        if (g->state != STATE_PLAYING) {
            break;
        }
        if (g->mode == MODE_TIME && !g->bugMode) {
            g->timeLeft -= dt;
            if (g->timeLeft <= 0.0f) {
                g->timeLeft = 0.0f;
                game_finish(g, g->player.score >= g->targetScore);
            }
        }
        break;

    case STATE_PAUSED:
        if (input_pressed(in, 'P') || input_pressed(in, VK_RETURN)) {
            sound_play(L"click.wav");
            g->state = STATE_PLAYING;
        } else if (input_pressed(in, VK_ESCAPE)) {
            game_finish(g, false);
        }
        break;

    case STATE_OVER:
        fishlist_update(&g->fish, dt, GAME_W, GAME_H);
        consume_turns(g, false);
        while (g->fish.count < 8) {
            if (!fishlist_spawn(&g->fish, GAME_W, GAME_H)) {
                break;
            }
        }
        if (input_pressed(in, VK_RETURN)) {
            sound_play(L"click.wav");
            game_start_round(g);
        } else if (input_pressed(in, VK_ESCAPE)) {
            sound_play(L"click.wav");
            game_to_menu(g);
        }
        break;

    default:
        break;
    }

    popup_update(g);
}
