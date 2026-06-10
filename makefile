DIST_DIR = dist
TARGET = $(DIST_DIR)/main

all: build
	$(MAKE) run

build: $(TARGET)

$(TARGET): main.c | $(DIST_DIR)
	gcc -lraylib $^ -o $@

$(DIST_DIR):
	mkdir -p $@

run:
	$(TARGET)

clean:
	rm -rf $(TARGET)

.PHONY: all build run clean
