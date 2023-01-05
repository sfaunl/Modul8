#include "gui_modulator.h"

// TODO: move to modulator.h
const char* modTypeStr[] = {"ASK", "BPSK", "QPSK", "8PSK", "8QAM", "16QAM", "64QAM", "256QAM", "1024QAM", "4096QAM"};
const char* modInputStr[] = {"RANDOM", "TAUNT.WAV"};

void gui_modulator_main_window(App *app)
{
    Gui *gui = app->gui;
    int symbolSize = modulation_get_symbol_size(app->mod);
    int dataSize = modulation_get_data_size(app->mod);

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
#ifdef DEBUG
        ImGui::SameLine(ImGui::GetWindowWidth() - 250);
        if (ImGui::Button("Demo Window"))
            gui->show_demo_window = true;
#endif //DEBUG
        ImGui::SameLine(ImGui::GetWindowWidth() - 150);
        ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    ImGui::EndMainMenuBar();
    }

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, -1), ImGuiCond_FirstUseEver);
    ImGui::Begin("Modulation Demo");
    {
        if (ImGui::Button("Continue/Pause"))
            app->mod->running ^= true;
        ImGui::Combo("Input", (int*)&app->mod->input, modInputStr, IM_ARRAYSIZE(modInputStr));
        if (app->mod->input == MODINPUT_AUDIO) ImGui::BeginDisabled(true);
        ImGui::SliderInt("symbol/frame", &app->mod->numSymbols, 10, 1000);
        if (app->mod->input == MODINPUT_AUDIO) ImGui::EndDisabled();
        ImGui::Combo("Modulation", (int*)&app->mod->modType, modTypeStr, IM_ARRAYSIZE(modTypeStr));
        ImGui::SliderFloat("SNR(dB)", &app->mod->noiseSNRdB, 0.0f, 50.0f);
        ImGui::Text("Bit Rate = %.2f kbps", dataSize * ImGui::GetIO().Framerate / (1024.0));
        ImGui::Text("Symbol Rate = %.2f ksps", symbolSize * ImGui::GetIO().Framerate / (1000.0));
        ImGui::Text("Bit Error Rate = %.6f%%", 100 * app->mod->bitErrorRate);
        ImGui::Text("Symbol Error Rate = %.6f%%", 100 * app->mod->symbolErrorRate);
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 270), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 350), ImGuiCond_FirstUseEver);
    ImGui::Begin("Constellation Diagram");
    {
        static bool constel = false;
        ImGui::Checkbox("Show constellations", &constel);
        if (ImPlot::BeginPlot("Constellations", ImVec2(-1, -1), ImPlotFlags_NoTitle | ImPlotFlags_NoLegend)) { 
            ImPlot::SetupAxisLimits(ImAxis_X1, -1.5f, 1.5f);
            ImPlot::SetupAxisLimits(ImAxis_Y1, -1.5f, 1.5f);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 1);
            // cmplx* is an array of a structure which has float real and imaginary numbers inside. 
            // memory structure of cmplx is in this way: [RIRIRIRI...]
            // So using stride of two element size and using same pointer twice with one of them with one size of a float offset, we can plot scatter.
            // [RIRIRI], [IRIRI]
            ImPlot::PlotScatter("Modulated Data", 
            (float*)app->mod->rxData, 
            ((float*)app->mod->rxData) + 1, 
            symbolSize, 0, 0, sizeof(float) * 2);

            if (constel)
            {
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Plus, 3);
                ImPlot::PlotScatter(modTypeStr[app->mod->modType], (float*)modulation_get_constellation_data(app->mod),
                ((float*)modulation_get_constellation_data(app->mod)) + 1, modulation_get_symbol_element_size(app->mod), 0, 0, sizeof(float) * 2);
            }
            ImPlot::EndPlot();
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 370, main_viewport->WorkPos.y + 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 250), ImGuiCond_FirstUseEver);
    ImGui::Begin("Bit stream");
    {
        static bool subplots = false;
        ImGui::Checkbox("Enable subplots",&subplots);
        if (subplots)
        {
            static float rratios[] = {0.85,1.15};
            static float cratios[] = {1};
            if (ImPlot::BeginSubplots("Bit stream", 2, 1, ImVec2(-1,-1), 
            ImPlotFlags_NoTitle | ImPlotSubplotFlags_LinkCols | ImPlotSubplotFlags_LinkAllY, 
            rratios, cratios)) {
                if (ImPlot::BeginPlot("",ImVec2(),ImPlotFlags_NoLegend)) {
                    ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5f, 1.5f);
                    ImPlot::SetupAxisLimits(ImAxis_X1, -1.0, dataSize + 1.0f);
                    ImPlot::SetupAxes(NULL,"Input",ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels,
                    ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock);
                    ImPlot::PlotStairs("Input", app->mod->data, dataSize);
                    ImPlot::EndPlot();
                }
                if (ImPlot::BeginPlot("",ImVec2(),ImPlotFlags_NoLegend)) {
                    ImPlot::SetupAxes(NULL,"Output",0,ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock);
                    ImPlot::PlotStairs("Output", app->mod->demodData, dataSize);
                    ImPlot::EndPlot();
                }
                ImPlot::EndSubplots();
            }
        }
        else{
            if (ImPlot::BeginPlot("Bit stream", ImVec2(-1,-1), ImPlotFlags_NoTitle)) {
                ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5f, 1.5f);
                ImPlot::SetupAxisLimits(ImAxis_X1, -1.0, dataSize + 1.0f);
                ImPlot::SetupAxis(ImAxis_Y1, NULL, ImPlotAxisFlags_Lock);
                ImPlot::PlotStairs("Input", app->mod->data, dataSize);
                ImPlot::PlotStairs("Output", app->mod->demodData, dataSize);
                ImPlot::EndPlot();
            }
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 370, main_viewport->WorkPos.y + 300), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 250), ImGuiCond_FirstUseEver);
    ImGui::Begin("Modulated Data");
    {   
        static bool subplots = false;
        ImGui::Checkbox("Enable subplots",&subplots);
        if (subplots)
        {
            static float rratios[] = {0.85,1.15};
            static float cratios[] = {1};
            if (ImPlot::BeginSubplots("Modulated Data", 2, 1, ImVec2(-1,-1), 
            ImPlotFlags_NoTitle | ImPlotSubplotFlags_LinkCols | ImPlotSubplotFlags_LinkAllY, 
            rratios, cratios)) {
                if (ImPlot::BeginPlot("",ImVec2(),ImPlotFlags_NoLegend)) {
                    ImPlot::SetupAxes(NULL,"Modulated",ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels,
                    ImPlotAxisFlags_Lock);
                    ImPlot::PlotLine("Modulated", (float*)app->mod->modData, symbolSize, 1.0, 0.0, 0, sizeof(float)*2);
                    ImPlot::EndPlot();
                }
                if (ImPlot::BeginPlot("",ImVec2(),ImPlotFlags_NoLegend)) {
                    ImPlot::SetupAxisLimits(ImAxis_Y1, -1.5f, 1.5f);
                    ImPlot::SetupAxes(NULL,"Demodulated",ImPlotAxisFlags_None,ImPlotAxisFlags_Lock);
                    ImPlot::PlotLine("Demodulated", (float*)app->mod->rxData, symbolSize, 1.0, 0.0, 0, sizeof(float)*2);
                    ImPlot::EndPlot();
                }
                ImPlot::EndSubplots();
            }
        }
        else
        {
            if (ImPlot::BeginPlot("Modulated Data", ImVec2(-1,-1), ImPlotFlags_NoTitle)) {
                ImPlot::SetupAxisLimits(ImAxis_Y1, -1.5f, 1.5f);
                ImPlot::SetupAxis(ImAxis_Y1, NULL, ImPlotAxisFlags_Lock);
                ImPlot::PlotLine("Modulated", (float*)app->mod->modData, symbolSize, 1.0, 0.0, 0, sizeof(float)*2);
                ImPlot::PlotLine("Demodulated", (float*)app->mod->rxData, symbolSize, 1.0, 0.0, 0, sizeof(float)*2);
                ImPlot::EndPlot();
            }
        }
    }
    ImGui::End();

    // About window
    if (gui->show_about_window)
    {
        ImGui::Begin("About", &gui->show_about_window);
        ImGui::Text("Modulation Demo");
        ImGui::Text("sefaunal.com/p/modulance");
        ImGui::Text("Build time: " __DATE__ " " __TIME__);
        
        if (ImGui::Button("Close"))
            gui->show_about_window = false;
        ImGui::End();
    }
}