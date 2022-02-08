@echo off

mkdir bin
pushd bin

cl /EHsc /Zi /WX /DDEBUG ..\src\breakout.cpp ..\src\entry.cpp /I..\..\engine\src /link ..\..\bin\engine.lib user32.lib %VULKAN_SDK%\Lib\vulkan-1.lib

LIB /OUT:engine.lib *.obj
popd bin