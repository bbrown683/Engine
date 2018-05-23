# Ivy3
Experimental Direct3D12 and Vulkan Engine.

# Building
Currently the build system only supports Windows via Visual Studio 2017. This project uses [vcpkg](https://github.com/Microsoft/vcpkg) for installing dependencies. A CMake script is under development for cross platform support.

## Library Dependencies
* freetype2
* icu
* libogg
* libpng
* libvorbis 
* SDL2
* zlib

## External SDKs
* Vulkan SDK
* Windows 10 SDK

# Requirements
* Deferred Shading/Lighting through use of a G-Buffer. 
* Multithreaded renderer for increased CPU core usage.
* Scenegraph for optimal object filtering in a scene.
* Stored shader bytecode on disk for skipping compilation time.