# mkdir build
# cmake -G "MSYS Makefiles"  ../src
# make

if(CMAKE_GENERATOR MATCHES "MSYS")
set(USE_CONSOLE 1)
endif()

message("generator is set to ${CMAKE_GENERATOR}")

cmake_minimum_required(VERSION 2.8)

project(mucom88 CXX)
enable_language(C)

add_executable(mucom88 
    membuf.cpp adpcm.cpp

    md5.c

    soundbuf.cpp
    cmucom.cpp
    mucomvm.cpp
    mucomerror.cpp
    plugin/plugin.cpp

    Z80/Z80.cpp

    fmgen/file.cpp fmgen/fmgen.cpp fmgen/fmtimer.cpp 
    fmgen/opm.cpp fmgen/opna.cpp fmgen/psg.cpp

    callback.cpp
    osdep.cpp

    utils/vgmwrite.cpp
    utils/s98write.cpp
    utils/wavwrite.cpp

    sdl/osdep_sdl.cpp
    sdl/audiobuffer.cpp
    sdl/audiotime.cpp
    main.cpp
)

add_compile_definitions(USE_SDL)

find_package(SDL)

target_include_directories(mucom88 PRIVATE ${SDL_INCLUDE_DIR})
target_link_libraries(mucom88 ${SDL_LIBRARY})

if (USE_CONSOLE)
target_link_options(mucom88 PRIVATE -mconsole)
endif()