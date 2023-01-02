#include "gui_modulator.h"

// TODO: move to modulator.h
const char* modTypeStr[] = { "BPSK", "QPSK", "8QAM", "16QAM", "64QAM"};

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
    ImGui::SetNextWindowSize(ImVec2(350, -1), ImGuiCond_FirstUseEver);
    ImGui::Begin("Modulation Demo");
    {
        if (ImGui::Button("Continue/Pause"))
            app->mod->running ^= true;
        // ImGui::SliderInt("symbol/frame", &app->mod->symbolSize, 10, 10000);
        ImGui::Combo("Modulation", (int*)&app->mod->modType, modTypeStr, IM_ARRAYSIZE(modTypeStr));
        ImGui::SliderFloat("SNR(dB)", &app->mod->noiseSNRdB, 0.0f, 50.0f);
        ImGui::Text("Bit Error Rate = %.6f%%", 100 * app->mod->bitErrorRate);
        ImGui::Text("Symbol Error Rate = %.6f%%", 100 * app->mod->symbolErrorRate);
    }
    ImGui::End();

    int symbolSize = modulation_get_symbol_size(app->mod);
    int dataSize = modulation_get_data_size(app->mod);

    float x_datad[dataSize];
    for (int i=0; i<dataSize; i++) x_datad[i] = (float)i;

    float real_arrayrx[symbolSize];
    float imag_arrayrx[symbolSize];
    for (int i=0; i<symbolSize; i++) {
        real_arrayrx[i] = real(app->mod->rxData[i]);
        imag_arrayrx[i] = imag(app->mod->rxData[i]);
    }
    float real_array[symbolSize];
    float imag_array[symbolSize];
    for (int i=0; i<symbolSize; i++) {
        real_array[i] = real(app->mod->modData[i]);
        imag_array[i] = imag(app->mod->modData[i]);
    }

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 180), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, -1), ImGuiCond_FirstUseEver);
    ImGui::Begin("Constellations");
    {
        if (ImPlot::BeginPlot("Constellations")) { //ImPlotFlags_Equal
            ImPlot::SetupAxisLimits(ImAxis_X1, -2.0f, 2.0f);
            ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0f, 2.0f);
            ImPlot::PlotScatter("Modulated Data", real_arrayrx, imag_arrayrx, symbolSize);
            ImPlot::PlotScatter(modTypeStr[app->mod->modType], real_array, imag_array, symbolSize); // TODO: change modData to a fixed constellation data
            ImPlot::EndPlot();
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 370, main_viewport->WorkPos.y + 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, -1), ImGuiCond_FirstUseEver);
    ImGui::Begin("Bit stream");
    {    
        static bool subplots = false;
        ImGui::Checkbox("Enable subplots",&subplots);
        if (subplots)
        {
            static float rratios[] = {0.85,1.15};
            static float cratios[] = {1};
            if (ImPlot::BeginSubplots("Bit stream", 2, 1, ImVec2(-1,200), 
            ImPlotSubplotFlags_LinkCols | ImPlotSubplotFlags_LinkAllY, 
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
            if (ImPlot::BeginPlot("Bit stream", ImVec2(-1,200))) {
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
    ImGui::SetNextWindowSize(ImVec2(350, -1), ImGuiCond_FirstUseEver);
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
                    ImPlot::PlotLine("Modulated", x_datad, real_array, symbolSize);
                    ImPlot::EndPlot();
                }
                if (ImPlot::BeginPlot("",ImVec2(),ImPlotFlags_NoLegend)) {
                    ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0f, 2.0f);
                    ImPlot::SetupAxes(NULL,"Demodulated",ImPlotAxisFlags_None,ImPlotAxisFlags_Lock);
                    ImPlot::PlotLine("Demodulated", x_datad, real_arrayrx, symbolSize);
                    ImPlot::EndPlot();
                }
                ImPlot::EndSubplots();
            }
        }
        else
        {
            if (ImPlot::BeginPlot("Modulated Data", ImVec2(-1,200))) {
                ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0f, 2.0f);
                ImPlot::SetupAxis(ImAxis_Y1, NULL, ImPlotAxisFlags_Lock);
                ImPlot::PlotLine("Modulated", x_datad, real_array, symbolSize);
                ImPlot::PlotLine("Demodulated", x_datad, real_arrayrx, symbolSize);
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