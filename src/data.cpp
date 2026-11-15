#include "data.h"

static void data_sort(ScoreRecord records[], int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (records[j].score > records[i].score) {
                ScoreRecord tmp = records[i];
                records[i] = records[j];
                records[j] = tmp;
            }
        }
    }
}

void data_load(ScoreRecord records[], int* count) {
    *count = 0;
    FILE* fp = _wfopen(L"records.txt", L"r, ccs=UTF-8");
    if (!fp) {
        return;
    }

    wchar_t line[256];
    while (fgetws(line, 256, fp) && *count < MAX_RECORDS) {
        wchar_t* p = line;
        if (p[0] == 0xFEFF) {
            p++;
        }
        wchar_t* nl = wcschr(p, L'\n');
        if (nl) {
            *nl = 0;
        }
        wchar_t* cr = wcschr(p, L'\r');
        if (cr) {
            *cr = 0;
        }
        wchar_t* tab = wcschr(p, L'\t');
        if (!tab) {
            continue;
        }
        *tab = 0;
        if (!p[0]) {
            continue;
        }
        wcsncpy_s(records[*count].name, MAX_NAME, p, _TRUNCATE);
        records[*count].score = _wtoi(tab + 1);
        (*count)++;
    }
    fclose(fp);
    data_sort(records, *count);
}

void data_save(ScoreRecord records[], int* count, const wchar_t* name, int score) {
    ScoreRecord rec;
    wcsncpy_s(rec.name, MAX_NAME, name, _TRUNCATE);
    rec.score = score;

    if (*count < MAX_RECORDS) {
        records[*count] = rec;
        (*count)++;
    } else if (score > records[MAX_RECORDS - 1].score) {
        records[MAX_RECORDS - 1] = rec;
    } else {
        return;
    }
    data_sort(records, *count);

    FILE* fp = _wfopen(L"records.txt", L"w, ccs=UTF-8");
    if (!fp) {
        return;
    }
    for (int i = 0; i < *count; i++) {
        fwprintf(fp, L"%ls\t%d\n", records[i].name, records[i].score);
    }
    fclose(fp);
}

int data_high_score(const ScoreRecord records[], int count) {
    return count > 0 ? records[0].score : 0;
}
