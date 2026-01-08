cmake_minimum_required(VERSION 3.25)
project(umg C)

set(CMAKE_C_STANDARD 99)

# Define platform before adding raylib
set(PLATFORM "Android" CACHE STRING "Platform")

# Android glue path
set(ANDROID_NATIVE_APP_GLUE
        ${ANDROID_NDK}/sources/android/native_app_glue)

# Add raylib
add_subdirectory(raylib)

# Your app
add_library(umg SHARED
        main.c
        chat.c
        ${ANDROID_NATIVE_APP_GLUE}/android_native_app_glue.c
)

# Include paths for YOUR app
target_include_directories(umg PRIVATE
        ${ANDROID_NATIVE_APP_GLUE}
        ${CMAKE_CURRENT_SOURCE_DIR}/raylib/src
)

# Link everything needed
target_link_libraries(umg
        raylib
        android
        log
        EGL
        GLESv2
        OpenSLES
)
