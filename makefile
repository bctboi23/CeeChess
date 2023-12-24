# new makefile
# old makefile was contributed to by servusDei2018 and hamzainaan

CC = gcc
CFLAGS = -O3 -s -Wall
LDFLAGS = -lm

SRC_DIR = .\src
BIN_DIR = .\bin

TARGET = $(BIN_DIR)\CeeChess-v1.4.5.10

ifeq ($(OS),Windows_NT)
    RM = del /Q
else
    RM = rm -f
endif

# Automatically discover all source and header files in the ./src directory
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
HEADER_FILES := $(wildcard $(SRC_DIR)/*.h)

# Convert source files to object files in the ./bin directory
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC_FILES))

all: $(TARGET).exe

$(TARGET): $(OBJ_FILES)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)
	-$(RM) $(BIN_DIR)\*.o

# Rule for compiling individual source files to object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER_FILES)
	$(CC) -c $< -o $@ $(CFLAGS)

linux: CFLAGS += -D LINUX
linux: $(TARGET)-linux

# clean the object files and the binary (don't need to clean object files anymore)
clean:
	-$(RM) $(BIN_DIR)\*.o 
	-$(RM) $(TARGET)
