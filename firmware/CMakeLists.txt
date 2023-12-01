cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CONFIG_TLSF ON)
set(CHIP bl616)

include(proj.conf)

set(CMAKE_PREFIX_PATH "../../../Pkgs/boufallo_sdk")

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})

sdk_add_include_directories(include/usb)
sdk_add_include_directories(include/shell)
sdk_add_include_directories(include/audio)

target_sources(app PRIVATE 
    src/usb/usb_cdc_acm_interface.c
    src/shell/shell.c
    src/audio/audio.c
)
sdk_set_main_file(src/main.c)
sdk_set_vscode_dir(./)

project(nano-revival)
