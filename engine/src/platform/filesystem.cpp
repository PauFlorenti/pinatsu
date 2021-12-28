#include "filesystem.h"

#include <fstream>
#include "core/logger.h"

bool filesystemExists(const char* filename)
{
    return true;
}

bool filesystemOpen(const char* filename, FileModes mode, bool binary, FileHandle* handle)
{
    const char* modeStr;

    // Write and read
    if((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE))
    {
        modeStr = binary ? "w+b" : "w+";
    }
    else if((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0)
    {
        modeStr = binary ? "rb" : "r";
    }
    else if((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0)
    {
        modeStr = binary ? "wb" : "w";
    }

    FILE* f = fopen(filename, modeStr);
    if(!f) {
        PERROR("Could not open file %s.", filename);
        handle->isValid = false;
        handle->handle = nullptr;
        return false;
    }

    handle->handle = f;
    handle->isValid = true;
    return true;
}

void filesystemClose(FileHandle* handle)
{
    if(handle->handle)
    {
        FILE* f = static_cast<FILE*>(handle->handle);
        fclose(f);
        handle->handle = nullptr;
        handle->isValid = false;
    }
}

bool filesystemRead(FileHandle* handle, u64 dataSize, void* outData, void* outBinaryData)
{
    if(handle->handle)
    {
        fread(outData, sizeof(char), dataSize, static_cast<FILE*>(handle->handle));
        //if()
        return true;
    }
    return false;
}