# Sokol GP

Minimal efficient cross platform 2D graphics painter in C 
using modern graphics API through sokol_gfx.

## Features

* Minimal
* Single header
* Pure C
* Cross platform
* Optimized for 2D rendering, no 3D support
* Use modern unfixed pipeline graphics APIs for more efficiency
* Build on top D3D11/OpenGL 3.3/Metal/WebGPU APIs through sokol_gfx
* Support for fragment shaders on 2D surfaces
* Deferred rendering into a draw command queue
* Automatic batching (merge recent draw calls into batches automatically)
* Batch optimizer (rearranges draw calls order to batch more)
* Extensions to use with SDL2 (context creation)

## Future features planned

These are features I would like to implement someday in the future.

* Automatic texture atlas packer (to allow more batching)
