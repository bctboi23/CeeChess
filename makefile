# new makefile
# old makefile was contributed to by servusDei2018 and hamzainaan

CC = gcc
CFLAGS = -O3 -s -Wall
LDFLAGS = -lm

SRC_DIR = .\src
BIN_DIR = .\bin

TARGET = $(BIN_DIR)\CeeChess-v1.4

ifeq ($(OS),Windows_NT)
	RM = del /Q
	EXE_EXTENSION = .exe
else
	RM = rm -rf
	EXE_EXTENSION = -linux
	CFLAGS += -D LINUX
endif

# Automatically discover all source and header files in the ./src directory
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
HEADER_FILES := $(wildcard $(SRC_DIR)/*.h)

all: $(TARGET)$(EXE_EXTENSION)

$(TARGET)$(EXE_EXTENSION): $(SRC_FILES) $(HEADER_FILES)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

# clean the object files and the binary (don't need to clean object files anymore)
clean:
	-$(RM) $(TARGET)$(EXE_EXTENSION)
