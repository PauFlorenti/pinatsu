@echo off

echo "Building sandbox ..."

mkdir ..\bin
pushd ..\bin
cl /EHsc /Zi /WX ..\sandbox\src\sandbox.cpp /I..\engine\src /link engine.lib user32.lib
popd ..\bin