#include "app.h"

// Main code
int main(int, char**)
{
    App *app = app_init();

    app_run(app);

    return 0;
}
