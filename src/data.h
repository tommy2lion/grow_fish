#pragma once

#include "common.h"

void data_load(ScoreRecord records[], int* count);
void data_save(ScoreRecord records[], int* count, const wchar_t* name, int score);
int data_high_score(const ScoreRecord records[], int count);
