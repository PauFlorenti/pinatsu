#pragma once

#include "defines.h"
#include <stdio.h>

typedef struct FileHandle
{
    bool isValid;
    FILE* handle;
} FileHandle;

typedef enum FileModes
{
    FILE_MODE_WRITE = 0x01,
    FILE_MODE_READ = 0x02
} FileModes;

bool filesystemExists(const char* filename);
void filesystemSize(FileHandle* handle, u64* size);

bool filesystemOpen(const char* filename, FileModes mode, bool binary, FileHandle* handle);
void filesystemClose(FileHandle* handle);
bool filesystemRead(FileHandle* handle, void* outData);
bool filesystemReadLine(
    FileHandle* handle, 
    u64 maxLength, 
    u64* outLength, 
    void* outData);

// TODO bool filesystemWrite();