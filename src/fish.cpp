#include "fish.h"

static float frand(float lo, float hi) {
    return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo);
}

static Fish* fish_create(FishType type, float x, float y, float vx, float vy,
                         float halfW, float halfH, int score) {
    Fish* f = (Fish*)malloc(sizeof(Fish));
    if (!f) {
        return NULL;
    }
    f->type = type;
    f->x = x;
    f->y = y;
    f->vx = vx;
    f->vy = vy;
    f->halfW = halfW;
    f->halfH = halfH;
    f->size = (int)(halfW * 2.0f);
    f->score = score;
    f->variant = rand() % 3;
    f->turnsLeft = 0;
    f->justTurned = false;
    f->alive = true;
    f->next = NULL;
    return f;
}

void fishlist_init(FishList* list) {
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

void fishlist_clear(FishList* list) {
    Fish* cur = list->head;
    while (cur) {
        Fish* next = cur->next;
        free(cur);
        cur = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

void fishlist_append(FishList* list, Fish* fish) {
    if (!fish) {
        return;
    }
    fish->next = NULL;
    if (list->tail) {
        list->tail->next = fish;
    } else {
        list->head = fish;
    }
    list->tail = fish;
    list->count++;
}

Fish* fishlist_detach(FishList* list, Fish* fish) {
    if (!fish || !list->head) {
        return NULL;
    }
    Fish* prev = NULL;
    Fish* cur = list->head;
    while (cur && cur != fish) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) {
        return NULL;
    }
    if (prev) {
        prev->next = cur->next;
    } else {
        list->head = cur->next;
    }
    if (list->tail == cur) {
        list->tail = prev;
    }
    cur->next = NULL;
    list->count--;
    return cur;
}

void fishlist_remove(FishList* list, Fish* fish) {
    Fish* detached = fishlist_detach(list, fish);
    if (detached) {
        free(detached);
    }
}

Fish* fishlist_spawn(FishList* list, float gameW, float gameH) {
    int r = rand() % 100;
    FishType type;
    if (r < 65) {
        type = FISH_SMALL;
    } else if (r < 80) {
        type = FISH_BIG;
    } else if (r < 90) {
        type = FISH_BOMB;
    } else {
        type = FISH_CLOCK;
    }

    float hw = 20.0f;
    float hh = 14.0f;
    float speed = 80.0f;
    int score = 0;

    switch (type) {
    case FISH_SMALL:
        hw = frand(18.0f, 32.0f);
        hh = hw * 0.62f;
        score = 1 + (int)((hw - 18.0f) / 7.0f);
        speed = frand(75.0f, 150.0f);
        break;
    case FISH_BIG:
        hw = frand(48.0f, 72.0f);
        hh = hw * 0.60f;
        score = 0;
        speed = frand(35.0f, 70.0f);
        break;
    case FISH_BOMB:
        hw = 18.0f;
        hh = 18.0f;
        score = 0;
        speed = frand(55.0f, 110.0f);
        break;
    default: /* FISH_CLOCK */
        hw = 18.0f;
        hh = 18.0f;
        score = 0;
        speed = frand(55.0f, 110.0f);
        break;
    }

    bool fromLeft = (rand() % 2) == 0;
    float x = fromLeft ? (-hw - 20.0f) : (gameW + hw + 20.0f);
    float y = frand(hh + 30.0f, gameH - hh - 20.0f);
    float vx = fromLeft ? speed : -speed;
    float vy = frand(-18.0f, 18.0f);

    Fish* f = fish_create(type, x, y, vx, vy, hw, hh, score);
    if (f) {
        /* 大鱼到边缘会掉头一次，小鱼偶尔掉头，增加“猛回头”效果 */
        if (type == FISH_BIG) {
            f->turnsLeft = 1;
        } else if (type == FISH_SMALL && (rand() % 4 == 0)) {
            f->turnsLeft = 1;
        }
    }
    fishlist_append(list, f);
    return f;
}

void fishlist_update(FishList* list, float dt, float gameW, float gameH) {
    Fish* prev = NULL;
    Fish* cur = list->head;

    while (cur) {
        cur->x += cur->vx * dt;
        cur->y += cur->vy * dt;

        if (cur->y < cur->halfH + 10.0f) {
            cur->y = cur->halfH + 10.0f;
            cur->vy = -cur->vy;
        } else if (cur->y > gameH - cur->halfH - 10.0f) {
            cur->y = gameH - cur->halfH - 10.0f;
            cur->vy = -cur->vy;
        }

        /* 到达左右边缘时掉头（只对还有掉头次数的鱼生效） */
        if (cur->turnsLeft > 0) {
            bool turned = false;
            if (cur->x < cur->halfW + 6.0f && cur->vx < 0.0f) {
                cur->x = cur->halfW + 6.0f;
                cur->vx = -cur->vx;
                turned = true;
            } else if (cur->x > gameW - cur->halfW - 6.0f && cur->vx > 0.0f) {
                cur->x = gameW - cur->halfW - 6.0f;
                cur->vx = -cur->vx;
                turned = true;
            }
            if (turned) {
                cur->turnsLeft--;
                cur->justTurned = true;
            }
        }

        if (cur->x < -cur->halfW * 3.0f || cur->x > gameW + cur->halfW * 3.0f) {
            Fish* next = cur->next;
            if (prev) {
                prev->next = next;
            } else {
                list->head = next;
            }
            if (list->tail == cur) {
                list->tail = prev;
            }
            free(cur);
            list->count--;
            cur = next;
        } else {
            prev = cur;
            cur = cur->next;
        }
    }
}

const wchar_t* fish_type_name(FishType type) {
    switch (type) {
    case FISH_SMALL:
        return L"小鱼";
    case FISH_BIG:
        return L"大鱼";
    case FISH_BOMB:
        return L"炸弹";
    default:
        return L"闹钟";
    }
}
