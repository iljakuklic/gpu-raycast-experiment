Raycasting on the GPU using the fragment shader
===============================================

This is a demo application that shows various raycasting techniques
implemented as a fragment shader in the GPU.
The resulting image is rendered using just one flat quad.
The third dimension is speceified as a height map texture.
3D look is a result of computations in the fragment shader (GLSL).

The demo app has only been tested on Fedora 16 and Fedora 19, x86\_64 architecture,
libpng v1.5.13.

Build & Run HOWTO
-----------------

Dependencies: OpenGL, glew and libpng libraries including development headers.

Building the application:

    $ cmake . && make

Runnging the application:

    $ ./local_raycast_demo img/small/noise2.png

The `noise2.png` file is the height map to be used. Try also other height maps in the folder.
*Please be patient* as it will take a while before the application really starts.
It has to pre-compute cones for the cone stepping algorithm first.
Normally, the cone map would be saved in a file alongside the height map
but it has not been implemented yet.

Usage
-----

Once the application has started, you will see a screen with two images.
The left one is the rendered scene according to the specified height map.
The one on the right shows relative computational cost of individual pixels.
The redder the pixel, the more steps had the algorithm perform before
it computed the intersection of the ray with the surface.

Following controls can be used:

 * Mouse click and drag rotates the scene
 * Arrow keys also rotate the scene
 * `W`, `S`, `A`, `D`: move the light source by a tiny bit (hold the key for bigger changes)
 * `0`: No height-map-related effects (default)
 * `1`: Normal mapping
 * `2`: Parallax mapping
 * `3`: Linear search
 * `4`: Cone stepping
 * `M`: Toggle the binary search method to find a more accurate ray/surface intersection. Only applicable to linear search and cone stepping.

References
----------

 * http://en.wikipedia.org/wiki/Normal\_mapping
 * http://en.wikipedia.org/wiki/Parallax\_mapping
 * http://http.developer.nvidia.com/GPUGems3/gpugems3\_ch18.html
