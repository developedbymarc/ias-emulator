.PHONY: linux macos win64 clean

LINUX_CC ?= g++
WIN64_CC ?= x86_64-w64-mingw32-g++
MACOS_CC ?= clang++
BIN_DIR = bin
OUT ?= emulator
CFLAGS ?=-O2 -Wall

SOURCES = $(wildcard *.cpp) $(wildcard */*.cpp)

INCLUDES = -IInstructions -ICPU -IUtils -ILexerParser

linux: $(BIN_DIR)/linux
	$(LINUX_CC) $(CFLAGS) $(INCLUDES) -o $(BIN_DIR)/linux/$(OUT).out $(SOURCES)

win64: $(BIN_DIR)/win64
	$(WIN64_CC) $(CFLAGS) $(INCLUDES) -o $(BIN_DIR)/win64/$(OUT).exe $(SOURCES)

macos: $(BIN_DIR)/macos
	$(MACOS_CC) $(CFLAGS) $(INCLUDES) -o $(BIN_DIR)/macos/$(OUT).out $(SOURCES)

$(BIN_DIR)/linux:
	mkdir -p $@

$(BIN_DIR)/win64:
	mkdir -p $@

$(BIN_DIR)/macos:
	mkdir -p $@

clean:
	rm -i $(BIN_DIR)/linux/*.out
	rm -i $(BIN_DIR)/win64/*.exe
	rm -i $(BIN_DIR)/macos/*.out
