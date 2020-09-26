# StageCast Compositor

## Build directions

To build the source, you'll need the following packages:

* `g++` >= 8.0
* `libxinerama-dev`
* `libxcursor-dev`
* `libglu1-mesa-dev`
* `libxrandr-dev`
* `libxi-dev`
* `libglew-dev`
* `libglfw3-dev`
* `libjpeg-dev`

The rest should be straightforward:

```
$ mkdir build
$ cd build
$ cmake ..
$ make -j`nproc`
```
