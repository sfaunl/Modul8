all: x86_release wasm

BUILD_PRE_W = build/wasm
BUILD_PRE_X = build/x86
TEMPLATE_PRE = html_template

SRC = src/main.c \
src/app.c \
src/ui/gui.c \
src/scheduler/linkedlist.c \
src/scheduler/scheduler.c \
src/modulator/modulator.c 
LIBS += -lm -lSDL2 -lGL -lGLU -lGLEW
CFLAGS = -Wall -Wextra

x86_debug:
	mkdir -p $(BUILD_PRE_X)
	gcc $(SRC) -g $(CFLAGS) $(X86DEFINES) $(LIBS) -o $(BUILD_PRE_X)/modulator 

x86_release:
	mkdir -p $(BUILD_PRE_X)
	gcc $(SRC) -O3 $(CFLAGS) $(X86DEFINES) $(LIBS) -o $(BUILD_PRE_X)/modulator

wasm:
	mkdir -p $(BUILD_PRE_W)
	cp $(TEMPLATE_PRE)/index.htm $(BUILD_PRE_W)/.
	emcc $(SRC) \
	$(CFLAGS) \
	$(WASMDEFINES) \
	$(LIBS) \
	-Os \
	-s WASM=1 \
	-s USE_SDL=2 \
	-s ASYNCIFY \
	-o $(BUILD_PRE_W)/modulator.js

clean:
	rm -f $(BUILD_PRE_W)/*
	rm -f $(BUILD_PRE_X)/*
