@echo off

echo "Building all project ..."

pushd bin
del /Q *
popd

pushd engine
call build.bat
popd

pushd sandbox
call build.bat
popd