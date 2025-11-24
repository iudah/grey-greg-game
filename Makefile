# Project name
TARGET = gregory

CC = C:\\msys64\\ucrt64\\bin\\gcc
CFLAGS = -Wall -Wextra -O0 -g -msse2 -msse3 -std=c2x

# Include directories
INCLUDES = \
    -Isource \
    -Isource/game_application \
    -Isource/game_logic \
    -I3rd-party/neon_2_sse \
    -I.build/zot/include/zot.h \
    -I.build/zot/include \
    -I.build/memalloc/include

LIBS = \
    .build/zot/lib/libzot.a \
    .build/memalloc/lib/libmemalloc.a \
    -lws2_32 \
    -lpthread \
    -lm

# Colors
GREEN  := \033[1;32m
BLUE   := \033[1;34m
YELLOW := \033[1;33m
RED    := \033[1;31m
RESET  := \033[0m

# WINDOWS-SAFE DIRECTORY SCANNING ---------------------------------------------

# Recursively collect source files using Make's wildcard + foreach
SRC_DIRS := \
    source \
    source/game_application \
    source/game_logic \
    game

SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

OBJ := $(SRC:.c=.o)

# Build target
$(TARGET): $(OBJ)
	@echo "$(BLUE)[Linking]$(RESET) $@"
	@$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

# Compile rule
%.o: %.c
	@echo "$(GREEN)[Compiling]$(RESET) $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean
clean:
	@echo "$(RED)[Cleaning]$(RESET)"
	@rm -f $(OBJ) $(TARGET)

.PHONY: clean
