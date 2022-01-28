@echo off

echo "Building sandbox ..."

mkdir ..\bin
pushd ..\bin
cl /EHsc /Zi /WX /DDEBUG ..\sandbox\src\sandbox.cpp ..\sandbox\src\entry.cpp /I..\engine\src /link engine.lib user32.lib %VULKAN_SDK%\Lib\vulkan-1.lib
popd