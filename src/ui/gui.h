#ifndef _GUI_H
#define _GUI_H

#include "imgui_renderer.h"

typedef struct{
    Imgui   *imgui;
    bool    show_demo_window;
    bool    show_about_window;
} Gui;

Gui *gui_init();
int gui_run(void *userArg);

#endif // _GUI_H
