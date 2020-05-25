# NanoGP

Minimal efficient cross platform 2D graphics painter in C 
using modern graphics API through sokol_gfx.

**Warning:** Under development.

## Planned features

* Be minimal
* Use pure C
* Single header
* Cross platform
* Optimized for 2D rendering, no 3D support
* Take advantage of modern unfixed pipeline graphics APIs
* Build on top DX11/OpenGL 3.3/Metal/WebGPU APIs through sokol_gfx
* Support fragment shaders on 2D surfaces
* Deferred rendering into a draw command queue
* Automatic batching through command buffer optimization
* Seamless dirty rectangles optimization to avoid redrawing
* Have utilities for context creation on SDL window
* Optional asynchronous rendering
