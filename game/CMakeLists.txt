# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
    xGAMEx
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
    DESCRIPTION  "xGAMEx"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

psn00bsdk_add_executable(xGAMEx GPREL source/main.c source/joypad.c source/display.c)

psn00bsdk_add_cd_image(
	iso      # Target name
    xGAMEx # Output file name (= xGAMEx.bin + xGAMEx.cue)
	iso.xml  # Path to config file
    DEPENDS xGAMEx system.cnf
)
