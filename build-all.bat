@echo off

echo "Building all project ..."

mkdir bin
pushd bin
del /Q *
popd

pushd engine
call build.bat
popd

pushd sandbox
call build.bat
popd