CC = arm-linux-gnueabihf-gcc
CFLAGS = -Iinclude -Wall -static
SRC_DIR = src
BIN_DIR = bin

SOURCES = \
    test_accumulate_dump_csv.c \
    test_accumulate_fullbin.c \
    test_ioctl_accumulate.c \
    test_mmap_accumulate.c

TARGETS = $(patsubst %.c,$(BIN_DIR)/%,$(SOURCES))

all: $(TARGETS)

$(BIN_DIR)/%: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
