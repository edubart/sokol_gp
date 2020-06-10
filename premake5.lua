workspace "workspace"
   kind "ConsoleApp"
   targetdir "build/%{cfg.buildcfg}"
   location "build"
   language "C"
   cdialect "C99"
   architecture "x86_64"
   configurations { "debug", "release" }
   includedirs {"."}
   warnings "Extra"
   links {"SDL2", "m"}

   newoption {
      trigger     = "backend",
      value       = "BACKEND",
      description = "Choose backend",
      default     = "glcore33",
      allowed = {
         { "glcore33",  "OpenGL Core 3.3" },
         { "gles2",     "OpenGL ES 2.0" },
         { "gles3",     "OpenGL ES 3.0" },
         { "wgpu",      "WebGPU" },
         { "d3d11",     "Direct3D 11 (Windows only)" },
         { "metal",     "Metal (MacOS only)" },
         { "dummy",     "Dummy" }
      }
   }

   filter "system:windows"
      defines {"SDL_MAIN_HANDLED"}
      links {"SDL2main", "opengl32", "d3d11", "dxgi", "dxguid"}

   filter "system:linux"
      links {"GL", "dl"}

   filter "configurations:debug"
      optimize "On"
      symbols "On"

   filter "configurations:release"
      optimize "Full"
      symbols "On"
      floatingpoint "Fast"
      flags { "LinkTimeOptimization" }
      defines { "NDEBUG" }

   filter {"options:backend=glcore33"}
      defines {"SOKOL_GLCORE33"}

   filter {"options:backend=gles2"}
      defines {"SOKOL_GLES2"}

   filter {"options:backend=gles3"}
      defines {"SOKOL_GLES3"}

   filter {"options:backend=d3d11"}
      defines {"SOKOL_D3D11"}

   filter {"options:backend=dummy"}
      defines {"SOKOL_DUMMY_BACKEND"}

project "sample-prims"
   files { "**.h", "samples/sample-prims.c" }

project "sample-blend"
   files { "**.h", "samples/sample-blend.c" }

project "sample-capture"
   files { "**.h", "samples/sample-capture.c" }

project "sample-fb"
   files { "**.h", "samples/sample-fb.c" }
