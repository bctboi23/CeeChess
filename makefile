# new makefile
# old makefile was contributed to by servusDei2018 and hamzainaan

CC = gcc
CFLAGS = -O3 -s -Wall -flto
#CFLAGS = -O3 -g -pg -no-pie # use these compile flags if profiling with gprof
CFLAGS = -O3 -s -Wall -fopenmp -flto # use these compile flags if tuning eval and have ability to use OpenMP
LDFLAGS = -lm
VERSION = CeeChess-v2.2

# windows requires backslash
ifeq ($(OS),Windows_NT)
	RM = del /Q
	EXE_EXTENSION = .exe
	SRC_DIR = .\src
	BIN_DIR = .\bin
	TARGET = $(BIN_DIR)\$(VERSION)
else
	RM = rm -rf
	EXE_EXTENSION = -linux 
	CFLAGS += -D LINUX
	SRC_DIR = ./src
	BIN_DIR = ./bin
	TARGET = $(BIN_DIR)/$(VERSION)
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
