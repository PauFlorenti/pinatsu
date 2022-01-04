#pragma once
#include "defines.h"

#include "external/glm/glm.hpp"

u64 stringLength(const char* str);
char* stringDuplicate(const char* str);
bool stringEquals(const char* str0, const char* str1);
char* stringEmpty(char* str);
char* stringCopy(const char* src, char* dst);
i32 stringFormat(char* dest, const char* format, ...);
char* stringTrim(char* str);
i32 stringIndexOf(const char* str, char c);
void stringMid(char* buffer, const char* str, i32 start, i32 length);

bool stringToVec4(const char* str, glm::vec4* outVector);
bool stringToVec3(const char* str, glm::vec3* outVector);
bool stringToVec2(const char* str, glm::vec2* outVector);
bool stringToObjFace(const char* str, glm::vec3* outFace);