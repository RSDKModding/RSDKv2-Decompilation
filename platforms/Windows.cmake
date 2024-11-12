project(RetroEngine)

add_executable(RetroEngine ${RETRO_FILES})

set(DEP_PATH windows)

find_package(Ogg CONFIG)

if(NOT ${Ogg_FOUND})
    set(COMPILE_OGG TRUE)
    message(NOTICE "libogg not found, attempting to build from source")
else()
    message("found libogg")
    add_library(libogg ALIAS Ogg::ogg)
    target_link_libraries(RetroEngine libogg)
endif()

# i don't like this but it's what's on vcpkg so
find_package(unofficial-theora CONFIG)

if(NOT unofficial-theora_FOUND)
    message(NOTICE "could not find libtheora from unofficial-theora, attempting to find through Theora")
    find_package(Theora CONFIG)

    if(NOT Theora_FOUND)
        message("could not find libtheora, attempting to build manually")
        set(COMPILE_THEORA TRUE)
    else()
        message("found libtheora")
        add_library(libtheora ALIAS Theora::theora) # my best guess
        target_link_libraries(RetroEngine libtheora)
    endif()
else()
    message("found libtheora")
    add_library(libtheora ALIAS unofficial::theora::theora)
    target_link_libraries(RetroEngine libtheora)
endif()

find_package(Vorbis CONFIG)

if(NOT ${Vorbis_FOUND})
    set(COMPILE_VORBIS TRUE)
    message(NOTICE "libvorbis not found, attempting to build from source")
else()
    message("found libvorbis")
    add_library(libvorbis ALIAS Vorbis::vorbisfile)
    target_link_libraries(RetroEngine libvorbis)
endif()

if(RETRO_SDL_VERSION STREQUAL "2")
    find_package(SDL2 CONFIG REQUIRED)
    target_link_libraries(RetroEngine
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )
elseif(RETRO_SDL_VERSION STREQUAL "1")
    find_package(SDL1 CONFIG REQUIRED)
    target_link_libraries(RetroEngine
        $<TARGET_NAME_IF_EXISTS:SDL1::SDL1main>
        $<IF:$<TARGET_EXISTS:SDL1::SDL1>,SDL1::SDL1,SDL1::SDL1-static>
    )
else()
    message(FATAL_ERROR "RETRO_SDL_VERSION must be 1 or 2")
endif()

target_compile_definitions(RetroEngine PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(RetroEngine
    winmm
    comctl32
)

if(RETRO_MOD_LOADER)
    set_target_properties(RetroEngine PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(RetroEngine PRIVATE -Wno-microsoft-cast -Wno-microsoft-exception-spec)
endif()
    
target_sources(RetroEngine PRIVATE ${RETRO_NAME}/${RETRO_NAME}.rc)
target_link_options(RetroEngine PRIVATE /subsystem:windows)