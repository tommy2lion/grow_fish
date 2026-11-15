#include "common.h"
#include "game.h"
#include "input.h"
#include "render.h"

#include <imm.h>
#pragma comment(lib, "imm32.lib")

int main(void) {
    srand((unsigned)time(NULL));
    SetProcessDPIAware();
    ImmDisableIME(0);

    HWND console = GetConsoleWindow();
    if (console) {
        ShowWindow(console, SW_HIDE);
    }

    initgraph(WIN_W, WIN_H);
    setbkmode(TRANSPARENT);
    BeginBatchDraw();
    render_init();

    Game game;
    game_init(&game);

    Input input;
    input_init(&input);

    DWORD last = GetTickCount();
    while (!input.quit) {
        DWORD now = GetTickCount();
        float dt = (float)(now - last) / 1000.0f;
        if (dt > 0.1f) {
            dt = 0.1f;
        }
        last = now;

        input_update(&input);
        game_update(&game, &input, dt);
        render_frame(&game);
        FlushBatchDraw();
        Sleep(16);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}
