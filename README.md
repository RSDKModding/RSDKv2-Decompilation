# Sonic Nexus (2008, RSDK) Decompilation
A Full Decompilation of Sonic Nexus (2008), a Retro Engine game made by Taxman.

Without assets from the game, this decompilation will not run. You can download the game [here](https://info.sonicretro.org/Sonic_Nexus).

# Additional Tweaks
* Added a built in mod loader, allowing to easily create and play mods.
* There is now a `settings.ini` file that the game uses to load all settings, similar to Sonic Mania.
* Dev menu can now be accessed from anywhere by pressing the `ESC` key if enabled in the config.
* The `F12` pause, `F11` step over & fast forward debug features from Sonic Mania have all been ported and are enabled if `devMenu` is enabled in the config.

This project uses [CMake](https://cmake.org/), a versatile building system that supports many different compilers and platforms. You can download CMake [here](https://cmake.org/download/). **(Make sure to enable the feature to add CMake to the system PATH during the installation!)**

## Get the source code

In order to clone the repository, you need to install Git, which you can get [here](https://git-scm.com/downloads).

Clone the repo, using:
`git clone https://github.com/Rubberduckycooly/Sonic-Nexus-Decompilation.git`

If you've already cloned the repo, run this command inside of the repository:
```git submodule update --init```

## Follow the build steps

### Windows
To handle dependencies, you'll need to install [Visual Studio Community](https://visualstudio.microsoft.com/downloads/) (make sure to install the `Desktop development with C++` package during the installation) and [vcpkg](https://github.com/microsoft/vcpkg#quick-start-windows).

After installing those, run the following in Command Prompt (make sure to replace `[vcpkg root]` with the path to the vcpkg installation!):
- `[vcpkg root]\vcpkg.exe install glew sdl2 libogg libtheora libvorbis --triplet=x64-windows-static` (If you're compiling a 32-bit build, replace `x64-windows-static` with `x86-windows-static`.)

Finally, follow the [compilation steps below](#compiling) using `-DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_PREFIX_PATH=[vcpkg root]/installed/x64-windows-static/` as arguments for `cmake -B build`.
  - Make sure to replace each instance of `[vcpkg root]` with the path to the vcpkg installation!
  - If you're compiling a 32-bit build, replace each instance of `x64-windows-static` with `x86-windows-static`.

### Mac
* Clone the repo, follow the instructions in the [depencencies readme for Mac](./dependencies/mac/dependencies.txt) to setup dependencies, then build via the Xcode project.

### Linux
Install the following dependencies: then follow the [compilation steps below](#compiling):
- **pacman (Arch):** `sudo pacman -S base-devel cmake glew sdl2 libogg libtheora libvorbis`
- **apt (Debian/Ubuntu):** `sudo apt install build-essential cmake libglew-dev libglfw3-dev libsdl2-dev libogg-dev libtheora-dev libvorbis-dev`
- **rpm (Fedora):** `sudo dnf install make gcc cmake glew-devel glfw-devel sdl2-devel libogg-devel libtheora-devel libvorbis-devel zlib-devel`
- **apk (Alpine/PostmarketOS)** `sudo apk add build-base cmake glew-dev glfw-dev sdl2-dev libogg-dev libtheora-dev libvorbis-dev`
- Your favorite package manager here, [make a pull request](https://github.com/Rubberduckycooly/Sonic-Nexus-Decompilation/fork/)
  
### Compiling

Compiling is as simple as typing the following in the root repository directory:
```
cmake -B build
cmake --build build --config release
```

The resulting build will be located somewhere in `build/` depending on your system.

The following cmake arguments are available when compiling:
- Use these on the first `cmake -B build` step like so: `cmake -B build -DRETRO_DISABLE_PLUS=on`

## Unofficial Branches
Follow the installation instructions in the readme of each branch.
* For the **Nintendo Switch**, go to [LittlePlanetCD's fork](https://github.com/LittlePlanetCD/Sonic-Nexus-Switch).
  
Because these branches are unofficial, we can't provide support for them and they may not be up-to-date.

## Other Platforms
Currently the only supported platforms are the ones listed above, however the backend uses libogg, libvorbis, libtheora & SDL2 to power it, so the codebase is very multiplatform. If you're able to, you can clone this repo and port it to a platform not on the list.

# Contact:
Join the [Retro Engine Modding Discord Server](https://dc.railgun.works/retroengine) for any extra questions you may need to know about the decompilation or modding it.
