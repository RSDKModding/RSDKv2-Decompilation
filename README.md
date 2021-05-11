# Sonic Nexus (2008, RSDK) Decompilation
A Full Decompilation of Sonic Nexus (2008)

# How to build:
## Windows:
* Clone the repo, then follow the instructions in the [depencencies readme for Windows](./dependencies/windows/dependencies.txt) to setup dependencies, then build via the Visual Studio solution
* or grab a prebuilt executable from the releases section

## Windows UWP (Phone, Xbox, etc.):
* Clone the repo, then follow the instructions in the [depencencies readme for Windows](./dependencies/windows/dependencies.txt) and [depencencies readme for UWP](./dependencies/win-uwp/dependencies.txt) to setup dependencies, then build and deploy via the UWP Visual Studio solution.
* After install copy your `Data.rsdk` and `videos` folder into the apps localstate folder

## Windows via MSYS2 (64-bit Only):

* Download the newest version of the MSYS2 installer from [here](https://www.msys2.org/) and install it.
* Run the MINGW64 prompt (from the windows Start Menu/MSYS2 64-bit/MSYS2 MinGW 64-bit), when the program starts enter `pacman -Syuu` in the prompt and hit Enter. Press `Y` when it asks if you want to update packages. If it asks you to close the prompt, do so, then restart it and run the same command again. This updates the packages to their latest versions.
* Now install the dependencies with the following command: `pacman -S pkg-config make git mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-libogg mingw-w64-x86_64-libvorbis mingw-w64-x86_64-libtheora`
* Clone the repo with the following command: `git clone https://github.com/Rubberduckycooly/Sonic-CD-11-Decompilation.git`
* Go into the repo you just cloned with `cd Sonic-CD-11-Decompilation`
* Then run `make CXXFLAGS=-O2 CXX=x86_64-w64-mingw32-g++ STATIC=1 -j5` (-j switch is optional but will make building faster, it's based on the number of cores you have +1 so 8 cores wold be -j9)

## Mac:
* Clone the repo, then follow the instructions in the [depencencies readme for Mac](./dependencies/mac/dependencies.txt) to setup dependencies, then build via the Xcode project
* or grab a prebuilt executable from the releases section

## Linux:
* To setup your build enviroment and library dependecies run the following commands:
* Ubuntu (Mint, Pop!_OS, etc...): `sudo apt install build-essential git libsdl2-dev libvorbis-dev libogg-dev libtheora-dev`
* Arch Linux: `sudo pacman -S base-devel git sdl2 libvorbis libogg libtheora`
* Clone the repo with the following command: `git clone https://github.com/Rubberduckycooly/Sonic-CD-11-Decompilation.git`
* Go into the repo you just cloned with `cd Sonic-CD-11-Decompilation`
* Then run `make CXXFLAGS=-O2 -j5` (-j switch is optional but will make building faster, it's based on the number of cores you have +1 so 8 cores wold be -j9)

## Other platforms:
Currently the only "officially" supported platforms are the ones listed above, however the backend uses libogg, libvorbis, libtheora & SDL2 to power it, so the codebase is very multiplatform.
If you've cloned this repo and ported it to a platform not on the list or made some changes you'd like to see added to this repo, submit a pull request and it'll most likely be added

# Contact:
Join the [Retro Engine Modding Discord Server](https://dc.railgun.works/retroengine) for any extra questions you may need to know about the decompilation or modding it.
