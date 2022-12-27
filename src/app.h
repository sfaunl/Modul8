#ifndef _APP_H
#define _APP_H

#include <stdint.h>
#include <stdbool.h>
#include "ui/gui.h"

typedef struct{
    bool    keepRunning;
    Gui     *gui;
} App;

App* app_init();

int app_run(void *userArg);

#endif // _APP_H
