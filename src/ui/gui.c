
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "../app.h"
#include "nk_renderer.h"
#include "nuklear/canvas.c"
#include "nuklear/overview.c"

#include "gui.h"

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

void quit(su_nk *instance)
{
    nk_sdl_shutdown();
    su_nuklear_free(instance);
}

int handle_events(su_nk *instance)
{
    SDL_Event evt;
    nk_input_begin(instance->ctx);
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_QUIT) return 1;
        nk_sdl_handle_event(&evt);
    }
    nk_input_end(instance->ctx);

    return 0;
}

void render_gui(struct nk_context *ctx, App *app)
{
    (void)app;
    canvas(ctx);
    overview(ctx);
}

int window_width = 1200;
int window_height = 800;
su_nk nk_inst;

// Init Nuklear
void init_gui()
{
    // get html5 canvas size
#ifdef __EMSCRIPTEN__
    window_width = EM_ASM_INT_V(return window.innerWidth 
        || document.documentElement.clientWidth 
        || document.body.clientWidth
    );
    window_height = EM_ASM_INT_V(return window.innerHeight
        || document.documentElement.clientHeight
        || document.body.clientHeight
    );
    window_width = (window_width * 7) / 8;
    window_height = (window_height * 7) / 8;
#endif

    nk_inst = su_nuklear_init(window_width, window_height);
}

int gui_run(void *userArg)
{
    App *app = (App*)userArg;

    struct nk_colorf bg = { .r = 0.10f,
                            .g = 0.18f,
                            .b = 0.24f,
                            .a = 1.0f};        

    if(app->keepRunning)
    {
        // Input
        if (handle_events(&nk_inst)) app->keepRunning = 0;

        // GUI
        render_gui(nk_inst.ctx, app);

        // Render
        su_nuklear_render(&nk_inst, &bg);

        #ifdef __EMSCRIPTEN__
			emscripten_sleep(0);
		#endif
    }
    else
    {
        quit(&nk_inst);
        return 0;
    }
    
    return 1;
}