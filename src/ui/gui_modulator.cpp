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
    ImGui::SetNextWindowSize(ImVec2(480, -1), ImGuiCond_FirstUseEver);
    ImGui::Begin("Modulation Demo");
    {
        if (ImGui::Button("Continue/Pause"))
            app->mod->running ^= true;
        ImGui::Combo("Modulation", (int*)&app->mod->modType, modTypeStr, IM_ARRAYSIZE(modTypeStr));
        ImGui::SliderFloat("SNR(dB)", &app->mod->noiseSNRdB, 0.0f, 50.0f);
        ImGui::Text("Bit Error Rate = %.6f%%", 100 * app->mod->bitErrorRate);
        ImGui::Text("Symbol Error Rate = %.6f%%", 100 * app->mod->symbolErrorRate);
    }
    ImGui::End();

    int numSymbols = app->mod->numSymbols;
    int bitLength = numSymbols * (app->mod->modType + 1); // TODO: fix this hack
    uint8_t x_data[bitLength];
    for (int i=0; i<bitLength; i++) x_data[i] = i;
    float x_datad[bitLength];
    for (int i=0; i<bitLength; i++) x_datad[i] = (float)i;

    float real_arrayrx[numSymbols];
    float imag_arrayrx[numSymbols];
    for (int i=0; i<numSymbols; i++) {
        real_arrayrx[i] = real(app->mod->rxData[i]);
        imag_arrayrx[i] = imag(app->mod->rxData[i]);
    }
    float real_array[numSymbols];
    float imag_array[numSymbols];
    for (int i=0; i<numSymbols; i++) {
        real_array[i] = real(app->mod->modData[i]);
        imag_array[i] = imag(app->mod->modData[i]);
    }

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 160), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(480, -1), ImGuiCond_FirstUseEver);
    ImGui::Begin("Constellations");
    {
        if (ImPlot::BeginPlot("Constellations")) { //ImPlotFlags_Equal
            ImPlot::SetupAxisLimits(ImAxis_X1, -7.0f, 7.0f);
            ImPlot::SetupAxisLimits(ImAxis_Y1, -7.0f, 7.0f);
            ImPlot::PlotScatter("Modulated Data", real_arrayrx, imag_arrayrx, numSymbols);
            ImPlot::PlotScatter(modTypeStr[app->mod->modType], real_array, imag_array, numSymbols); // TODO: change modData to a fixed constellation data
            ImPlot::EndPlot();
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 500, main_viewport->WorkPos.y + 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(480, -1), ImGuiCond_FirstUseEver);
    ImGui::Begin("Bit stream");
    {    
        static float rratios[] = {0.85,1.15};
        static float cratios[] = {1};
        if (ImPlot::BeginSubplots("Bit stream", 2, 1, ImVec2(-1,200), 
        ImPlotSubplotFlags_LinkCols | ImPlotSubplotFlags_LinkAllY, 
        rratios, cratios)) {
            if (ImPlot::BeginPlot("",ImVec2(),ImPlotFlags_NoLegend)) {
                ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5f, 1.5f);
                ImPlot::SetupAxisLimits(ImAxis_X1, -1.0, bitLength + 1.0f);
                ImPlot::SetupAxes(NULL,"Input",ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels,
                ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock);
                ImPlot::PlotBars("Input", x_data, app->mod->data, bitLength, 0.98, ImPlotBarsFlags_None);
                ImPlot::EndPlot();
            }
            if (ImPlot::BeginPlot("",ImVec2(),ImPlotFlags_NoLegend)) {
                ImPlot::SetupAxes(NULL,"Output",0,ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock);
                ImPlot::PlotBars("Output", x_data, app->mod->demodData, bitLength, 0.98, ImPlotBarsFlags_None);
                ImPlot::EndPlot();
            }
            ImPlot::EndSubplots();
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 500, main_viewport->WorkPos.y + 280), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(480, -1), ImGuiCond_FirstUseEver);
    ImGui::Begin("Modulated Data");
    {   
        static bool subplots = false;
        ImGui::Checkbox("Enable subplots",&subplots);
        if (subplots)
        {
            static float rratios[] = {0.85,1.15};
            static float cratios[] = {1};
            if (ImPlot::BeginSubplots("Modulated Data", 2, 1, ImVec2(-1,200), 
            ImPlotSubplotFlags_LinkCols | ImPlotSubplotFlags_LinkAllY, 
            rratios, cratios)) {
                if (ImPlot::BeginPlot("",ImVec2(),ImPlotFlags_NoLegend)) {
                    ImPlot::SetupAxes(NULL,"Modulated",ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels,
                    ImPlotAxisFlags_Lock);
                    ImPlot::PlotLine("Modulated", x_datad, real_array, numSymbols);
                    ImPlot::EndPlot();
                }
                if (ImPlot::BeginPlot("",ImVec2(),ImPlotFlags_NoLegend)) {
                    ImPlot::SetupAxisLimits(ImAxis_Y1, -7.0f, 7.0f);
                    ImPlot::SetupAxes(NULL,"Demodulated",ImPlotAxisFlags_None,ImPlotAxisFlags_Lock);
                    ImPlot::PlotLine("Demodulated", x_datad, real_arrayrx, numSymbols);
                    ImPlot::EndPlot();
                }
                ImPlot::EndSubplots();
            }
        }
        else
        {
            if (ImPlot::BeginPlot("Modulated Data", ImVec2(-1,200))) {
                ImPlot::SetupAxisLimits(ImAxis_Y1, -7.0f, 7.0f);
                ImPlot::SetupAxis(ImAxis_Y1, NULL, ImPlotAxisFlags_Lock);
                ImPlot::PlotLine("Modulated", x_datad, real_array, numSymbols);
                ImPlot::PlotLine("Demodulated", x_datad, real_arrayrx, numSymbols);
                ImPlot::EndPlot();
            }
        }
    }
    ImGui::End();

    // 3. Show another simple window.
    if (gui->show_about_window)
    {
        ImGui::Begin("About", &gui->show_about_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Modulation Demo");
        ImGui::Text("sefaunal.com/p/mod_demo");
        ImGui::Text("Build time: " __DATE__ " " __TIME__);
        
        if (ImGui::Button("Close"))
            gui->show_about_window = false;
        ImGui::End();
    }
}