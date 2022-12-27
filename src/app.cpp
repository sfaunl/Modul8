#include "app.h"

App* app_init()
{
    App *app = new App;

    app->keepRunning = true;
    app->gui = NULL;
    app->mod = NULL;

    return app;
}