@ECHO ON

set BASEDIR=%~dp0
PUSHD %BASEDIR%

RMDIR /Q /S build

./vcpkg/bootstrap-vcpkg.bat
./vcpkg/vcpkg install

cmake -B build
cmake --build build
