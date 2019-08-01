Curve Detect
-------------

Almost cross-platform (no macOS support) way to convert scan or picture 
of your graph to digital data.

Tested with Manjaro Linux and Windows 7/10 x64



Requirements
------------

To build from source you will need `cmake`.
Supported compilers are gcc (Linux and MinGW) and msvc
(tested with Visual Studio 2015 Community Edition)

Linux is covered in [Linux](#linux) section.
Windows is covered in [Windows](#windows) section.


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

To be able to launch it, you will need to place 3 DLLs to the same folder as the executable
(if they're not accessible through PATH variable). All of them should be available in
 `C:\msysXX\mingw64\bin` (assuming MSYS2 was installed to `C:\msysXX`).
As of time of writing this readme, app requires the following DLLs:
~~~
libgcc_s_seh-1.dll
libstdc++-6.dll
libwinpthread-1.dll
~~~
The easiest way to determine what DLLs are actually required is to copy all DLLs from
`C:\msysXX\mingw64\bin` to the folder with executable and launch it.
While app is working, select everything and delete them.
Windows will refuse to delete loaded libraries while all unused will be safely deleted

##### Visual Studio

1. Install Cmake GUI
2. Create build folder (name it as you like, e.g. `build`) in project root directory
3. Launch Cmake GUI, set source path to project root and set build path to your
created build folder (e.g. `build`)
4. Click `Configure`, after it's done, click `Generate`
5. Open generated solution (`curvedetect.sln`) inside build folder
6. Select desired build type (e.g. `Release`) an build
7. Builded binary should be in build folder inside folder with name of build type
(e.g. `Release`)

Basic usage
-----------

Run the binary `curvedetect` and open desired image. 

Work mode is changed from context menu (right click on opened image and select Points, Horizon or Ticks).

It is better to enable "Show binarization" and tune binarization level in such way that you see you curve while
background is white.

If image is rotated - choose horizon and change default horizontal line to horizontal line of image.

Switch to ticks. Position ticks at the correct location on image (it is better to choose ticks at the ends of curve
for better accuracy). Double click each tick and enter correct value for this tick line.

Switch to points. Add new points by holding Ctrl and clicking with left mouse button. Then drag point to the point on 
curve (hold ctrl to snap point to black pixel in black-white image). Increase subdivision to create extra points between
manually created ones. Subdivided points will automatically snap to black pixels.

After you added all points and you see a correct curve - export it. You can copy to clipboard in text format or export
to file. Export is supported only to Matlab (.mat) format and text formats (any other extension).

Examples
--------
In `examples` directory you can find result of processing two images of the same curve:
`y=x^2+50*x*sin(x/10); x=0:100`
First sample is the rotated screenshot of this curve, second sample is photo of curve, that was tuned in image editor.
Tuning was just color correction, levels and a bit of blur. On verification images blue solid line - original,
red dotted line - processed with curvedetect.
