@echo off
REM SkinCut Build Script
REM This script automates the vcpkg setup and CMake build process

echo ===============================================
echo SkinCut Build Script
echo ===============================================
echo.

REM Check if VCPKG_ROOT is set
if not defined VCPKG_ROOT (
    echo ERROR: VCPKG_ROOT environment variable is not set
    echo Please set VCPKG_ROOT to your vcpkg installation directory
    echo Example: set VCPKG_ROOT=C:\vcpkg
    pause
    exit /b 1
)

echo VCPKG_ROOT is set to: %VCPKG_ROOT%
echo.

REM Check if vcpkg exists
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo ERROR: vcpkg.exe not found at %VCPKG_ROOT%
    echo Please make sure vcpkg is properly installed
    pause
    exit /b 1
)

echo Step 1: Installing dependencies with vcpkg...
echo.
%VCPKG_ROOT%\vcpkg.exe install --triplet x64-windows

if %errorlevel% neq 0 (
    echo ERROR: vcpkg install failed
    pause
    exit /b %errorlevel%
)

echo.
echo Step 2: Configuring with CMake...
echo.
cmake -S . -B Build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake

if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b %errorlevel%
)

echo.
echo Step 3: Building Release configuration...
echo.
cmake --build Build --config Release

if %errorlevel% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b %errorlevel%
)

echo.
echo ===============================================
echo Build completed successfully!
echo ===============================================
echo Executable location: Bin\Release\SkinCut.exe
echo Resources automatically copied to: Bin\Release\Resources\
echo Ready to run!
echo.
pause