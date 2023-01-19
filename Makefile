
all: wasm-release x86-release

EXE = modulator
WASM_TEMPLATE_FILE = html_template/index.htm
BUILD_DIR = build
IMGUI_DIR = src/ui/imgui
IMPLOT_DIR = src/ui/implot

SRC = src/main.cpp \
src/app.cpp \
src/modulation.cpp \
src/audio.cpp \
src/ui/imgui_renderer.cpp \
src/ui/gui.cpp \
src/ui/gui_modulator.cpp 
SRC += src/modulator/modulator.cpp
SRC += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SRC += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SRC += $(IMPLOT_DIR)/implot.cpp $(IMPLOT_DIR)/implot_items.cpp

OBJS = $(addprefix $(BUILD_DIR)/obj/,$(notdir $(SRC:.cpp=.o)))
vpath %.cpp $(sort $(dir $(SRC)))

LIBS += -ldl `sdl2-config --libs`
LIBS += -lm -lGL

CXXFLAGS 				+= $(CXXFLAGS_EXT)
CXXFLAGS 				+= -std=c++11
CXXFLAGS 				+= -Wall -Wextra -Wformat -Wformat -pedantic
CXXFLAGS 				+= `sdl2-config --cflags`
CXXFLAGS 				+= -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS 				+= -I$(IMPLOT_DIR)
CXXFLAGS 				+= -DImDrawIdx='unsigned int'

LDFLAGS 				+= $(LDFLAGS_EXT)
LDFLAGS 				+= -Wl,--gc-sections

# x86 target flags
CXXFLAGS.x86-debug 		= -O0 -g3 -DDEBUG
CXXFLAGS.x86-release 	= -Oz -s

# wasm target flags
CXXFLAGS.wasm-debug	 	+= -Os -g -DDEBUG
CXXFLAGS.wasm-debug 	+= -fsanitize=address
CXXFLAGS.wasm-release 	+= -Oz -s
CXXFLAGS.wasm			+= -s USE_SDL=2
LDFLAGS.wasm-debug 		+= -s ASSERTIONS=2
LDFLAGS.wasm-release 	+= -s ASSERTIONS=1
LDFLAGS.wasm 			+= -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY
LDFLAGS.wasm 			+= --preload-file assets/wav/taunt.wav

BUILD_DIR.x86-debug 	= build/x86-debug
BUILD_DIR.x86-release 	= build/x86-release
BUILD_DIR.wasm-debug 	= build/wasm-debug
BUILD_DIR.wasm-release 	= build/wasm-release


# recursively call makefile with new variables
x86-debug:
	@$(MAKE) $@ TARGET=$@ BUILD_DIR="$(BUILD_DIR.x86-debug)"    CXX="g++"  CXXFLAGS_EXT="$(CXXFLAGS.x86) $(CXXFLAGS.x86-debug)"     LDFLAGS_EXT="$(LDFLAGS.x86) $(LDFLAGS.x86-debug)"
	size $(BUILD_DIR.x86-debug)/bin/$(EXE)
x86-release:
	@$(MAKE) $@ TARGET=$@ BUILD_DIR="$(BUILD_DIR.x86-release)"  CXX="g++"  CXXFLAGS_EXT="$(CXXFLAGS.x86) $(CXXFLAGS.x86-release)"   LDFLAGS_EXT="$(LDFLAGS.x86) $(LDFLAGS.x86-release)"
	size $(BUILD_DIR.x86-release)/bin/$(EXE)
wasm-debug:
	@$(MAKE) $@ TARGET=$@ BUILD_DIR="$(BUILD_DIR.wasm-debug)"   CXX="em++" CXXFLAGS_EXT="$(CXXFLAGS.wasm) $(CXXFLAGS.wasm-debug)"   LDFLAGS_EXT="$(LDFLAGS.wasm) $(LDFLAGS.wasm-debug)"   EXE="$(EXE).js"
	cp $(WASM_TEMPLATE_FILE) $(BUILD_DIR.wasm-debug)/bin/.
wasm-release:
	@$(MAKE) $@ TARGET=$@ BUILD_DIR="$(BUILD_DIR.wasm-release)" CXX="em++" CXXFLAGS_EXT="$(CXXFLAGS.wasm) $(CXXFLAGS.wasm-release)" LDFLAGS_EXT="$(LDFLAGS.wasm) $(LDFLAGS.wasm-release)" EXE="$(EXE).js"
	cp $(WASM_TEMPLATE_FILE) $(BUILD_DIR.wasm-release)/bin/.

$(TARGET): $(BUILD_DIR)/bin/$(EXE) | $(BUILD_DIR)/bin
	echo $@

$(BUILD_DIR)/obj/%.o: %.cpp | $(BUILD_DIR)/obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/bin/$(EXE): $(OBJS) | $(BUILD_DIR)/bin
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(BUILD_DIR)/bin:
	mkdir -p $@
	
$(BUILD_DIR)/obj:
	mkdir -p $@

.PHONY : all clean x86-debug x86-release wasm-debug wasm-release

clean:
	rm -rf $(BUILD_DIR)