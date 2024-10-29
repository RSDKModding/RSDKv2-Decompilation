# RSDKv2 Decompilation

A complete decompilation of Retro Engine v2.

Without assets from the Sonic Nexus 2008 demo, this decompilation will not run. You can download the game [here](https://info.sonicretro.org/Sonic_Nexus).

# Additional Tweaks
* Added a built in mod loader, allowing to easily create and play mods.
* There is now a `settings.ini` file that the game uses to load all settings, similar to Sonic Mania.
* Dev menu can now be accessed from anywhere by pressing the `ESC` key if enabled in the config.
* The `F12` pause, `F11` step over & fast forward debug features from Sonic Mania have all been ported and are enabled if `devMenu` is enabled in the config.

# How to build
## Windows
* Clone the repo, then follow the instructions in the [depencencies readme for Windows](./dependencies/windows/dependencies.txt) to setup dependencies, then build via the visual studio solution.
* Alternatively, you can grab a prebuilt executable from the releases section.

## Mac
* Clone the repo, follow the instructions in the [depencencies readme for Mac](./dependencies/mac/dependencies.txt) to setup dependencies, then build via the Xcode project.

## Linux
### Make
* To setup your build enviroment and library dependecies, run the following commands:
  * Ubuntu (Mint, Pop!_OS, etc...): `sudo apt install build-essential git libsdl2-dev libvorbis-dev libogg-dev libtheora-dev`
  * Arch Linux: `sudo pacman -S base-devel git sdl2 libvorbis libogg libtheora`
* Clone the repo with the following command: `git clone https://github.com/RSDKModding/RSDKv2-Decompilation.git`
* Go into the repo you just cloned with `cd Sonic-Nexus-Decompilation`.
* Then run `make CXXFLAGS=-O2 -j5`.
  * If your distro is using gcc 8.x.x, then add the argument `LIBS=-lstdc++fs`.
  * The `CXXFLAGS` option can be removed if you do not want optimizations.
  * -j switch is optional, but will make building faster by running it parallel on multiple cores (8 cores would be -j9.)

### CMake
* Install the following dependencies depending on your platform through your terminal:
  * **pacman (Arch):** `sudo pacman -S base-devel cmake glew sdl2 libogg libtheora libvorbis`
  * **apt (Debian/Ubuntu):** `sudo apt install build-essential cmake libglew-dev libglfw3-dev libsdl2-dev libogg-dev libtheora-dev libvorbis-dev`
  * **rpm (Fedora):** `sudo dnf install make gcc cmake glew-devel glfw-devel sdl2-devel libogg-devel libtheora-devel libvorbis-devel zlib-devel`
  * **apk (Alpine/PostmarketOS)** `sudo apk add build-base cmake glew-dev glfw-dev sdl2-dev libogg-dev libtheora-dev libvorbis-dev`
  * Your favorite package manager here, [make a pull request](https://github.com/RSDKModding/RSDKv2-Decompilation/fork)


* Then just run the following in the root repository directory:
  ```
  cmake -B build
  cmake --build build --config release
  ```

## Unofficial Branches
Follow the installation instructions in the readme of each branch.
* For the **Nintendo Switch**, go to [LittlePlanetCD's fork](https://github.com/LittlePlanetCD/Sonic-Nexus-Switch).
  
Because these branches are unofficial, we can't provide support for them and they may not be up-to-date.

## Other Platforms
Currently the only supported platforms are the ones listed above, however the backend uses libogg, libvorbis, libtheora & SDL2 to power it, so the codebase is very multiplatform. If you're able to, you can clone this repo and port it to a platform not on the list.

# Contact:
Join the [Retro Engine Modding Discord Server](https://dc.railgun.works/retroengine) for any extra questions you may need to know about the decompilation or modding it.
