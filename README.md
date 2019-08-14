Curve Detect
-------------

Almost cross-platform (no macOS support) way to convert scan or picture 
of your graph to digital data.

`curvedetect` uses OpenGL 3.0 for rendering so it is required to run it.

Tested with Manjaro Linux (gcc) and Windows 7/10 x64 (mingw-gcc and msvc14)



Requirements
------------

To build from source you will need `cmake`.

Supported compilers are `gcc` (Linux and MinGW) and `msvc`
(tested with Visual Studio 2015 Community Edition)

Linux is covered in [Linux](#Linux) section.
Windows is covered in [Windows](#Windows) section.


Building from source
--------------------

### Linux

Since distributions have their own repositories, check them for following packages:
~~~
gcc
make
cmake
~~~
For example, if you're using Arch/Manjaro first you should update existing packages via `pacman -Syu`.
Then install/update required packages:
~~~
pacman --needed -S gcc make cmake
~~~
`gcc` and `make` should be already installed in any Linux distribution but you might need to check that they are up to date.
So you will need to install only `cmake` (if it is not installed already).
After installing all required packages you can just build executable using helper file `build-lnx.sh`:
~~~
./build-lnx.sh
~~~
Just make sure you are at the project root directory.
If it builds successfully, output will be stored in `bin` directory.


### Windows

##### MinGW

1. Install [MSYS2](https://www.msys2.org) somewhere, prefer to install it to `C:\msys64` to avoid any problems
2. Launch MSYS2 console: `C:\msysXX\msys2.exe` (assuming MSYS2 was installed to `C:\msysXX`).
3. Update system by executing `pacman -Syu`
(run this command several times until it will display that there is nothing to update)
4. The compilers are not installed by default, so install them and required packages:
~~~
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-cmake
~~~
Now you should be able to build application with mingw64. Launch mingw64 console:
`C:\msysXX\mingw64.exe` (assuming MSYS2 was installed to `C:\msysXX`).

After that just execute `build-win.sh` helper script while located in project directory to build the app.
If it builds successfully, output will be stored in `bin` directory.
No MinGW DLLs are required to launch executable (static linking is used).

##### Visual Studio

1. Install Cmake GUI
2. Create build folder (name it as you like, e.g. `build`) in project root directory
3. Launch Cmake GUI, set source path to project root and set build path to your
created build folder (e.g. `build`)
4. Click `Configure`, select desired generator (tested with Visual Studio 14 2015, but should work with later versions)
and press `Finish`. Don't forget to set desired build architecture (Win32 or x64)
5. After configuring is done, click `Generate`
6. Open generated solution (`curvedetect.sln`) inside build folder
7. Select desired build type (e.g. `Release`) an build
8. Builded binary should be in build folder inside folder with name of build type
(e.g. `Release`)

Basic usage
-----------

Run the binary `curvedetect` and open desired image. 

Work mode is changed from context menu (right click on opened image and select Points, Grid or Horizon).

It is better to enable "Show binarization" and tune binarization level in such way that you see you curve as thick
as possible while background is white. If your image doesn't have big resolution and curve is very thin
(few pixels thick) it's recommended to manually resize your image to bigger resolution in any image editor
and open it again.

Tune thickness of curve. It should be about the same as actual thickness of curve on the image.
For example, if you have high dpi image, your curve might be 20 pixels thick. If you leave curve thickness at 3 (default),
snapping will be not accurate and you can see that if you zoom in. Points will not be at the center of curve. When you
increase curve thickness, subdivided points will start moving towards the center of curve and finally will be positioned
at its center.

If image is rotated - choose horizon and change default horizontal line to horizontal line of image.

Switch to grid mode. Position grid lines at the correct location on image (it is better to choose grid lines
at the ends of curve for better accuracy). Double click each grid line and enter correct value for this grid line.

Switch to points. Add new points by holding Ctrl and clicking with left mouse button. Then drag it to the point on
curve (hold ctrl to snap point to black pixel in black-white image). By default subdivision is at maximum and gives you
a lot of extra points between manually added ones. Subdivided points are automatically snapped to black pixels. But you
can decrease subdivision if you don't want extra points.

After you added all points and you see a correct curve - export it. You can copy to clipboard in text format or export
to file. Export is supported only to Matlab (.mat) format and text formats (any other extension).

If X or Y axis has logarithmic scale you should set it as such before exporting or you will get wrong results. You can
turn on displaying of minor grid to see automatic grid.

Examples
--------
In `examples` directory you can find result of processing two images of the same curve:
`y=x^2+50*x*sin(x/10); x=0:100`
First sample is the screenshot of this curve, second sample is photo of curve, taken from ~30cm distance (so camera
doesn't focus on pixels). On verification images blue solid line - original, red dashed line - processed with
curvedetect. Both screenshot and photo give relatively similar error - about 6% in maximum. But over the whole range
of X values data from screenshot is more precise than from photo. Due to perspective distortions by phone camera.

Acknowledgements
------------
Curve Detect uses following open source software and resources:

GLFW, zlib license, https://github.com/glfw/glfw

glad, MIT license, https://github.com/Dav1dde/glad

Dear ImGui, MIT license, https://github.com/ocornut/imgui

stb, public domain, https://github.com/nothings/stb

tiny file dialogs, zlib license, https://sourceforge.net/projects/tinyfiledialogs/

Open Sans font, Apache license, https://www.fontsquirrel.com/fonts/open-sans
