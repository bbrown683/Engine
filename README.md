# Ivy3
Experimental Direct3D12 and Vulkan Engine.

# Building
Currently the build system only supports Windows. Start by installing Visual Studio 2017, along with following the steps of setting up [vcpkg](https://github.com/Microsoft/vcpkg). You will need to install GLFW via vcpkg for 32 and 64 bit targets. 

# External Dependencies
* Vulkan SDK
* Windows 10 SDK

# Requirements
* Deferred Shading/Lighting through use of a G-Buffer. 
* Multithreaded renderer for increased CPU core usage.
* Scenegraph for optimal object filtering in a scene.