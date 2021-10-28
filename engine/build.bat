@echo off

echo "Building engine ..."

mkdir ..\bin
pushd ..\bin
cl /EHsc /Zi /WX /c /I..\engine\src\ /I..\math\ /I%VULKAN_SDK%\Include\ ..\engine\src\core\*.cpp ..\engine\src\platform\*.cpp ..\engine\src\memory\*.cpp ..\engine\src\renderer\*.cpp ..\engine\src\renderer\vulkan\*.cpp

LIB /OUT:engine.lib *.obj
popd ..\bin