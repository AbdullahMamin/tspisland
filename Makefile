# ===========================================
CC := mpicc
ERRFLAGS := -Wall -Wextra -Wpedantic -Werror
LIBS := -lm

OPT := -O3
CCFLAGS := -std=c99 -DNDEBUG
DEBUG_OPT := -O3
DEBUG_CCFLAGS := -std=c99 -pg

SRC_DIR := ./src
OUT_DIR := ./out
BIN_NAME := tspisland
# ===========================================

OBJ_DIR := $(OUT_DIR)/obj
BIN_DIR := $(OUT_DIR)/bin
DEBUG_OBJ_DIR := $(OUT_DIR)/debug_obj
DEBUG_BIN_DIR := $(OUT_DIR)/debug_bin

EXPERIMENTS_DIR := $(OUT_DIR)/experiments
TESTS_DIR := $(OUT_DIR)/tests

# A makefile hack to make sure out directories exist
_MAKE_OUT_DIRS := $(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DEBUG_OBJ_DIR) $(DEBUG_BIN_DIR) $(EXPERIMENTS_DIR) $(TESTS_DIR))

SOURCES := $(wildcard $(SRC_DIR)/*.c)

OBJECTS := $(patsubst $(SRC_DIR)%.c, $(OBJ_DIR)%.o, $(SOURCES))
DEPENDS := $(patsubst $(SRC_DIR)%.c, $(OBJ_DIR)%.d, $(SOURCES))
DEBUG_OBJECTS := $(patsubst $(SRC_DIR)%.c, $(DEBUG_OBJ_DIR)%.o, $(SOURCES))
DEBUG_DEPENDS := $(patsubst $(SRC_DIR)%.c, $(DEBUG_OBJ_DIR)%.d, $(SOURCES))

BINARY := $(BIN_DIR)/$(BIN_NAME)
DEBUG_BINARY := $(DEBUG_BIN_DIR)/$(BIN_NAME)

.PHONY: all release debug experiments tests clean

all: $(BINARY) $(DEBUG_BINARY)

release: $(BINARY)

debug: $(DEBUG_BINARY)

experiments: $(BINARY)
	bash experiments.sh

tests: $(DEBUG_BINARY)
	bash tests.sh

clean:
	rm -rf $(OUT_DIR)

$(BINARY): $(OBJECTS)
	$(CC) $(CCFLAGS) $(ERRFLAGS) $(OPT) $^ -o $@ $(LIBS)

$(DEBUG_BINARY): $(DEBUG_OBJECTS)
	$(CC) $(DEBUG_CCFLAGS) $(ERRFLAGS) $(DEBUG_OPT) $^ -o $@ $(LIBS)

-include $(DEPENDS)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c Makefile
	$(CC) $(CCFLAGS) $(ERRFLAGS) $(OPT) -MMD -MP -c $< -o $@ $(LIBS)

-include $(DEBUG_DEPENDS)

$(DEBUG_OBJ_DIR)%.o: $(SRC_DIR)%.c Makefile
	$(CC) $(DEBUG_CCFLAGS) $(ERRFLAGS) $(DEBUG_OPT) -MMD -MP -c $< -o $@ $(LIBS)
