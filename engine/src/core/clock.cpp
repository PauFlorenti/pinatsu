#include "clock.h"

#include "platform\platform.h"

void clockUpdate(Clock* clock)
{
    if(clock->startTime != 0){
        clock->elapsedTime = platformGetCurrentTime() - clock->elapsedTime;
    }
}

void clockStart(Clock* clock)
{
    clock->startTime = platformGetCurrentTime();
    clock->elapsedTime = 0;
}

void clockStop(Clock* clock)
{
    clock->startTime = 0;
}