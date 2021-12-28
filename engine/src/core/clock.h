#pragma once

#include "defines.h"

typedef struct Clock
{
    f64 startTime;
    f64 elapsedTime;
} Clock;

void clockUpdate(Clock* clock);
void clockStart(Clock* clock);
void clockStop(Clock* clock);