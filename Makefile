CC = gcc
# Wall: Warning all
# Wextra: Warning extra
# I./include: Include directory(header files)
CFLAGS = -Wall -Wextra -I./include
LDFLAGS = 

# Directory settings
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# List of executables
EXECUTABLES = arping icmp

# Common source files
COMMON_SRCS = $(wildcard $(SRC_DIR)/common/*.c)
COMMON_OBJS = $(COMMON_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Rules for each executable
all: $(EXECUTABLES)

arping: $(COMMON_OBJS) $(OBJ_DIR)/arping/main.o
	@mkdir -p $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $(BIN_DIR)/$@

icmp: $(COMMON_OBJS) $(OBJ_DIR)/icmp/main.o
	@mkdir -p $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $(BIN_DIR)/$@

# Object file generation rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean