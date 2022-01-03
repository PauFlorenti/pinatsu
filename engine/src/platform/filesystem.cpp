#include "filesystem.h"

#include "core/logger.h"
#include <string.h>

bool filesystemExists(const char* filename)
{
    return true;
}

// Get the size of the file, 
// but returning the cursor to the current position.
void filesystemSize(
    FileHandle* handle, 
    u64* size)
{
    if(handle->isValid)
    {
        fseek(handle->handle, 0, SEEK_END);
        *size = ftell(handle->handle);
        rewind(handle->handle);
    }
}

bool filesystemOpen(
    const char* filename, 
    FileModes mode, 
    bool binary, 
    FileHandle* handle)
{
    const char* modeStr;

    // Write and read
    if((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0)
    {
        modeStr = binary ? "w+b" : "w+";
    }
    // Read
    else if((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0)
    {
        modeStr = binary ? "rb" : "r";
    }
    // Write
    else if((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0)
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
        FILE* f = handle->handle;
        fclose(f);
        handle->handle = nullptr;
        handle->isValid = false;
    }
}

bool filesystemRead(
    FileHandle* handle, 
    u64 dataSize, 
    void* outData, 
    void* outBinaryData)
{
    if(handle->handle)
    {
        fread(outData, sizeof(char), dataSize, static_cast<FILE*>(handle->handle));
        //if()
        return true;
    }
    return false;
}

bool filesystemReadLine(
    FileHandle* handle, 
    u64 maxLenght,
    u64* outLength,
    void* outData)
{
    if(handle && handle->isValid && maxLenght > 0)
    {
        if(fgets((char*)outData, maxLenght, handle->handle) != 0){
            *outLength = strlen((char*)outData);
            return true;
        }
    }
    return false;
}