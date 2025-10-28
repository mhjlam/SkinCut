# Building SkinCut

This guide explains how to build SkinCut using CMake and vcpkg for dependency management.

## Prerequisites

1. **Visual Studio 2022**
   - Install "Desktop development with C++" workload.
   - Make sure that the Windows SDK is included.

2. **CMake 3.20 or later**
   - Download from <https://cmake.org/download>.
   - Add to `PATH` during installation.

3. `vcpkg` (Microsoft's C++ Package Manager)

   ```cmd
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   ```

4. **Set Environment Variable**

   ```cmd
   set VCPKG_ROOT=C:\path\to\vcpkg
   ```

   Or even better, add `VCPKG_ROOT` to your system environment variables.

## Build Instructions

### Option 1: Using CMakePresets (Recommended)

1. **Configure the project**:

   ```cmd
   cmake --preset msvc-x64
   ```

2. **Build Debug**:

   ```cmd
   cmake --build --preset debug
   ```

3. **Build Release**:

   ```cmd
   cmake --build --preset release
   ```

### Option 2: Manual Configuration

1. **Configure with vcpkg**:

   ```cmd
   cmake -S . -B Build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
   ```

2. **Build**:

   ```cmd
   cmake --build Build --config Release
   ```

## Dependencies

The following libraries will be automatically downloaded and built by `vcpkg`:

- **DirectXTex**: DirectX texture processing library.
- **DirectXTK**: DirectX Tool Kit for common graphics tasks.
- **Dear ImGui**: Immediate mode GUI library with DirectX 11 bindings.
- **nlohmann/json**: Modern C++ JSON library.

## Output Location

The built executable will be located at:

```text
Bin/Release/SkinCut.exe
```

The `Resources/` folder is automatically copied to `Bin/Release/Resources/` during build.

## Development

For development, you can open the project directory in Visual Studio code:

```cmd
code C:\path\to\repo\
```

Or open the generated solution file in Visual Studio directly:

```cmd
cmake --open Build
```
