all: x86_release wasm

BUILD_PRE_W = build/wasm
BUILD_PRE_X = build/x86
TEMPLATE_PRE = html_template

EXE = modulator_imgui
IMGUI_DIR = src/ui/imgui
SRC = src/main.cpp
SRC += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SRC += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

LIBS += $(LINUX_GL_LIBS) -ldl `sdl2-config --libs`
LIBS += -lm -lSDL2 -lGL -lGLU -lGLEW

CXXFLAGS = -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += `sdl2-config --cflags`
CXXFLAGS += -g -Wall -Wextra -Wformat
CXXFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CFLAGS = $(CXXFLAGS)

x86_debug:
	mkdir -p $(BUILD_PRE_X)
	g++ $(SRC) -g $(CFLAGS) $(X86DEFINES) $(LIBS) -o $(BUILD_PRE_X)/$(EXE) 

x86_release:
	mkdir -p $(BUILD_PRE_X)
	g++ $(SRC) -O3 $(CFLAGS) $(X86DEFINES) $(LIBS) -o $(BUILD_PRE_X)/$(EXE)

wasm:
	mkdir -p $(BUILD_PRE_W)
	cp $(TEMPLATE_PRE)/index.htm $(BUILD_PRE_W)/.
	em++ $(SRC) \
	$(CFLAGS) \
	$(WASMDEFINES) \
	$(LIBS) \
	-Os \
	-s WASM=1 \
	-s USE_SDL=2 \
	-s DISABLE_EXCEPTION_CATCHING=1 \
	-s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1 \
	-s ASYNCIFY \
	-o $(BUILD_PRE_W)/modulator.js

clean:
	rm -f $(BUILD_PRE_W)/*
	rm -f $(BUILD_PRE_X)/*
