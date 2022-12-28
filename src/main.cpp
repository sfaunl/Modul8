#include "app.h"
#include "ui/gui.h"

// Main code
int main(int, char**)
{
    App *app = app_init();
    app->gui = gui_init();
    app->mod = modulator_init();

    // Main loop
    while (app->keepRunning)
    {
        modulator_run(app);
        gui_run(app);
    }

    return 0;
}
