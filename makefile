# compiler flags
CC       := gcc
CFLAGS   := -Wall -Wextra -Iinclude
LDFLAGS  := -Llib
LDLIBS   := -lncurses -lpthread
LUA      := lua

# directory
SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build
OUT_DIR   := out
LIB_DIR   := lib

TARGET    := $(OUT_DIR)/strngs

# find all .c files in src/
SRCS := $(wildcard $(SRC_DIR)/*.c)

# transform src/name.c in build/name.o
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# autogen .d files
DEPS := $(OBJS:.o=.d)

.PHONY: all clean run json test

all: $(TARGET)

$(TARGET): $(OBJS) | $(OUT_DIR)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# compile .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# autogen of missing (if any) folders
$(BUILD_DIR) $(OUT_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(OUT_DIR)

json: clean
	bear -- $(MAKE) all

-include $(DEPS)
