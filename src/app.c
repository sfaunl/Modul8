#include <stdio.h>
#include <stdlib.h>

#include "app.h"

App *app_init()
{
    App *app = malloc(sizeof(App));
    app->keepRunning = 1;
    app->name = "Modulator Demo";

    printf("App %s started\n", app->name);

    return app;
}

int app_run(App *app)
{
    if(app->keepRunning)
    {
    }
    else
    {
        return 0;
    }

    return 1;
}
