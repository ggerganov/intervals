/*! \file view-full-gui.cpp
 *  \brief Visualize data recorded with record-full
 *  \author Georgi Gerganov
 */

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#endif

#include "intervals.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <SDL.h>

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <deque>
#include <fstream>
#include <map>
#include <string>
#include <tuple>
#include <vector>
#include <thread>
#include <algorithm>
#include <functional>

static std::function<bool()> g_doInit;
static std::function<bool()> g_mainUpdate;

void mainUpdate() {
    g_mainUpdate();
}

// JS interface

extern "C" {
    int doInit() { return g_doInit(); }
}

// globals
#ifdef __EMSCRIPTEN__
int g_windowSizeX = 1024;
int g_windowSizeY = 400;
#else
int g_windowSizeX = 1024;
int g_windowSizeY = 400;
#endif

IntervalArray generate(int x_max, int wavg_green, int wavg_blue) {
    int x0 = 0;
    int col = rand()%2;
    int wavg[2] = { wavg_blue, wavg_green };
    IntervalArray res;
    printf("Generating ...\n");
    while (true) {
        int r = rand()%wavg[col];
        int w = 0;
        if (col == 1) {
            w = r*r/(wavg[col]) + 1;
            while (rand()%100 > 75) {
                r = rand()%wavg[col];
                w += r*r/(wavg[col]);
            }
        } else {
            if (rand()%2 == 0) {
                w = 1;
            } else {
                w = r*r/(wavg[col]) + 1;
            }
        }
        res.push_back({x0, std::min(x0 + w, x_max), col == 0 ? "blue" : "green"});
        x0 = x0 + w;
        col = 1 - col;
        if (x0 >= x_max) break;
    }
    printf("Done\n");

    return res;
}

void renderIntervals(const IntervalArray & intervals, float xmax, float wsizeX, float wsizeY) {
    auto drawList = ImGui::GetWindowDrawList();

    // background
    {
        auto p0 = ImGui::GetCursorScreenPos();
        auto p1 = p0;
        p1.x += wsizeX;
        p1.y += wsizeY;
        drawList->AddRectFilled(p0, p1, ImGui::ColorConvertFloat4ToU32({ 0.0f, 0.0f, 1.0f, 1.0f }), 0.0f, 0);
    }

    for (auto & i : intervals) {
        if (i.color == "blue") continue;

        auto p0 = ImGui::GetCursorScreenPos();
        auto p1 = p0;
        auto p2 = p0;
        float x0 = i.x0;
        float x1 = i.x1;
        p1.x += wsizeX*(x0/xmax);
        p2.x += wsizeX*(x1/xmax);
        p2.y += wsizeY;
        drawList->AddRectFilled(p1, p2, ImGui::ColorConvertFloat4ToU32({ 0.0f, 1.0f, 0.0f, 1.0f }), 0.0f, 0);
    }

    ImGui::InvisibleButton("", { wsizeX, wsizeY });
}

int main(int argc, char ** argv) {
    srand(time(0));

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    g_doInit = [&]() {
        return true;
    };

#if __APPLE__
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
#ifdef __EMSCRIPTEN__
	SDL_Window* window;
	SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer(g_windowSizeX, g_windowSizeY, SDL_WINDOW_OPENGL, &window, &renderer);
#else
    SDL_Window* window = SDL_CreateWindow("Interval downsampling", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_windowSizeX, g_windowSizeY, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
#endif
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
#ifdef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Init("#version 300 es");
#else
    ImGui_ImplOpenGL3_Init(glsl_version);
#endif

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    ImFontConfig fontConfig;
    //fontConfig.SizePixels = 14.0f;
    ImGui::GetIO().Fonts->AddFontDefault(&fontConfig);

    ImGui::GetStyle().AntiAliasedFill = true;
    ImGui::GetStyle().AntiAliasedLines = true;

    IntervalArray input = generate(768, 3, 40);

    int nDownsample = 15;
    int nDownsampleMax = std::min(200, (int) input.size()/2);
    std::map<float, std::vector<IntervalArray>> cache;
    for (float alpha = 1.0f; alpha <= 16.1f; alpha *= 2.0f) {
        cache[alpha] = downsample(input, nDownsampleMax, alpha);
    }

    bool finishApp = false;
    g_mainUpdate = [&]() {
        if (finishApp) return false;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
                case SDL_QUIT:
                    finishApp = true;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        finishApp = true;
                    }
                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) finishApp = true;
                    break;
            };
        }

        SDL_GetWindowSize(window, &g_windowSizeX, &g_windowSizeY);

        auto tStart = std::chrono::high_resolution_clock::now();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(g_windowSizeX, g_windowSizeY));
        ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        if (ImGui::Button("Generate")) {
            input = generate(768, 3, 40);
            nDownsampleMax = std::min(200, (int) input.size()/2);
            nDownsample = std::min(nDownsample, nDownsampleMax);
            cache.clear();
            for (float alpha = 1.0f; alpha <= 16.1f; alpha *= 2.0f) {
                cache[alpha] = downsample(input, nDownsampleMax, alpha);
            }
        }
        ImGui::Text("Input (%d intervals):", input.size()/2);
        renderIntervals(input, input.back().x1, ImGui::GetContentRegionAvail().x, 24.0f);
        ImGui::Text("%s", "");
        ImGui::Text("Downsampling result using %d intervals:", nDownsample);
        for (auto & res : cache) {
            ImGui::Text("Alpha = %4.2f, F = %d", res.first, res.second[nDownsample].F);
            renderIntervals(res.second[nDownsample], input.back().x1, ImGui::GetContentRegionAvail().x, 24.0f);
        }
        ImGui::Text("%s", "");
        ImGui::Text("N:");
        ImGui::SameLine();
        ImGui::PushItemWidth(400);
        if (ImGui::SliderInt("##N", &nDownsample, 1, nDownsampleMax)) {
        }
        ImGui::PopItemWidth();
        ImGui::End();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Render();
        SDL_GL_MakeCurrent(window, gl_context);
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);

        return true;
    };

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainUpdate, 60, 1);
#else
    if (g_doInit() == false) {
        printf("Error: failed to initialize\n");
        return -2;
    }

    while (true) {
        if (g_mainUpdate() == false) break;
    }
#endif

    printf("[+] Terminated");

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
