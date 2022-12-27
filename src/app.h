#ifndef _APP_H
#define _APP_H

#include <stdint.h>
#include <stdbool.h>
#include "ui/imgui_renderer.h"

typedef struct{
    bool    keepRunning;
    Imgui   *imgui;
} App;

App* app_init();

int app_run(void *userArg);

#endif // _APP_H
