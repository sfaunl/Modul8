#ifndef _APP_H
#define _APP_H

#include "ui/gui.h"
#include "modulation.h"

typedef struct{
    bool    keepRunning;
    Gui     *gui;
    Mod     *mod;
} App;

App* app_init();

int app_run(void *userArg);

#endif // _APP_H
