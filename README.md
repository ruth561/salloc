# Simple Allocator

This is a very simple memory allocator, created to test the heaphook itself.

# How to build
Firstly, you need to create a [heaphook](https://github.com/ruth561/heaphook) environment.
```bash
$ mkdir -p /path/to/heaphook_ws && cd /path/to/heaphook_ws
$ mkdir src & git clone https://github.com/ruth561/heaphook src/heaphook
```
Next, clone this repository under the src directory of heaphook.
```bash
$ git clone git@github.com:ruth561/salloc.git src/heaphook/src/salloc
```
Finally, add the following text to the CMakeLists.txt file of heaphook.
```cmake
...

add_subdirectory(src/salloc) # <-- add!

ament_package()
```
Once these are done, you can build by colcon.
```bash
$ colcon build
```

# How to use
After building, you can use libsalloc.so like the following.
```bash
$ source install/setup.bash
$ LD_PRELOAD=libsalloc.so executable
```