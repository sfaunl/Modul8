#include "app.h"
#include "ui/gui.h"

// Main code
int main(int, char**)
{
    App *app = app_init();
    app->imgui = gui_init();

    // Main loop
    while (app->keepRunning)
    {
        gui_run(app);
    }

    return 0;
}
