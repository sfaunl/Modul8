#include "app.h"

App* app_init()
{
    App *app = new App;

    app->keepRunning = true;

    app->gui = gui_init();
    app->mod = modulation_init();
    app->audio = audio_init();

    return app;
}

int app_run(void *userArg)
{
    App *app = (App*)userArg;
    // Main loop
    while (app->keepRunning)
    {
        modulation_run(app);
        gui_run(app);
    }

    return 0;
}
