# ===========================================
CC := mpicc
ERRFLAGS := -Wall -Wextra -Wpedantic -Werror
LIBS := -lm

OPT := -O3
CCFLAGS := -std=c99 # -DNDEBUG

SRC_DIR := ./src
OUT_DIR := ./out
BIN_NAME := tspisland
# ===========================================

OBJ_DIR := $(OUT_DIR)/obj
BIN_DIR := $(OUT_DIR)/bin

EXPERIMENTS_DIR := $(OUT_DIR)/experiments

# A makefile hack to make sure out directories exist
_MAKE_OUT_DIRS := $(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) $(EXPERIMENTS_DIR))

SOURCES := $(wildcard $(SRC_DIR)/*.c)

OBJECTS := $(patsubst $(SRC_DIR)%.c, $(OBJ_DIR)%.o, $(SOURCES))
DEPENDS := $(patsubst $(SRC_DIR)%.c, $(OBJ_DIR)%.d, $(SOURCES))

BINARY := $(BIN_DIR)/$(BIN_NAME)

.PHONY: all release experiments clean

all: $(BINARY)

release: $(BINARY)

experiments: $(BINARY)
	bash experiments.sh

clean:
	rm -rf $(OUT_DIR)

$(BINARY): $(OBJECTS)
	$(CC) $(CCFLAGS) $(ERRFLAGS) $(OPT) $^ -o $@ $(LIBS)

-include $(DEPENDS)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c Makefile
	$(CC) $(CCFLAGS) $(ERRFLAGS) $(OPT) -MMD -MP -c $< -o $@ $(LIBS)
