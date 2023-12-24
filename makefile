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
    EXE_EXTENSION = .exe
else
    RM = rm -rf
    EXE_EXTENSION = -linux
	CFLAGS += -D LINUX
endif

# Automatically discover all source and header files in the ./src directory
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
HEADER_FILES := $(wildcard $(SRC_DIR)/*.h)

# Convert source files to object files in the ./bin directory
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC_FILES))

# Add dependencies to include header files
DEP_FILES := $(OBJ_FILES:.o=.d)

# Include dependency files
-include $(DEP_FILES)

all: $(TARGET)$(EXE_EXTENSION)

$(TARGET)$(EXE_EXTENSION): $(OBJ_FILES)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)
	-$(RM) $(BIN_DIR)\*.o

# Rule for compiling individual source files to object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER_FILES)
	$(CC) -c $< -o $@ $(CFLAGS)

# clean the object files and the binary (don't need to clean object files anymore)
clean:
	-$(RM) $(BIN_DIR)\*.o 
	-$(RM) $(TARGET)$(EXE_EXTENSION)
