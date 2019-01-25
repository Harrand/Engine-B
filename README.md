# Engine B
## About

Engine B is an improved copy of [Engine A](https://github.com/Harrand/Engine-A). Both Engines A and B are built upon the [Topaz engine](https://github.com/Harrand/Topaz). Specifically, commit hash [6fdf6d4ca620087aa06350a926a92c12bc6a580d](https://github.com/Harrand/Topaz/tree/6fdf6d4ca620087aa06350a926a92c12bc6a580d).

### Built With

* [GLEW](http://glew.sourceforge.net/) - The OpenGL Extension Wrangler Library, to access OpenGL trivially.
* [SDL2](https://www.libsdl.org/) - Simple DirectMedia Layer, cross-platform library used for windowing.
* [SDL2_mixer 2.0](https://www.libsdl.org/projects/SDL_mixer/) - SDL2 Plugin used to load audio files (.wav, .ogg files etc...)
* [SDL_ttf 2.0](https://www.libsdl.org/projects/SDL_ttf/) - SDL2 Plugin used to load TrueType fonts (.ttf files)
* [Masked Occlusion Culling (MSOC)](https://github.com/GameTechDev/MaskedOcclusionCulling) - Implementation of the MSOC algorithm highlighted in the aforementioned [Intel paper from the write-up](https://software.intel.com/sites/default/files/managed/ef/61/masked-software-occlusion-culling.pdf).

### License

This project is licensed under the Apache License 2.0 - see the [LICENSE.md](LICENSE.md) file for details.

## Installation
### Prerequisites

* OpenGL v4.3 or later. See your graphics card information to verify that you support this.
* SDL2-supported hardware. See [their wiki on installation](http://wiki.libsdl.org/Installation) to verify this.
### Compilation

If you have CMake:

See `CMakeLists.txt` in the root directory.

If you do not, see the windows batch files (.bat) included in the root folder to aid with compilation. If you are not on Windows nor currently have CMake, then you must [install CMake](https://cmake.org/) to use this library.