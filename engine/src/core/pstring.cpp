#include "pstring.h"

#include "memory/pmemory.h"

#include <stdarg.h>
#include <string.h>

static i32 stringFormatV(char* buf, const char* format, va_list va_list);

u64 stringLength(const char* str)
{
    return strlen(str);
}

char* stringDuplicate(const char* str)
{
    u64 size = stringLength(str);
    void* outStr = memAllocate(size, MEMORY_TAG_STRING);
    memCopy((void*)str, outStr, size);
    return (char*)outStr;
}

bool stringEquals(const char* str0, const char* str1)
{
    return strcmp(str0, str1) == 0;
}

char* stringEmpty(char* str)
{
    if(str){
        str[0] = 0;
    }
    return str;
}

char* stringCopy(const char* src, char* dst)
{
    return strcpy(dst, src);
}

i32 stringFormat(char* buf, const char* format, ...)
{
    if(buf)
    {
        va_list args;
        va_start(args, format);
        i32 written = stringFormatV(buf, format, args);
        va_end(args);
        return written;
    }
    return -1;
}

static i32 stringFormatV(char* buf, const char* format, va_list va_list)
{
    if(buf)
    {
        char buffer[3200];
        i32 written = vsnprintf(buffer, 3200, format, va_list);
        buffer[written] = 0;
        memCopy(buffer, buf, written + 1);

        return written;
    }
    return -1;
}

char* stringTrim(char* str)
{
    while(isspace((unsigned char)*str)){
        str++;
    }

    // String is empty.
    if(*str == 0)
        return str;

    if(*str) {
        char* p = str;
        while(*p){
            p++;
        }
        while(isspace((unsigned char)*p)) {
            --p;
        }
        p[1] = '\0';
    }
    return str;
}

i32 stringIndexOf(const char* str, char c)
{
    if(!str){
        return -1;
    }

    u64 length = stringLength(str);
    for(u32 i = 0; i < length; ++i){
        if(str[i] == c){
            return i;
        }
    }
    return -1;
}

void stringMid(char* buffer, const char* str, i32 start, i32 length)
{
    if(!str){
        return;
    }

    u64 strLength = stringLength(str);
    if(length >= strLength){
        buffer = nullptr;
        return;
    }

    if(length > 0)
    {
        for(u64 i = start, j = 0; i < length && str[i]; ++i, ++j)
        {
            buffer[j] = str[i];
        }
        buffer[start + length] = 0;
    } 
    else 
    {
        u64 j = 0;
        for(u64 i = start; str[i]; ++i, ++j)
        {
            buffer[j] = str[i];
        }
        buffer[start + j] = 0;
    }
}