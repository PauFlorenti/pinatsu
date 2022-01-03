#pragma once
#include "defines.h"

u64 stringLength(const char* str);
char* stringDuplicate(const char* str);
bool stringEquals(const char* str0, const char* str1);
char* stringEmpty(char* str);
char* stringCopy(const char* src, char* dst);
i32 stringFormat(char* dest, const char* format, ...);
char* stringTrim(char* str);
i32 stringIndexOf(const char* str, char c);
void stringMid(char* buffer, const char* str, i32 start, i32 length);