#!/bin/bash

g++ -O3 -std=c++11 -I ../ -I ./imgui -I ./imgui/examples/ -I ./imgui/examples/libs/gl3w/ -I/usr/local/Cellar/sdl2/2.0.9/include/SDL2 ./intervals-gui.cpp ./imgui/examples/imgui_impl_opengl3.cpp imgui/examples/imgui_impl_sdl.cpp ./imgui/examples/libs/gl3w/GL/gl3w.c ./imgui/imgui.cpp ./imgui/imgui_draw.cpp ./imgui/imgui_widgets.cpp -lSDL2 -framework OpenGL -framework CoreFoundation
