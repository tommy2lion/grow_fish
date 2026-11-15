#include "render.h"

static const COLORREF C_TEXT = RGB(230, 242, 255);
static const COLORREF C_ACCENT = RGB(255, 205, 80);

static IMAGE g_canvas;
static IMAGE g_screen;
static int g_viewW = WIN_W;
static int g_viewH = WIN_H;

static COLORREF lerp_color(COLORREF a, COLORREF b, float t) {
    t = clampf(t, 0.0f, 1.0f);
    int r = (int)(GetRValue(a) + (GetRValue(b) - GetRValue(a)) * t);
    int g = (int)(GetGValue(a) + (GetGValue(b) - GetGValue(a)) * t);
    int bl = (int)(GetBValue(a) + (GetBValue(b) - GetBValue(a)) * t);
    return RGB(r, g, bl);
}

static void set_font(int size, bool bold) {
    settextstyle(size, 0, L"Microsoft YaHei", 0, 0,
                 bold ? FW_BOLD : FW_NORMAL, false, false, false);
}

static void text_at(int x, int y, const wchar_t* s, int size, COLORREF color, bool bold) {
    set_font(size, bold);
    setbkmode(TRANSPARENT);
    settextcolor(color);
    outtextxy(x, y, s);
}

static void text_center(int cx, int y, const wchar_t* s, int size, COLORREF color, bool bold) {
    set_font(size, bold);
    setbkmode(TRANSPARENT);
    settextcolor(color);
    outtextxy(cx - textwidth(s) / 2, y, s);
}

void render_init(int screenW, int screenH) {
    g_viewW = screenW > 0 ? screenW : WIN_W;
    g_viewH = screenH > 0 ? screenH : WIN_H;
    g_canvas.Resize(WIN_W, WIN_H);
    g_screen.Resize(g_viewW, g_viewH);
    SetWorkingImage(&g_canvas);
    setbkcolor(RGB(6, 22, 42));
    cleardevice();
    setbkmode(TRANSPARENT);
    set_font(20, false);
}

void render_present(void) {
    HDC screenDC = GetImageHDC(&g_screen);
    HDC canvasDC = GetImageHDC(&g_canvas);
    if (screenDC && canvasDC) {
        SetStretchBltMode(screenDC, HALFTONE);
        SetBrushOrgEx(screenDC, 0, 0, NULL);
        StretchBlt(screenDC, 0, 0, g_viewW, g_viewH,
                   canvasDC, 0, 0, WIN_W, WIN_H, SRCCOPY);
    }

    SetWorkingImage(NULL);
    putimage(0, 0, &g_screen);
    SetWorkingImage(&g_canvas);
}

void render_button(int x, int y, int w, int h, const wchar_t* label, bool hover, bool enabled) {
    COLORREF fill;
    COLORREF border;
    COLORREF textColor;

    if (!enabled) {
        fill = RGB(48, 58, 68);
        border = RGB(70, 80, 90);
        textColor = RGB(140, 150, 160);
    } else if (hover) {
        fill = RGB(52, 132, 190);
        border = RGB(130, 200, 245);
        textColor = RGB(250, 253, 255);
    } else {
        fill = RGB(28, 86, 135);
        border = RGB(85, 150, 200);
        textColor = RGB(235, 245, 255);
    }

    setfillcolor(fill);
    solidroundrect(x, y, x + w, y + h, 10, 10);
    setlinecolor(border);
    roundrect(x, y, x + w, y + h, 10, 10);

    set_font(18, true);
    setbkmode(TRANSPARENT);
    settextcolor(textColor);
    int tw = textwidth(label);
    int th = textheight(label);
    outtextxy(x + (w - tw) / 2, y + (h - th) / 2, label);
}

static void render_game_background(const Game* g) {
    const int band = 8;
    for (int y = 0; y < GAME_H; y += band) {
        float t = (float)y / (float)GAME_H;
        setfillcolor(lerp_color(RGB(20, 78, 130), RGB(7, 28, 55), t));
        solidrectangle(GAME_X, GAME_Y + y, GAME_X + GAME_W, GAME_Y + y + band);
    }

    /* 沙地 */
    setfillcolor(RGB(188, 154, 92));
    solidrectangle(GAME_X, GAME_Y + GAME_H - 46, GAME_X + GAME_W, GAME_Y + GAME_H);
    setfillcolor(RGB(210, 178, 112));
    for (int i = 0; i < 6; i++) {
        int cx = 60 + i * 130;
        solidellipse(cx - 90, GAME_Y + GAME_H - 54, cx + 90, GAME_Y + GAME_H - 30);
    }

    /* 气泡 */
    setlinecolor(RGB(70, 160, 220));
    for (int i = 0; i < 24; i++) {
        int bx = (i * 97 + (int)(g->animTime * (float)(10 + i % 5) * 2.0f)) % GAME_W;
        int by = GAME_Y + 40 + (i * 61) % (GAME_H - 100);
        int r = 2 + i % 4;
        circle(GAME_X + bx, by, r);
    }

    /* 水草 */
    setlinecolor(RGB(28, 120, 90));
    for (int i = 0; i < 7; i++) {
        int baseX = 40 + i * 105;
        int baseY = GAME_Y + GAME_H - 44;
        for (int k = 0; k < 5; k++) {
            int y1 = baseY - k * 16;
            int x1 = baseX + (int)(sinf(g->animTime * 1.6f + (float)i + (float)k * 0.5f) * 6.0f);
            int y2 = baseY - (k + 1) * 16;
            int x2 = baseX + (int)(sinf(g->animTime * 1.6f + (float)i + (float)(k + 1) * 0.5f) * 6.0f);
            line(x1, y1, x2, y2);
        }
    }

    setlinecolor(RGB(55, 115, 165));
    rectangle(GAME_X, GAME_Y, GAME_X + GAME_W, GAME_Y + GAME_H);
}

static void render_fish(const Fish* f) {
    int x = GAME_X + (int)f->x;
    int y = GAME_Y + (int)f->y;
    int hw = (int)f->halfW;
    int hh = (int)f->halfH;
    int dir = f->vx >= 0.0f ? 1 : -1;

    if (f->type == FISH_BOMB) {
        setfillcolor(RGB(45, 45, 55));
        setlinecolor(RGB(20, 20, 25));
        solidcircle(x, y, hw);
        setlinecolor(RGB(210, 180, 120));
        line(x + 2, y - hh, x + hw / 2, y - hh - 10);
        setfillcolor(RGB(255, 190, 60));
        setlinecolor(RGB(255, 190, 60));
        solidcircle(x + hw / 2, y - hh - 12, 5);
        return;
    }

    if (f->type == FISH_CLOCK) {
        setfillcolor(RGB(245, 200, 70));
        setlinecolor(RGB(190, 140, 30));
        solidcircle(x, y, hw);
        setfillcolor(RGB(255, 252, 235));
        setlinecolor(RGB(255, 252, 235));
        solidcircle(x, y, hw * 3 / 4);
        setlinecolor(RGB(90, 60, 20));
        line(x, y, x, y - hw / 2);
        line(x, y, x + hw / 2, y + hw / 4);
        return;
    }

    COLORREF body = (f->type == FISH_SMALL) ? RGB(255, 140, 60) : RGB(125, 70, 175);
    COLORREF fin = (f->type == FISH_SMALL) ? RGB(220, 100, 35) : RGB(95, 50, 135);
    COLORREF outline = (f->type == FISH_SMALL) ? RGB(180, 75, 25) : RGB(70, 35, 100);

    setfillcolor(body);
    setlinecolor(body);
    solidellipse(x - hw, y - hh, x + hw, y + hh);

    POINT tail[3] = {
        { x - dir * hw, y },
        { x - dir * (hw + hw / 2), y - hh },
        { x - dir * (hw + hw / 2), y + hh }
    };
    setfillcolor(fin);
    setlinecolor(fin);
    solidpolygon(tail, 3);

    POINT topfin[3] = {
        { x - dir * hw / 3, y - hh },
        { x + dir * hw / 3, y - hh },
        { x + dir * hw / 6, y - hh - hh / 2 }
    };
    solidpolygon(topfin, 3);

    int ex = x + dir * hw * 3 / 5;
    int ey = y - hh / 3;
    int er = hw / 6;
    if (er < 2) {
        er = 2;
    }
    int pr = er * 2 / 3;
    if (pr < 1) {
        pr = 1;
    }
    setfillcolor(RGB(255, 255, 255));
    setlinecolor(RGB(255, 255, 255));
    solidcircle(ex, ey, er);
    setfillcolor(RGB(20, 20, 25));
    setlinecolor(RGB(20, 20, 25));
    solidcircle(ex + dir * er / 3, ey, pr);

    setlinecolor(outline);
    ellipse(x - hw, y - hh, x + hw, y + hh);
}

static void render_fish_list(const FishList* list) {
    for (const Fish* f = list->head; f; f = f->next) {
        render_fish(f);
    }
}

static void render_player(const Game* g) {
    const Player* p = &g->player;
    int x = GAME_X + (int)p->x;
    int y = GAME_Y + (int)p->y;
    int hw = (int)p->halfW;
    int hh = (int)p->halfH;
    int dir = p->dir;

    bool inv = player_invulnerable(p);
    bool blink = inv && ((GetTickCount() / 100) % 2 == 0);

    if (!blink) {
        setfillcolor(RGB(60, 190, 150));
        setlinecolor(RGB(60, 190, 150));
        solidellipse(x - hw, y - hh, x + hw, y + hh);

        POINT tail[3] = {
            { x - dir * hw, y },
            { x - dir * (hw + hw / 2), y - hh },
            { x - dir * (hw + hw / 2), y + hh }
        };
        setfillcolor(RGB(35, 145, 110));
        setlinecolor(RGB(35, 145, 110));
        solidpolygon(tail, 3);

        POINT topfin[3] = {
            { x - dir * hw / 3, y - hh },
            { x + dir * hw / 3, y - hh },
            { x + dir * hw / 6, y - hh - hh / 2 }
        };
        solidpolygon(topfin, 3);

        int ex = x + dir * hw * 3 / 5;
        int ey = y - hh / 3;
        int er = hw / 6;
        if (er < 2) {
            er = 2;
        }
        int pr = er * 2 / 3;
        if (pr < 1) {
            pr = 1;
        }
        setfillcolor(RGB(255, 255, 255));
        setlinecolor(RGB(255, 255, 255));
        solidcircle(ex, ey, er);
        setfillcolor(RGB(20, 20, 25));
        setlinecolor(RGB(20, 20, 25));
        solidcircle(ex + dir * er / 3, ey, pr);

        setlinecolor(RGB(20, 100, 75));
        ellipse(x - hw, y - hh, x + hw, y + hh);
    }

    if (inv) {
        setlinecolor(RGB(255, 230, 100));
        circle(x, y, hw + 8);
    }
}

static bool hover_btn(const Game* g, int x) {
    return g->mouseX >= x && g->mouseX <= x + BTN_W &&
           g->mouseY >= BTN_Y && g->mouseY <= BTN_Y + BTN_H;
}

static void render_topbar(const Game* g) {
    setfillcolor(RGB(8, 24, 44));
    solidrectangle(0, 0, GAME_W, TOPBAR_H);
    setlinecolor(RGB(50, 105, 155));
    line(0, TOPBAR_H - 1, GAME_W, TOPBAR_H - 1);

    const wchar_t* startLabel = L"开始游戏";
    if (g->state == STATE_PLAYING || g->state == STATE_PAUSED) {
        startLabel = L"重新开始";
    } else if (g->state == STATE_OVER) {
        startLabel = L"再玩一次";
    }
    const wchar_t* pauseLabel = (g->state == STATE_PAUSED) ? L"继续" : L"暂停";
    const wchar_t* endLabel = (g->state == STATE_OVER) ? L"返回菜单" : L"结束";

    bool pauseEnabled = (g->state == STATE_PLAYING || g->state == STATE_PAUSED);
    bool endEnabled = (g->state == STATE_PLAYING || g->state == STATE_PAUSED ||
                       g->state == STATE_OVER);

    render_button(BTN_START_X, BTN_Y, BTN_W, BTN_H, startLabel,
                  hover_btn(g, BTN_START_X), true);
    render_button(BTN_PAUSE_X, BTN_Y, BTN_W, BTN_H, pauseLabel,
                  hover_btn(g, BTN_PAUSE_X) && pauseEnabled, pauseEnabled);
    render_button(BTN_END_X, BTN_Y, BTN_W, BTN_H, endLabel,
                  hover_btn(g, BTN_END_X) && endEnabled, endEnabled);

    text_at(GAME_W - 250, 14, L"大鱼吃小鱼", 26, C_ACCENT, true);
}

static void render_info_panel(const Game* g) {
    setfillcolor(RGB(10, 30, 52));
    solidrectangle(INFO_X, 0, WIN_W, WIN_H);
    setlinecolor(RGB(60, 120, 170));
    line(INFO_X, 0, INFO_X, WIN_H);

    int x = INFO_X + 20;
    int y = 18;
    wchar_t buf[128];

    text_at(x, y, L"游戏信息", 24, C_ACCENT, true);
    y += 44;

    swprintf_s(buf, 128, L"玩家：%ls", g->player.name[0] ? g->player.name : L"-");
    text_at(x, y, buf, 17, C_TEXT, false);
    y += 30;

    swprintf_s(buf, 128, L"最高分：%d", g->highScore);
    text_at(x, y, buf, 17, C_TEXT, false);
    y += 30;

    swprintf_s(buf, 128, L"本局得分：%d / %d", g->player.score, g->targetScore);
    text_at(x, y, buf, 17, C_TEXT, false);
    y += 30;

    swprintf_s(buf, 128, L"已吃鱼：%d", g->eaten.count);
    text_at(x, y, buf, 17, C_TEXT, false);
    y += 30;

    swprintf_s(buf, 128, L"剩余时间：%.1f s", g->timeLeft);
    text_at(x, y, buf, 17, C_TEXT, false);
    y += 26;

    int barX = x;
    int barY = y;
    int barW = INFO_W - 40;
    int barH = 10;
    setfillcolor(RGB(28, 48, 68));
    solidrectangle(barX, barY, barX + barW, barY + barH);
    float t = clampf(g->timeLeft / INITIAL_TIME, 0.0f, 1.0f);
    COLORREF barColor = RGB(90, 210, 130);
    if (t <= 0.5f) {
        barColor = RGB(245, 200, 70);
    }
    if (t <= 0.2f) {
        barColor = RGB(240, 90, 90);
    }
    setfillcolor(barColor);
    solidrectangle(barX, barY, barX + (int)(barW * t), barY + barH);
    y += 24;

    swprintf_s(buf, 128, L"生命：%d", g->player.lives);
    text_at(x, y, buf, 17, C_TEXT, false);
    int lx = x + 76;
    for (int i = 0; i < g->player.lives && i < 8; i++) {
        setfillcolor(RGB(240, 90, 90));
        setlinecolor(RGB(240, 90, 90));
        solidcircle(lx + i * 16, y + 9, 6);
    }
    y += 42;

    text_at(x, y, L"排行榜", 20, C_ACCENT, true);
    y += 30;
    int top = g->recordCount < 3 ? g->recordCount : 3;
    for (int i = 0; i < top; i++) {
        swprintf_s(buf, 128, L"%d. %ls  %d", i + 1, g->records[i].name, g->records[i].score);
        text_at(x, y, buf, 15, RGB(200, 220, 240), false);
        y += 22;
    }

    text_at(x, WIN_H - 92, L"操作：", 16, RGB(160, 190, 220), true);
    text_at(x, WIN_H - 68, L"方向键 / WASD 移动", 15, RGB(180, 205, 230), false);
    text_at(x, WIN_H - 46, L"P 暂停    ESC 结束", 15, RGB(180, 205, 230), false);
}

static void render_overlay_panel(int x, int y, int w, int h) {
    setfillcolor(RGB(8, 28, 50));
    solidroundrect(x, y, x + w, y + h, 16, 16);
    setlinecolor(RGB(90, 170, 220));
    roundrect(x, y, x + w, y + h, 16, 16);
}

static void render_menu_overlay(void) {
    int x = 155;
    int y = 250;
    int w = 450;
    int h = 170;
    render_overlay_panel(x, y, w, h);
    text_center(GAME_W / 2, y + 24, L"准备开始", 30, C_ACCENT, true);
    text_center(GAME_W / 2, y + 68, L"吃掉小鱼，躲避大鱼和炸弹", 18, C_TEXT, false);
    text_center(GAME_W / 2, y + 98, L"闹钟加时间，炸弹减时间", 18, C_TEXT, false);
    text_center(GAME_W / 2, y + 130, L"点击【开始游戏】或按 Enter", 18, RGB(150, 220, 255), true);
}

static void render_pause_overlay(const Game* g) {
    int x = 220;
    int y = 280;
    int w = 320;
    int h = 150;
    render_overlay_panel(x, y, w, h);
    text_center(GAME_W / 2, y + 22, L"已暂停", 30, C_ACCENT, true);
    text_center(GAME_W / 2, y + 70, L"按 P 或点击【继续】恢复", 18, C_TEXT, false);
    wchar_t buf[64];
    swprintf_s(buf, 64, L"当前得分：%d", g->player.score);
    text_center(GAME_W / 2, y + 104, buf, 18, RGB(150, 220, 255), false);
}

static void render_over_overlay(const Game* g) {
    int x = 150;
    int y = 210;
    int w = 460;
    int h = 260;
    render_overlay_panel(x, y, w, h);

    const wchar_t* title = g->win ? L"过关！" : L"游戏结束";
    COLORREF titleColor = g->win ? RGB(120, 240, 160) : RGB(255, 130, 120);
    text_center(GAME_W / 2, y + 24, title, 36, titleColor, true);

    wchar_t buf[128];
    swprintf_s(buf, 128, L"本局得分：%d", g->player.score);
    text_center(GAME_W / 2, y + 88, buf, 22, C_TEXT, false);
    swprintf_s(buf, 128, L"最高分：%d", g->highScore);
    text_center(GAME_W / 2, y + 124, buf, 20, C_TEXT, false);

    text_center(GAME_W / 2, y + 166, L"点击【再玩一次】或按 Enter", 18, RGB(150, 220, 255), false);
    text_center(GAME_W / 2, y + 200, L"点击【返回菜单】或按 ESC", 16, RGB(160, 190, 220), false);
}

static void render_login(const Game* g) {
    for (int y = 0; y < WIN_H; y += 8) {
        float t = (float)y / (float)WIN_H;
        setfillcolor(lerp_color(RGB(15, 65, 110), RGB(4, 16, 32), t));
        solidrectangle(0, y, WIN_W, y + 8);
    }

    setlinecolor(RGB(70, 160, 220));
    for (int i = 0; i < 30; i++) {
        int bx = (i * 83 + (int)(g->animTime * (float)(8 + i % 7) * 3.0f)) % WIN_W;
        int by = 40 + (i * 71) % (WIN_H - 80);
        int r = 2 + i % 5;
        circle(bx, by, r);
    }

    text_center(WIN_W / 2, 106, L"大鱼吃小鱼", 56, RGB(255, 215, 90), true);
    text_center(WIN_W / 2, 178, L"Big Fish Eats Small Fish", 20, RGB(160, 210, 240), false);

    int px = 312;
    int py = 300;
    int pw = 400;
    int ph = 230;
    setfillcolor(RGB(8, 28, 50));
    solidroundrect(px, py, px + pw, py + ph, 18, 18);
    setlinecolor(RGB(90, 170, 220));
    roundrect(px, py, px + pw, py + ph, 18, 18);

    text_center(WIN_W / 2, py + 22, L"玩家登录", 26, C_ACCENT, true);
    text_center(WIN_W / 2, py + 64, L"请输入玩家名（英文/数字）", 17, RGB(180, 205, 230), false);

    int bx = px + 60;
    int by = py + 100;
    int bw = pw - 120;
    int bh = 42;
    setfillcolor(RGB(18, 48, 78));
    solidrectangle(bx, by, bx + bw, by + bh);
    setlinecolor(RGB(70, 140, 190));
    rectangle(bx, by, bx + bw, by + bh);

    bool cursor = ((GetTickCount() / 500) % 2) == 0;
    wchar_t display[MAX_NAME + 4];
    swprintf_s(display, MAX_NAME + 4, L"%ls%ls", g->loginName, cursor ? L"|" : L"");
    set_font(22, false);
    setbkmode(TRANSPARENT);
    settextcolor(C_TEXT);
    outtextxy(bx + 12, by + 8, display);

    if (g->loginMessage[0]) {
        text_center(WIN_W / 2, py + 152, g->loginMessage, 16, RGB(255, 180, 120), false);
    }
    text_center(WIN_W / 2, py + 186, L"按 Enter 进入游戏", 18, RGB(150, 220, 255), true);

    text_center(WIN_W / 2, WIN_H - 58,
                L"方向键 / WASD 移动    P 暂停    ESC 结束", 16, RGB(130, 170, 200), false);
}

void render_frame(const Game* g) {
    if (g->state == STATE_LOGIN) {
        render_login(g);
        return;
    }

    render_game_background(g);
    render_fish_list(&g->fish);

    if (g->state != STATE_MENU) {
        render_player(g);
    }

    render_topbar(g);
    render_info_panel(g);

    if (g->state == STATE_MENU) {
        render_menu_overlay();
    } else if (g->state == STATE_PAUSED) {
        render_pause_overlay(g);
    } else if (g->state == STATE_OVER) {
        render_over_overlay(g);
    }
}
