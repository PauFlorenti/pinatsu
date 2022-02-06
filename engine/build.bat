@echo off

echo "Building engine ..."
set IncludePaths= /I..\engine\src\ /I..\math\ /I%VULKAN_SDK%\Include\
set FilePaths= ..\engine\src\containers\*.cpp ..\engine\src\core\*.cpp ..\engine\src\platform\*.cpp ..\engine\src\memory\*.cpp ..\engine\src\renderer\*.cpp ..\engine\src\renderer\vulkan\*.cpp ..\engine\src\renderer\vulkan\shaders\*.cpp ..\engine\src\systems\*.cpp ..\engine\src\resources\loaders\*.cpp
set WarnDisabled= /wd4805

mkdir ..\bin
pushd ..\bin

cl /DDEBUG /EHsc /Zi /WX /c %WarnDisabled% %IncludePaths% %FilePaths%

LIB /OUT:engine.lib *.obj
popd ..\bin