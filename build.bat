@ECHO ON

set BASEDIR=%~dp0
PUSHD %BASEDIR%

IF EXIST build (
    RMDIR /Q /S build
)

set VCPKG_DEFAULT_TRIPLET=x%PROCESSOR_ARCHITECTURE:~-2%-mingw-static
set VCPKG_DEFAULT_HOST_TRIPLET=x%PROCESSOR_ARCHITECTURE:~-2%-mingw-static

IF NOT EXIST .\vcpkg\vcpkg.exe (
    call .\vcpkg\bootstrap-vcpkg.bat
)
.\vcpkg\vcpkg.exe install

cmake -G "MinGW Makefiles" -B build
cmake --build build
