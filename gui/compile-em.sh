#!/bin/bash

em++ -O3 -std=c++11 \
    -s WASM=1 \
    -s ASSERTIONS=1 \
    -s TOTAL_MEMORY=134217728 \
    -s USE_SDL=2 -s USE_WEBGL2=1 -s FULL_ES3=1 \
    -s EXPORTED_FUNCTIONS='["_doInit", "_main"]' \
    -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
    -I ../ \
    -I ./imgui \
    -I ./imgui/examples \
    -I ./imgui/examples/libs/gl3w \
    ./intervals-gui.cpp \
    ./imgui/imgui.cpp \
    ./imgui/imgui_draw.cpp \
    ./imgui/imgui_demo.cpp \
    ./imgui/imgui_widgets.cpp \
    ./imgui/examples/imgui_impl_sdl.cpp \
    ./imgui/examples/imgui_impl_opengl3.cpp \
    -o intervals-downsample-gui.js

cp -v intervals-downsample-gui.js     ../../ggerganov.github.io/scripts/intervals-downsample-gui/
cp -v intervals-downsample-gui.wasm   ../../ggerganov.github.io/scripts/intervals-downsample-gui/
