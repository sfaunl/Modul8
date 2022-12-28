#include "app.h"
#include "ui/gui.h"

// Main code
int main(int, char**)
{
    App *app = app_init();
    app->gui = gui_init();
    app->mod = modulation_init();

    // Main loop
    while (app->keepRunning)
    {
        modulation_run(app);
        gui_run(app);
    }

    return 0;
}
