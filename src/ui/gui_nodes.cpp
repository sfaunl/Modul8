#include "gui_nodes.h"
#include <vector>
#include <map>

// TODO: remove these:
#include "imgui_internal.h"
#include "../modulator/modulator.h"
#include "../modulation.h"

// TODO: move to modulator.h
static const char* modTypeStr[] = {"ASK", "BPSK", "QPSK", "8PSK", "8QAM", "16QAM", "64QAM", "256QAM", "1024QAM", "4096QAM"};
static const char* modInputStr[] = {"RANDOM", "TAUNT.WAV"};

// TODO: remove this
struct SlotData
{
    void *data;
    int  size;
};

struct Connection
{
    void* InputNode = nullptr;
    const char* InputSlot = nullptr;
    SlotData *InputData = nullptr;
    void* OutputNode = nullptr;
    const char* OutputSlot = nullptr;

    bool operator==(const Connection& other) const
    {
        return InputNode == other.InputNode &&
               InputSlot == other.InputSlot &&
               OutputNode == other.OutputNode &&
               OutputSlot == other.OutputSlot;
    }

    bool operator!=(const Connection& other) const
    {
        return !operator ==(other);
    }
};

enum NodeSlotTypes
{
    NodeSlotByte = 1, // ID can not be 0
    NodeSlotFloat,
    NodeSlotComplex
};

struct MyNode
{
    const char* Title = nullptr;
    bool Selected = false;
    void (*fp)(App *, MyNode*);
    ImVec2 Pos{};
    std::vector<Connection> Connections{};
    std::vector<ImNodes::Ez::SlotInfo> InputSlots{};
    std::vector<ImNodes::Ez::SlotInfo> OutputSlots{};

    explicit MyNode(const char* title,  void (*fpx)(App *, MyNode*),
        const std::vector<ImNodes::Ez::SlotInfo>&& input_slots,
        const std::vector<ImNodes::Ez::SlotInfo>&& output_slots)
    {
        Title = title;
        fp = fpx;
        InputSlots = input_slots;
        OutputSlots = output_slots;
    }

    void DeleteConnection(const Connection& connection)
    {
        for (auto it = Connections.begin(); it != Connections.end(); ++it)
        {
            if (connection == *it)
            {
                Connections.erase(it);
                break;
            }
        }
    }
};

void input(App *app, MyNode* node)
{
    (void)app;
    ImGui::BeginChild(node->Title, ImVec2(170 * ImNodes::GetCurrentCanvas()->Zoom, 60 * ImNodes::GetCurrentCanvas()->Zoom), true, ImGuiWindowFlags_NavFlattened);
    {
        // FIXME: Change data with a circular buffer
        if (!node->OutputSlots[0].datax)
        {
            node->OutputSlots[0].datax = new ImNodes::SlotData();
            node->OutputSlots[0].datax->data = (void*)new uint8_t[200000];
            node->OutputSlots[0].datax->size = 4096;
        }

        static ModInput modInput = MODINPUT_RANDOM;
        ImGui::BeginDisabled(true);
        ImGui::Combo("Input", (int*)&modInput, modInputStr, IM_ARRAYSIZE(modInputStr));
        ImGui::EndDisabled();
        if (modInput == MODINPUT_AUDIO) ImGui::BeginDisabled(true);
        ImGui::SliderInt("bits", &node->OutputSlots[0].datax->size, 10, 10000);
        if (modInput == MODINPUT_AUDIO) ImGui::EndDisabled();

        bool hasOutput = false;
        for (Connection& conn : node->Connections)
        {
            if (conn.OutputNode == node)
            {
                hasOutput = true;
            }
        }

        if (hasOutput)
        {
            switch(modInput)
            {
                case MODINPUT_RANDOM:
                    mod_random_nbits((uint8_t*)node->OutputSlots[0].datax->data, node->OutputSlots[0].datax->size);
                    break;
                case MODINPUT_AUDIO:
                    break;
            }
        }
        ImGui::EndChild();
    }
}

void modulator(App *app, MyNode* node)
{
    (void)app;
    ImGui::BeginChild(node->Title, ImVec2(170 * ImNodes::GetCurrentCanvas()->Zoom, 60 * ImNodes::GetCurrentCanvas()->Zoom), true, ImGuiWindowFlags_NavFlattened);
    {
        if (!node->OutputSlots[0].datax)
        {
            node->OutputSlots[0].datax = new ImNodes::SlotData();
            node->OutputSlots[0].datax->data = (void*)new cmplx[200000];
            node->OutputSlots[0].datax->size = 4096;
        }

        static ModType modType = MOD_256QAM;
        ImGui::Combo("Mod", (int*)&modType, modTypeStr, IM_ARRAYSIZE(modTypeStr));

        uint8_t *input = nullptr;
        int inputSize = 0;
        bool hasInput = false;
        bool hasOutput = false;
        for (Connection& conn : node->Connections)
        {
            if (!conn.InputData)
                conn.InputData = (SlotData*)((MyNode*)conn.OutputNode)->OutputSlots[0].datax;

            if (conn.InputNode == node)
            {
                input = (uint8_t*)conn.InputData->data;
                inputSize = conn.InputData->size;
                hasInput = true;
            }
            if (conn.OutputNode == node)
            {
                hasOutput = true;
            }
        }

        if (hasInput && hasOutput)
        {
            int nbits = modulation_get_symbol_nbits(modType);
            cmplx *constel = modulation_get_constellation_data(modType);
            mod_modulate(input, (cmplx*)node->OutputSlots[0].datax->data, inputSize, nbits, constel);
            node->OutputSlots[0].datax->size = inputSize / nbits;
        }
        ImGui::EndChild();
    }
}
void demodulator(App *app, MyNode* node)
{
    (void)app;
    ImGui::BeginChild(node->Title, ImVec2(170 * ImNodes::GetCurrentCanvas()->Zoom, 60 * ImNodes::GetCurrentCanvas()->Zoom), true, ImGuiWindowFlags_NavFlattened);
    {
        if (!node->OutputSlots[0].datax)
        {
            node->OutputSlots[0].datax = new ImNodes::SlotData();
            node->OutputSlots[0].datax->data = (void*)new uint8_t[200000];
            node->OutputSlots[0].datax->size = 4096;
        }

        static ModType modType = MOD_256QAM;
        ImGui::Combo("Demod", (int*)&modType, modTypeStr, IM_ARRAYSIZE(modTypeStr));

        cmplx *input = nullptr;
        int inputSize = 0;
        bool hasInput = false;
        bool hasOutput = false;
        for (Connection& conn : node->Connections)
        {
            if (!conn.InputData)
                conn.InputData = (SlotData*)((MyNode*)conn.OutputNode)->OutputSlots[0].datax;

            if (conn.InputNode == node)
            {
                input = (cmplx*)conn.InputData->data;
                inputSize = conn.InputData->size;
                hasInput = true;
            }
            if (conn.OutputNode == node)
            {
                hasOutput = true;
            }
        }

        if (hasInput && hasOutput)
        {
            int nbits = modulation_get_symbol_nbits(modType);
            int size = inputSize * nbits;
            cmplx *constel = modulation_get_constellation_data(modType);
            mod_demodulate(input, (uint8_t*)node->OutputSlots[0].datax->data, size, nbits, constel);
            node->OutputSlots[0].datax->size = size;
        }
        ImGui::EndChild();
    }
}
void channel(App *app, MyNode* node)
{
    (void)app;
    ImGui::BeginChild(node->Title, ImVec2(170 * ImNodes::GetCurrentCanvas()->Zoom, 60 * ImNodes::GetCurrentCanvas()->Zoom), true, ImGuiWindowFlags_NavFlattened);
    {
        if (!node->OutputSlots[0].datax)
        {
            node->OutputSlots[0].datax = new ImNodes::SlotData();
            node->OutputSlots[0].datax->data = (void*)new cmplx[200000];
            node->OutputSlots[0].datax->size = 4096;
        }

        static float noiseSNRdB = 20;
        ImGui::SliderFloat("SNR(dB)", &noiseSNRdB, -50.0f, 50.0f);

        cmplx *input = nullptr;
        int inputSize = 0;
        bool hasInput = false;
        bool hasOutput = false;
        for (Connection& conn : node->Connections)
        {
            if (!conn.InputData)
                conn.InputData = (SlotData*)((MyNode*)conn.OutputNode)->OutputSlots[0].datax;

            if (conn.InputNode == node)
            {
                input = (cmplx*)conn.InputData->data;
                inputSize = conn.InputData->size;
                hasInput = true;
            }
            if (conn.OutputNode == node)
            {
                hasOutput = true;
            }
        }

        if (hasInput && hasOutput)
        {
            mod_gaussian_channel(input, (cmplx*)node->OutputSlots[0].datax->data, inputSize, noiseSNRdB);
            node->OutputSlots[0].datax->size = inputSize;
        }
        ImGui::EndChild();
    }
}

void ber(App *app, MyNode* node)
{
    (void)app;
    ImGui::BeginChild(node->Title, ImVec2(170 * ImNodes::GetCurrentCanvas()->Zoom, 60 * ImNodes::GetCurrentCanvas()->Zoom), true, ImGuiWindowFlags_NavFlattened);
    {
        if (!node->OutputSlots[0].datax)
        {
            node->OutputSlots[0].datax = new ImNodes::SlotData();
            node->OutputSlots[0].datax->data = (void*)new float[1];
            node->OutputSlots[0].datax->size = 1;
        }
        
        int inputSize = 0;
        int inputCount = 0;
        for (Connection& conn : node->Connections)
        {
            if (!conn.InputData)
                conn.InputData = (SlotData*)((MyNode*)conn.OutputNode)->OutputSlots[0].datax;

            if (conn.InputNode == node)
            {
                inputSize = conn.InputData->size;
                inputCount++;
            }
        }

        float *output = (float*)node->OutputSlots[0].datax->data;
        if (inputCount == 2)
        {
            float BER = mod_bit_error_rate((uint8_t*)node->Connections[0].InputData->data, (uint8_t*)node->Connections[1].InputData->data, inputSize);

            *output = (*output * 0.75 + BER * 0.25);
            ImGui::Text("BER = %.6f%%", 100 * *output);
        }
        ImGui::EndChild();
    }
}
void xyPlotter(App *app, MyNode* node)
{
    (void)app;
    ImGui::BeginChild(node->Title, ImVec2(250 * ImNodes::GetCurrentCanvas()->Zoom, 200 * ImNodes::GetCurrentCanvas()->Zoom), true, ImGuiWindowFlags_NavFlattened);
    {
        cmplx *input = nullptr;
        int inputSize = 0;
        if (ImPlot::BeginPlot("Constellations", ImVec2(-1, -1), ImPlotFlags_NoTitle)) {
            ImPlot::SetupAxisLimits(ImAxis_X1, -1.5f, 1.5f);
            ImPlot::SetupAxisLimits(ImAxis_Y1, -1.5f, 1.5f);
            ImPlot::SetupLegend(ImPlotLocation_NorthWest, ImPlotLegendFlags_Horizontal);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 1);
            for (Connection& conn : node->Connections)
            {
                if (!conn.InputData)
                    conn.InputData = (SlotData*)((MyNode*)conn.OutputNode)->OutputSlots[0].datax;

                if (conn.InputNode != node)
                    continue;

                input = (cmplx*)conn.InputData->data;
                inputSize = conn.InputData->size;
                ImPlot::PlotScatter(conn.OutputSlot, (float*)input, ((float*)input) + 1, inputSize, 0, 0, sizeof(float) * 2);
            }
            ImPlot::EndPlot();
        }
        ImGui::EndChild();
    }
}
void plotter(App *app, MyNode* node)
{
    (void)app;
    ImGui::BeginChild(node->Title, ImVec2(400 * ImNodes::GetCurrentCanvas()->Zoom, 200 * ImNodes::GetCurrentCanvas()->Zoom), true, ImGuiWindowFlags_NavFlattened);
    {
        uint8_t *input = nullptr;
        int inputSize = 0;
        if (ImPlot::BeginPlot("Bit stream", ImVec2(-1,-1), ImPlotFlags_NoTitle)) {
            ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5f, 1.5f);
            ImPlot::SetupAxisLimits(ImAxis_X1, -1.0, 100.0f);
            ImPlot::SetupLegend(ImPlotLocation_NorthWest, ImPlotLegendFlags_Horizontal);
            ImPlot::SetupAxis(ImAxis_Y1, NULL, ImPlotAxisFlags_Lock);
            for (Connection& conn : node->Connections)
            {
                if (!conn.InputData)
                    conn.InputData = (SlotData*)((MyNode*)conn.OutputNode)->OutputSlots[0].datax;

                if (conn.InputNode != node)
                    continue;

                input = (uint8_t*)conn.InputData->data;
                inputSize = conn.InputData->size;
                ImPlot::PlotStairs(conn.OutputSlot, input, inputSize);
            }
            ImPlot::EndPlot();
        }
        ImGui::EndChild();
    }
}
void meter(App *app, MyNode* node)
{
    (void)app;
    ImGui::BeginChild(node->Title, ImVec2(170 * ImNodes::GetCurrentCanvas()->Zoom, 60 * ImNodes::GetCurrentCanvas()->Zoom), true, ImGuiWindowFlags_NavFlattened);
    {
        float *input = nullptr;
        for (Connection& conn : node->Connections)
        {
            if (!conn.InputData)
                conn.InputData = (SlotData*)((MyNode*)conn.OutputNode)->OutputSlots[0].datax;

            if (conn.InputNode != node)
                continue;

            input = (float*)conn.InputData->data;

            ImGui::Text("%s = %.6f%%", conn.OutputSlot, 100 * *input);
        }
        ImGui::EndChild();
    }
}

std::map<std::string, MyNode*(*)()> available_nodes{
    {"Input", []() -> MyNode* { return new MyNode("Input", input,
        {}, {{"Data", NodeSlotByte}});
    }},
    {"Modulator", []() -> MyNode* { return new MyNode("Modulator", modulator, 
        {{"In", NodeSlotByte}}, {{"Mod", NodeSlotComplex}}); 
    }},
    {"Demodulator", []() -> MyNode* { return new MyNode("Demodulator", demodulator,
        {{"In", NodeSlotComplex}}, {{"Demod", NodeSlotByte}}); 
    }},
    {"Channel", []() -> MyNode* { return new MyNode("Channel", channel,
        {{"In", NodeSlotComplex}}, {{"Chan", NodeSlotComplex}}); 
    }},
    {"BER", []() -> MyNode* { return new MyNode("BER", ber,
        {{"In1", NodeSlotByte}, {"In2", NodeSlotByte}}, {{"BER", NodeSlotFloat}}); 
    }},
    {"XY plotter", []() -> MyNode* { return new MyNode("XY plotter", xyPlotter,
        {{"In[]", NodeSlotComplex}}, {});
    }},
    {"Plotter", []() -> MyNode* { return new MyNode("Plotter", plotter,
        {{"In[]", NodeSlotByte}}, {});
    }},
    {"Meter", []() -> MyNode* { return new MyNode("Meter", meter,
        {{"In[]", NodeSlotFloat}}, {});
    }},
};

struct Blueprint
{
    const char* Title = nullptr;
    std::vector<MyNode*> Blocks{};

    explicit Blueprint(const char* title, std::vector<MyNode*> blocks)
    {
        Title = title;
        Blocks = blocks;
    }

};

Blueprint *newbl = nullptr;
void create_blueprint()
{
    std::vector<MyNode*> blocks;

    MyNode *input = available_nodes["Input"]();
    MyNode *mod = available_nodes["Modulator"]();
    MyNode *chan = available_nodes["Channel"]();
    MyNode *demod = available_nodes["Demodulator"]();
    MyNode *ber = available_nodes["BER"]();
    MyNode *xy1 = available_nodes["XY plotter"]();
    MyNode *xy2 = available_nodes["XY plotter"]();
    MyNode *plot = available_nodes["Plotter"]();
    MyNode *meter = available_nodes["Meter"]();

    Connection conn1{mod, mod->InputSlots[0].title, nullptr, input, input->OutputSlots[0].title};
    mod->Connections.push_back(conn1);
    input->Connections.push_back(conn1);
    Connection conn2{chan, chan->InputSlots[0].title, nullptr, mod, mod->OutputSlots[0].title};
    chan->Connections.push_back(conn2);
    mod->Connections.push_back(conn2);
    Connection conn3{demod, demod->InputSlots[0].title, nullptr, chan, chan->OutputSlots[0].title};
    demod->Connections.push_back(conn3);
    chan->Connections.push_back(conn3);
    Connection conn4{ber, ber->InputSlots[0].title, nullptr, input, input->OutputSlots[0].title};
    ber->Connections.push_back(conn4);
    input->Connections.push_back(conn4);
    Connection conn5{ber, ber->InputSlots[1].title, nullptr, demod, demod->OutputSlots[0].title};
    ber->Connections.push_back(conn5);
    demod->Connections.push_back(conn5);
    Connection conn6{xy1, xy1->InputSlots[0].title, nullptr, mod, mod->OutputSlots[0].title};
    xy1->Connections.push_back(conn6);
    mod->Connections.push_back(conn6);
    Connection conn7{xy2, xy2->InputSlots[0].title, nullptr, chan, chan->OutputSlots[0].title};
    xy2->Connections.push_back(conn7);
    chan->Connections.push_back(conn7);
    Connection conn8{plot, plot->InputSlots[0].title, nullptr, input, input->OutputSlots[0].title};
    plot->Connections.push_back(conn8);
    input->Connections.push_back(conn8);
    Connection conn9{plot, plot->InputSlots[0].title, nullptr, demod, demod->OutputSlots[0].title};
    plot->Connections.push_back(conn9);
    demod->Connections.push_back(conn9);
    Connection conn10{meter, meter->InputSlots[0].title, nullptr, ber, ber->OutputSlots[0].title};
    meter->Connections.push_back(conn10);
    ber->Connections.push_back(conn10);

    input->Pos  = {10,80};
    mod->Pos    = {240,120};
    chan->Pos   = {510,120};
    demod->Pos  = {810,120};
    ber->Pos    = {1080,80};
    xy1->Pos    = {470,230};
    xy2->Pos    = {810,230};
    plot->Pos   = {1150,230};
    meter->Pos  = {1380,80};

    blocks.push_back(input);
    blocks.push_back(mod);
    blocks.push_back(chan);
    blocks.push_back(demod);
    blocks.push_back(ber);
    blocks.push_back(xy1);
    blocks.push_back(xy2);
    blocks.push_back(plot);
    blocks.push_back(meter);

    newbl = new Blueprint("ELK5870 Demo", blocks);
}

void gui_nodes_window(App *app)
{
    static std::vector<MyNode*> *lnodes = nullptr;
    if (!newbl)
    {
        create_blueprint();
        lnodes = &newbl->Blocks;
    }

    if(ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("[Blocks] !experimental!")) {
                app->gui->show_nodes_window = true;
            }
            ImGui::EndMenu();
        }
    ImGui::EndMainMenuBar();
    }
    
    if (app->gui->show_nodes_window){
        static ImNodes::CanvasState *canvas;
        static ImNodes::Ez::Context* context = ImNodes::Ez::CreateContext();
        IM_UNUSED(context);

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(20, main_viewport->WorkPos.y + 40), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x - 40, main_viewport->WorkSize.y - 60), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("[Blocks] !!!EXPERIMENTAL!!!", &app->gui->show_nodes_window, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Close")) app->gui->show_nodes_window = false;
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            // Left
            {
                ImGui::BeginGroup();
                ImGui::BeginChild("Blueprints", ImVec2(150, 0), true);
                ImGui::Text("Select a blueprint:");
                ImGui::Separator();
                {
                    if (ImGui::BeginChild("Blueprintsx", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())))
                    {
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                        if (ImGui::TreeNode("Basic blueprints"))
                        {
                            // for (const auto& desc : available_nodes)
                            // {
                            if (ImGui::SmallButton(newbl->Title))
                            {
                                lnodes = &newbl->Blocks;
                            }
                            // }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
                ImGui::EndGroup();
            }
            ImGui::SameLine();
            // Middle
            {
                ImGui::BeginGroup();
                ImGui::BeginChild("Blocks", ImVec2(-150, 0), true);
                ImGui::Text("Loaded Blueprint: ""%s""", newbl->Title);
                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 100);
                if (ImGui::Button("-")) canvas->Zoom *= 0.9;
                ImGui::SameLine();
                if (ImGui::Button("Zoom")) canvas->Zoom = 1.0;
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Reset Zoom");
                ImGui::SameLine();
                if (ImGui::Button("+")) canvas->Zoom *= 1.1;
                ImGui::Separator();
                {
                    ImGui::BeginChild("Block view");
                    ImNodes::Ez::BeginCanvas();
                    canvas = ImNodes::GetCurrentCanvas();
                    if (lnodes)
                    {
                        for (auto it = lnodes->begin(); it != lnodes->end();)
                        {
                            MyNode* node = *it;

                            if (ImNodes::Ez::BeginNode(node, node->Title, &node->Pos, &node->Selected))
                            {
                                ImNodes::Ez::InputSlots(node->InputSlots.data(), node->InputSlots.size());
                                
                                node->fp(app, node);
                            
                                ImNodes::Ez::OutputSlots(node->OutputSlots.data(), node->OutputSlots.size());

                                Connection new_connection;
                                if (ImNodes::GetNewConnection(&new_connection.InputNode, &new_connection.InputSlot, (ImNodes::SlotData**)&new_connection.InputData,
                                                            &new_connection.OutputNode, &new_connection.OutputSlot))
                                {
                                    ((MyNode*) new_connection.InputNode)->Connections.push_back(new_connection);
                                    ((MyNode*) new_connection.OutputNode)->Connections.push_back(new_connection);
                                }

                                for (const Connection& connection : node->Connections)
                                {
                                    if (connection.OutputNode != node)
                                        continue;

                                    if (!ImNodes::Connection(connection.InputNode, connection.InputSlot, connection.OutputNode,
                                                            connection.OutputSlot))
                                    {
                                        ((MyNode*) connection.InputNode)->DeleteConnection(connection);
                                        ((MyNode*) connection.OutputNode)->DeleteConnection(connection);
                                    }
                                }
                            }
                            ImNodes::Ez::EndNode();

                            if (node->Selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete) && ImGui::IsWindowFocused())
                            {
                                for (auto& connection : node->Connections)
                                {
                                    if (connection.OutputNode == node)
                                    {
                                        ((MyNode*) connection.InputNode)->DeleteConnection(connection);
                                    }
                                    else
                                    {
                                        ((MyNode*) connection.OutputNode)->DeleteConnection(connection);
                                    }
                                }
                                node->Connections.clear();
                                
                                delete node;
                                it = lnodes->erase(it);
                            }
                            else
                                ++it;
                        }

                        if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(1))
                        {
                            ImGui::FocusWindow(ImGui::GetCurrentWindow());
                            ImGui::OpenPopup("NodesContextMenu");
                        }

                        if (ImGui::BeginPopup("NodesContextMenu"))
                        {
                            for (const auto& desc : available_nodes)
                            {
                                if (ImGui::MenuItem(desc.first.c_str()))
                                {
                                    lnodes->push_back(desc.second());
                                    ImNodes::AutoPositionNode(lnodes->back());
                                }
                            }

                            ImGui::Separator();
                            if (ImGui::MenuItem("Reset Zoom"))
                                ImNodes::GetCurrentCanvas()->Zoom = 1;

                            if (ImGui::IsAnyMouseDown() && !ImGui::IsWindowHovered())
                                ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                        }
                    }

                    ImNodes::Ez::EndCanvas();
                    ImGui::EndChild();
                }
                ImGui::EndChild();
                ImGui::EndGroup();
            }
            ImGui::SameLine();
            // Right
            {
                ImGui::BeginGroup();
                ImGui::BeginChild("Blocks pane", ImVec2(150, 0), true);
                ImGui::Text("Number of blocks: %ld", available_nodes.size());
                ImGui::Separator();
                {
                    if (ImGui::BeginChild("Blocks", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())))
                    {
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                        if (ImGui::TreeNode("Basic blocks"))
                        {
                            for (const auto& desc : available_nodes)
                            {
                                if (ImGui::SmallButton(desc.first.c_str()))
                                {
                                    lnodes->push_back(desc.second());
                                }
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
                ImGui::EndGroup();
            }
        }
        ImGui::End();
    }
}