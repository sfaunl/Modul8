#include "gui_modulator.h"

// TODO: move to modulator.h
const char* modTypeStr[] = { "BPSK", "QPSK", "8QAM", "16QAM"};

void gui_modulator_main_window(App *app)
{
    Gui *gui = app->gui;

    if(ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Exit")){
                app->keepRunning = false;
            }
            ImGui::EndMenu();
        }
        if(ImGui::MenuItem("About")){
            gui->show_about_window = true;
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 250);
        if (ImGui::Button("Demo Window"))
            gui->show_demo_window = true;
        ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    ImGui::EndMainMenuBar();
    }

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 30), ImGuiCond_FirstUseEver);
    ImGui::Begin("Modulation Demo");
    {
        ImGui::Combo("Modulation", (int*)&app->mod->modType, modTypeStr, IM_ARRAYSIZE(modTypeStr));
        ImGui::SliderFloat("Noise SNR(dB)", &app->mod->noiseSNRdB, 0.0f, 50.0f);
        ImGui::Text("Bit Error Rate = %.6f%%", 100 * app->mod->bitErrorRate);
        ImGui::Text("Symbol Error Rate = %.6f%%", 100 * app->mod->symbolErrorRate);
    }
    ImGui::End();

    int N = 1000;//app->mod->dataLength;
    uint8_t x_data[N] = {3};
    for (int i=0; i<N; i++) x_data[i] = i;
    float x_datad[N] = {1, 0};
    for (int i=0; i<N; i++) x_datad[i] = (float)i;

    float real_arrayrx[N];
    float imag_arrayrx[N];
    for (int i=0; i<N; i++) {
        real_arrayrx[i] = real(app->mod->rxData[i]);
        imag_arrayrx[i] = imag(app->mod->rxData[i]);
    }
    float real_array[N];
    float imag_array[N];
    for (int i=0; i<N; i++) {
        real_array[i] = real(app->mod->modData[i]);
        imag_array[i] = imag(app->mod->modData[i]);
    }

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 150), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(480, 450), ImGuiCond_FirstUseEver);
    ImGui::Begin("Constellations");
    {
        if (ImPlot::BeginPlot("Constellations")) {
            ImPlot::PlotScatter("Modulated Data", real_arrayrx, imag_arrayrx, N);
            ImPlot::PlotScatter(modTypeStr[app->mod->modType], real_array, imag_array, N);
            ImPlot::EndPlot();
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 590, main_viewport->WorkPos.y + 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Data");
    {
        if (ImPlot::BeginPlot("Input bit stream")) {
            ImPlot::PlotStairs("Input", x_data, app->mod->data, N, ImPlotStairsFlags_PreStep);
            ImPlot::PlotStairs("Output", x_data, app->mod->demodData, N, ImPlotStairsFlags_PreStep);
            ImPlot::EndPlot();
        }
        if (ImPlot::BeginPlot("Modulated Data")) {
            ImPlot::PlotLine("Modulated", x_datad, real_array, N);
            ImPlot::PlotLine("Demodulated", x_datad, real_arrayrx, N);
            ImPlot::EndPlot();
        }
    }
    ImGui::End();

    // 3. Show another simple window.
    if (gui->show_about_window)
    {
        ImGui::Begin("About", &gui->show_about_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Modulation Demo");
        ImGui::Text("sefaunal.com/p/mod_demo");
        if (ImGui::Button("Close"))
            gui->show_about_window = false;
        ImGui::End();
    }
}