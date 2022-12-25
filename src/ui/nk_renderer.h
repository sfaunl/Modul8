#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>

//#define NK_INCLUDE_FIXED_TYPES
//#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_sdl_renderer.h"

typedef struct
{
    SDL_Renderer *renderer;
    SDL_Window *window;

    struct nk_context *ctx;

} su_nk;

su_nk su_nuklear_init(int w, int h)
{
    su_nk nk_inst;

    // SDL setup
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(w, h, 0, &nk_inst.window, &nk_inst.renderer);

    // Init Nuklear
    nk_inst.ctx = nk_sdl_init(nk_inst.window, nk_inst.renderer);


    // Load default font
    struct nk_font_atlas *atlas;
    struct nk_font_config config = nk_font_config(0);
    struct nk_font *font;

    nk_sdl_font_stash_begin(&atlas);
    font = nk_font_atlas_add_default(atlas, 13, &config);
    nk_sdl_font_stash_end();
    
    nk_style_set_font(nk_inst.ctx, &font->handle);

    // Load Cursor: if you uncomment cursor loading please hide the cursor
    //nk_style_load_all_cursors(ctx, atlas->cursors);

    return nk_inst;
}

void su_nuklear_render(su_nk *instance, struct nk_colorf *bg_color)
{
    SDL_SetRenderDrawColor(instance->renderer, 
                            bg_color->r * 255, 
                            bg_color->g * 255, 
                            bg_color->b * 255, 
                            bg_color->a * 255);
    SDL_RenderClear(instance->renderer);
    nk_sdl_render(NK_ANTI_ALIASING_ON);
    SDL_RenderPresent(instance->renderer);
}

void su_nuklear_free(su_nk *instance)
{
    SDL_DestroyRenderer(instance->renderer);
    SDL_DestroyWindow(instance->window);
    SDL_Quit();
}

#endif // RENDERER_H
