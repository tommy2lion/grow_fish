#include "assets.h"

static IMAGE s_base[SPR_COUNT];
static IMAGE s_flip[SPR_COUNT];
static IMAGE s_bg[BG_COUNT];
static bool s_ok = false;

static const wchar_t* s_files[SPR_COUNT] = {
    L"fish/player_green.png",
    L"fish/small_orange.png",
    L"fish/small_blue.png",
    L"fish/small_pink.png",
    L"fish/big_brown.png",
    L"fish/big_red.png"
};

static const wchar_t* s_bg_files[BG_COUNT] = {
    L"fish/seaweed_a.png",
    L"fish/seaweed_b.png",
    L"fish/seaweed_c.png",
    L"fish/rock_a.png",
    L"fish/rock_b.png",
    L"fish/bubble_a.png",
    L"fish/bubble_b.png",
    L"fish/bubble_c.png"
};

static bool file_exists(const wchar_t* path) {
    DWORD attr = GetFileAttributesW(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static void get_exe_dir(wchar_t* out, int cap) {
    GetModuleFileNameW(NULL, out, cap);
    wchar_t* slash = wcsrchr(out, L'\\');
    if (slash) {
        *slash = 0;
    }
}

bool assets_find(const wchar_t* rel, wchar_t* out, int cap) {
    if (!rel || !out || cap <= 0) {
        return false;
    }

    /* 1. 当前工作目录（Visual Studio 调试时通常为仓库根） */
    swprintf_s(out, cap, L"assets/%ls", rel);
    if (file_exists(out)) {
        return true;
    }

    wchar_t dir[MAX_PATH];
    get_exe_dir(dir, MAX_PATH);

    /* 2. 可执行文件旁边的 assets（构建后自动复制） */
    swprintf_s(out, cap, L"%ls/assets/%ls", dir, rel);
    if (file_exists(out)) {
        return true;
    }

    /* 3. build/x64/<Config>/ 回退到仓库根 assets */
    swprintf_s(out, cap, L"%ls/../../../assets/%ls", dir, rel);
    if (file_exists(out)) {
        return true;
    }

    return false;
}

static void make_flip(IMAGE* dst, const IMAGE* src) {
    int w = src->getwidth();
    int h = src->getheight();
    if (w <= 0 || h <= 0) {
        return;
    }
    dst->Resize(w, h);
    DWORD* s = GetImageBuffer((IMAGE*)src);
    DWORD* d = GetImageBuffer(dst);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            d[y * w + x] = s[y * w + (w - 1 - x)];
        }
    }
}

void assets_init(void) {
    s_ok = true;
    wchar_t path[MAX_PATH];

    for (int i = 0; i < SPR_COUNT; i++) {
        bool loaded = false;
        if (assets_find(s_files[i], path, MAX_PATH)) {
            loadimage(&s_base[i], path);
            if (s_base[i].getwidth() > 0) {
                make_flip(&s_flip[i], &s_base[i]);
                loaded = true;
            }
        }
        if (!loaded) {
            s_ok = false;
        }
    }

    for (int i = 0; i < BG_COUNT; i++) {
        if (assets_find(s_bg_files[i], path, MAX_PATH)) {
            loadimage(&s_bg[i], path);
        }
    }
}

bool assets_ok(void) {
    return s_ok;
}

IMAGE* assets_get(SpriteId id, bool facingLeft) {
    if (id < 0 || id >= SPR_COUNT) {
        return NULL;
    }
    IMAGE* img = facingLeft ? &s_flip[id] : &s_base[id];
    return img->getwidth() > 0 ? img : NULL;
}

IMAGE* assets_bg(BackgroundSprite id) {
    if (id < 0 || id >= BG_COUNT) {
        return NULL;
    }
    IMAGE* img = &s_bg[id];
    return img->getwidth() > 0 ? img : NULL;
}
