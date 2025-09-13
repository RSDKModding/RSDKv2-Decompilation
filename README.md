# RSDKv2 Decompilation

A complete decompilation of the Retro-Sonic Engine v2.

Without assets from the Sonic Nexus 2008 demo, this decompilation will not run. You can download the game [here](https://info.sonicretro.org/Sonic_Nexus).

# Additional Tweaks
* Added a built in mod loader, allowing to easily create and play mods.
* There is now a `settings.ini` file that the game uses to load all settings, similar to Sonic Mania.
* The dev menu can now be accessed from anywhere by pressing the `ESC` key if enabled in the config.
* The `F12` pause, `F11` step over & fast forward debug features from Sonic Mania have all been ported and are enabled if `devMenu` is enabled in the config.

# How to Build

This project uses [CMake](https://cmake.org/), a versatile building system that supports many different compilers and platforms. You can download CMake [here](https://cmake.org/download/). **(Make sure to enable the feature to add CMake to the system PATH during the installation!)**

## Get the source code

**DO NOT** download the source code ZIP archive from GitHub, as they do not include the submodules required to build the decompilation.

Instead, you will need to clone the repository using Git, which you can get [here](https://git-scm.com/downloads).

Clone the repo, using:
`git clone --recursive https://github.com/RSDKModding/RSDKv2-Decompilation`

## Getting dependencies

### Windows
To handle dependencies, you'll need to install [Visual Studio Community](https://visualstudio.microsoft.com/downloads/) (make sure to install the `Desktop development with C++` package during the installation) and [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-cmd#1---set-up-vcpkg) (You only need to follow `1 - Set up vcpkg`).

After installing those, run the following in Command Prompt (make sure to replace `[vcpkg root]` with the path to the vcpkg installation!):
- `[vcpkg root]\vcpkg.exe install sdl2 libogg libvorbis --triplet=x64-windows-static` (If you're compiling a 32-bit build, replace `x64-windows-static` with `x86-windows-static`.)

Finally, follow the [compilation steps below](#compiling) using `-DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_PREFIX_PATH=[vcpkg root]/installed/x64-windows-static/` as arguments for `cmake -B build`.
  - Make sure to replace each instance of `[vcpkg root]` with the path to the vcpkg installation!
  - If you're compiling a 32-bit build, replace each instance of `x64-windows-static` with `x86-windows-static`.

### Mac
* Clone the repo, follow the instructions in the [depencencies readme for Mac](./dependencies/mac/dependencies.txt) to setup dependencies, then build via the Xcode project.

### Linux
Install the following dependencies: then follow the [compilation steps below](#compiling):
- **pacman (Arch):** `sudo pacman -S base-devel cmake sdl2 libogg libvorbis`
- **apt (Debian/Ubuntu):** `sudo apt install build-essential cmake libsdl2-dev libogg-dev libvorbis-dev`
- **rpm (Fedora):** `sudo dnf install make gcc cmake sdl2-devel libogg-devel libvorbis-devel zlib-devel`
- **apk (Alpine/PostmarketOS)** `sudo apk add build-base cmake sdl2-dev libogg-dev libvorbis-dev`
- Your favorite package manager here, [make a pull request](https://github.com/RSDKModding/RSDKv2-Decompilation/fork)

## Compiling

Compiling is as simple as typing the following in the root repository directory:
```
cmake -B build
cmake --build build --config release
```

The resulting build will be located somewhere in `build/` depending on your system.

The following cmake arguments are available when compiling:
- Use these by adding `-D[flag-name]=[value]` to the end of the `cmake -B build` command.

### RSDKv2 flags
- `FORCE_CASE_INSENSITIVE`: Forces case insensivity when loading files. Takes a boolean, defaults to `off`.
- `RETRO_MOD_LOADER`: Enables or disables the mod loader. Takes a boolean, defaults to `on`.
- `RETRO_ORIGINAL_CODE`: Removes any custom code. *A playable game will not be built with this enabled.* Takes a boolean, defaults to `off`.
- `RETRO_SDL_VERSION`: *Only change this if you know what you're doing.* Switches between using SDL1 or SDL2. Takes an integer of either `1` or `2`, defaults to `2`.

## Unofficial Branches
Follow the installation instructions in the readme of each branch.
* For the **Nintendo Switch**, go to [LittlePlanetCD's fork](https://github.com/LittlePlanetCD/Sonic-Nexus-Switch).
* For **WebAssembly**, go to [Jdsle's fork](https://github.com/Jdsle/RSDKv2-Decompilation/tree/web).

Because these branches are unofficial, we can't provide support for them and they may not be up-to-date.

## Other Platforms
Currently the only supported platforms are the ones listed above, however the backend uses libogg, libvorbis, & SDL2 to power it, so the codebase is very multiplatform.
If you're able to, you can clone this repo and port it to a platform not on the list.

# Contact:
Join the [Retro Engine Modding Discord Server](https://dc.railgun.works/retroengine) for any extra questions you may need to know about the decompilation or modding it.
