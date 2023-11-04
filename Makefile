## User configuration ##
CC ?= cc
CCFLAGS ?= -march=native -O2 -pipe


## Developer configuration ##
CCFLAGS := $(CCFLAGS) -Wall -Wextra -Werror -Wformat-security \
		-Wpedantic -pedantic-errors -std=c18

SRC_FILES := $(shell find examples/ -name "*.c")
OBJ_FILES := ${SRC_FILES:.c=}


## User targets ##
.PHONY: build run clean

build: $(OBJ_FILES)

run: build
	@for f in $(OBJ_FILES); do \
		printf "Run     $$f\n"; \
		"./$$f"; \
	done

clean:
	@rm -rf $(OBJ_FILES)


## Developer targets ##
examples/%: examples/%.c
	@printf "CC      $@\n"
	@$(CC) $(CCFLAGS) -g -o $@ $<
