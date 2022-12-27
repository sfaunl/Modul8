// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "gui.h"
#include "../app.h"

int window_width = 1200;
int window_height = 720;

// Init imgui
Imgui *gui_init()
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

    return imgui_renderer_init("Dear ImGui SDL2+OpenGL3 example", window_width, window_height);
}

void render_gui(App *app)
{
    (void)app;
    // Our state
    static bool show_demo_window = true;
    static bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
}

int gui_run(void *userArg)
{
    (void)userArg;
    App *app = (App*)userArg;

    if(app->keepRunning)
    {
        // Input
        if (imgui_renderer_handle_events(app->imgui)) app->keepRunning = 0;

        // GUI
        render_gui(app);

        // Render
        imgui_renderer_render(app->imgui);
        #ifdef __EMSCRIPTEN__
			emscripten_sleep(0);
		#endif
    }
    else
    {
        imgui_renderer_free(app->imgui);
        return 0;
    }
    
    return 1;
}