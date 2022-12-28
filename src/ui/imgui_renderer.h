#ifndef _IMGUI_RENDERER_H
#define _IMGUI_RENDERER_H

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

struct Imgui
{
    SDL_Renderer    *renderer;
    SDL_GLContext   glCtx;
    SDL_Window      *window;
    ImGuiIO         *imguiIO;
    ImVec4          bgColor;
};

Imgui *imgui_renderer_init(const char * windowName, int windowWidth, int windowHeight);
void imgui_renderer_render(Imgui *imgui);
int imgui_renderer_handle_events(Imgui *imgui);
void imgui_renderer_free(Imgui *imgui);

#endif // _IMGUI_RENDERER_H
