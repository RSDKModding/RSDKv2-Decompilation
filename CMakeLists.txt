cmake_minimum_required(VERSION 3.10)

project(RetroEngine)

option(FORCE_CASE_INSENSITIVE "Forces case insensivity." OFF)
option(RETRO_MOD_LOADER "Enables or disables the mod loader." ON)
option(RETRO_USE_ORIGINAL_CODE "Removes any change that differs from the original code, a playable game can't be built this way." OFF)

set(RETRO_SDL_VERSION 2 CACHE STRING "Select between SDL2 and SDL1, defaults to SDL2")

if(RETRO_ORIGINAL_CODE)
    set(RETRO_MOD_LOADER OFF)
endif()

set(RETRO_NAME "RSDKv2")

set(RETRO_OUTPUT_NAME ${RETRO_NAME} CACHE STRING "The exported name of the executable.")

set(RETRO_FILES
	RSDKv2/Animation.cpp
	RSDKv2/Audio.cpp
	RSDKv2/Collision.cpp
	RSDKv2/Debug.cpp
	RSDKv2/Drawing.cpp
	RSDKv2/Ini.cpp
	RSDKv2/Input.cpp
	RSDKv2/fcaseopen.c
	RSDKv2/main.cpp
	RSDKv2/Math.cpp
	RSDKv2/ModAPI.cpp
	RSDKv2/Object.cpp
	RSDKv2/Palette.cpp
	RSDKv2/Player.cpp
	RSDKv2/Reader.cpp
	RSDKv2/RetroEngine.cpp
	RSDKv2/Scene.cpp
	RSDKv2/Script.cpp
	RSDKv2/Sprite.cpp
	RSDKv2/String.cpp
	RSDKv2/Text.cpp
	RSDKv2/Userdata.cpp
	RSDKv2/Video.cpp
)

if(NOT PLATFORM)
    if(WIN32) # THIS ASSUMES VCPKG OR SOURCES !!!!!!!
        set(PLATFORM "Windows" CACHE STRING "The platform to compile for.")
    elseif(ANDROID)
        set(PLATFORM "Android" CACHE STRING "The platform to compile for.")
    else()
        set(PLATFORM ${CMAKE_SYSTEM_NAME} CACHE STRING "The platform to compile for.")
    endif()
endif()

include(platforms/${PLATFORM}.cmake)

set_target_properties(RetroEngine PROPERTIES OUTPUT_NAME ${RETRO_OUTPUT_NAME})

if(COMPILE_OGG)
    set(OGG_DIR dependencies/${DEP_PATH}/libogg)
    add_library(
        libogg
        STATIC
        ${OGG_DIR}/src/bitwise.c
        ${OGG_DIR}/src/framing.c
    )

    target_compile_options(libogg PRIVATE ${OGG_FLAGS})

    target_include_directories(libogg PRIVATE ${OGG_DIR}/include)
    target_include_directories(RetroEngine PRIVATE ${OGG_DIR}/include)
    target_link_libraries(RetroEngine libogg)
endif()

if(COMPILE_VORBIS)
    set(VORBIS_DIR dependencies/${DEP_PATH}/libvorbis)
    set(OGG_DIR dependencies/${DEP_PATH}/libogg)
    add_library(libvorbis STATIC
        ${VORBIS_DIR}/lib/analysis.c
        ${VORBIS_DIR}/lib/barkmel.c
        ${VORBIS_DIR}/lib/bitrate.c
        ${VORBIS_DIR}/lib/block.c
        ${VORBIS_DIR}/lib/codebook.c
        ${VORBIS_DIR}/lib/envelope.c
        ${VORBIS_DIR}/lib/floor0.c
        ${VORBIS_DIR}/lib/floor1.c
        ${VORBIS_DIR}/lib/info.c
        ${VORBIS_DIR}/lib/lookup.c
        ${VORBIS_DIR}/lib/lpc.c
        ${VORBIS_DIR}/lib/lsp.c
        ${VORBIS_DIR}/lib/mapping0.c
        ${VORBIS_DIR}/lib/mdct.c
        ${VORBIS_DIR}/lib/psy.c
        ${VORBIS_DIR}/lib/registry.c
        ${VORBIS_DIR}/lib/res0.c
        ${VORBIS_DIR}/lib/sharedbook.c
        ${VORBIS_DIR}/lib/smallft.c
        ${VORBIS_DIR}/lib/synthesis.c
        ${VORBIS_DIR}/lib/tone.c
        ${VORBIS_DIR}/lib/vorbisenc.c
        ${VORBIS_DIR}/lib/vorbisfile.c
        ${VORBIS_DIR}/lib/window.c
    )

    target_compile_options(libvorbis PRIVATE ${VORBIS_FLAGS})

    target_include_directories(libvorbis
    PRIVATE
    ${VORBIS_DIR}/include
    ${VORBIS_DIR}/lib
    ${OGG_DIR}/include
    )
    target_include_directories(RetroEngine PRIVATE ${VORBIS_DIR}/include)
    target_link_libraries(RetroEngine libvorbis libogg)
endif()

target_include_directories(RetroEngine PRIVATE
    RSDKv2/
)

if(DEFINED DEP_PATH)
    target_include_directories(RetroEngine PRIVATE
        dependencies/${DEP_PATH}/
    )
endif()


target_compile_definitions(RetroEngine PRIVATE
    RETRO_USE_MOD_LOADER=$<BOOL:${RETRO_USE_MOD_LOADER}>
    FORCE_CASE_INSENSITIVE=$<BOOL:${FORCE_CASE_INSENSITIVE}>
    RETRO_USE_ORIGINAL_CODE=$<BOOL:${RETRO_USE_ORIGINAL_CODE}>
)
