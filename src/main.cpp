#include "common.h"
#include "assets.h"
#include "game.h"
#include "input.h"
#include "render.h"
#include "sound.h"

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

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    if (screenW <= 0 || screenH <= 0) {
        screenW = WIN_W;
        screenH = WIN_H;
    }

    initgraph(screenW, screenH);

    HWND hwnd = GetHWnd();
    SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, screenW, screenH,
                 SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    SetForegroundWindow(hwnd);

    assets_init();
    sound_init();
    render_init(screenW, screenH);

    Game game;
    game_init(&game, screenW, screenH);

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
        sound_update();
        game_update(&game, &input, dt);
        render_frame(&game);
        render_present();
        Sleep(16);
    }

    sound_shutdown();
    SetWorkingImage(NULL);
    closegraph();
    return 0;
}
