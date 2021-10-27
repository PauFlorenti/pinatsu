@echo off

echo "Building engine ..."

mkdir ..\bin
pushd ..\bin
cl /EHsc /Zi /WX /c /I..\engine\src\ ..\engine\src\core\*.cpp ..\engine\src\platform\*.cpp ..\engine\src\memory\*.cpp ..\engine\src\renderer\*.cpp

LIB /OUT:engine.lib *.obj
popd ..\bin