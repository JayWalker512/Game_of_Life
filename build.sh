#!/bin/bash
#build.sh for game of life
g_target="gameoflife"
g_cc="gcc"
g_srcFiles=("main.c" "threadlife.c" "graphics.c" "lifegraphics.c" "loadfile.c" "dirtyregion.c" "stack.c" "vector3.c" "binary.c" "input.c")
g_cFlags="-std=c99 -Wall -Wextra -pedantic -g -O3 -D_GNU_SOURCE"
g_cLibs="-lm"
g_glLibs="-I/usr/include/GL -lGL -lGLEW" #ubuntu specific?
g_SDL2cFlags="$(sdl-config --cflags)"
g_SDL2Libs="-lSDL2 $(sdl-config --libs)"
g_ccFlags="$g_cFlags $g_cLibs $g_glLibs $g_SDL2cFlags $g_SDL2Libs"

OLDIFS=$IFS
IFS=' '
read -r -a g_cFlagsAry <<< $g_ccFlags
IFS=$OLDIFS

echo "$g_cc ${g_srcFiles[@]} -o $g_target ${g_cFlagsAry[@]}"
$g_cc ${g_srcFiles[@]} -o $g_target ${g_cFlagsAry[@]} 





