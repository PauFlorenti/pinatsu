@echo off

echo "Building engine ..."
set IncludePaths= /I..\engine\src\ /I..\math\ /I%VULKAN_SDK%\Include\
set FilePaths= ..\engine\src\core\*.cpp ..\engine\src\platform\*.cpp ..\engine\src\memory\*.cpp ..\engine\src\renderer\*.cpp ..\engine\src\renderer\vulkan\*.cpp

mkdir ..\bin
pushd ..\bin

cl /DDEBUG /EHsc /Zi /WX /c %IncludePaths% %FilePaths%

LIB /OUT:engine.lib *.obj
popd ..\bin