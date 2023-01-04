#include "app.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

App* app_init()
{
    App *app = new App;

    app->keepRunning = true;

    app->gui = gui_init();
    app->mod = modulation_init();
    app->audio = audio_init();

    return app;
}

void main_loop(void* arg)
{
    App *app = (App*)arg;

    modulation_run(app);
    gui_run(app);
}


int app_run(void *userArg)
{
    App *app = (App*)userArg;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(main_loop, app, 0, 1);
#else
    // Main loop
    while (app->keepRunning)
    {
        main_loop(app);
    }
#endif //__EMSCRIPTEN__

    return 0;
}
