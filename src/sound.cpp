#include "sound.h"
#include "assets.h"
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

static bool s_enabled = true;
static bool s_ambient = false;
static DWORD s_nextCheck = 0;

void sound_init(void) {
    wchar_t path[MAX_PATH];
    if (!assets_find(L"sounds/ambient.wav", path, MAX_PATH)) {
        return;
    }

    wchar_t cmd[1024];
    swprintf_s(cmd, 1024, L"open \"%ls\" type waveaudio alias gf_ambient", path);
    if (mciSendStringW(cmd, NULL, 0, NULL) == 0) {
        if (mciSendStringW(L"play gf_ambient", NULL, 0, NULL) == 0) {
            s_ambient = true;
        } else {
            mciSendStringW(L"close gf_ambient", NULL, 0, NULL);
        }
    }
}

void sound_update(void) {
    if (!s_ambient || !s_enabled) {
        return;
    }

    DWORD now = GetTickCount();
    if (now < s_nextCheck) {
        return;
    }
    s_nextCheck = now + 1000;

    wchar_t mode[32] = { 0 };
    mciSendStringW(L"status gf_ambient mode", mode, 32, NULL);
    if (mode[0] == 0 || wcscmp(mode, L"stopped") == 0) {
        mciSendStringW(L"play gf_ambient", NULL, 0, NULL);
    }
}

void sound_shutdown(void) {
    PlaySoundW(NULL, NULL, 0);
    if (s_ambient) {
        mciSendStringW(L"close gf_ambient", NULL, 0, NULL);
        s_ambient = false;
    }
}

void sound_play(const wchar_t* name) {
    if (!s_enabled || !name) {
        return;
    }

    wchar_t rel[128];
    swprintf_s(rel, 128, L"sounds/%ls", name);

    wchar_t path[MAX_PATH];
    if (!assets_find(rel, path, MAX_PATH)) {
        return;
    }
    PlaySoundW(path, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
}

void sound_set_enabled(bool enabled) {
    s_enabled = enabled;
    if (!s_ambient) {
        return;
    }

    if (enabled) {
        mciSendStringW(L"play gf_ambient", NULL, 0, NULL);
    } else {
        mciSendStringW(L"pause gf_ambient", NULL, 0, NULL);
        PlaySoundW(NULL, NULL, 0);
    }
}

bool sound_is_enabled(void) {
    return s_enabled;
}
