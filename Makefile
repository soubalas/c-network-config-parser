
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -g
DEPFLAGS = -MMD -MP

TARGET = network_config
SOURCES = main.c parser.c
OBJECTS = $(SOURCES:.c=.o)
DEPS = $(OBJECTS:.o=.d)

TEST_TARGET = test_parser
TEST_SOURCE = tests/test_parser.c
TEST_OBJECT = tests/test_parser.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(TEST_TARGET): $(TEST_OBJECT) parser.o
	$(CC) $(TEST_OBJECT) parser.o -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

-include $(DEPS) $(TEST_OBJECT:.o=.d)

clean:
	rm -f $(OBJECTS) $(DEPS) $(TEST_OBJECT) $(TEST_OBJECT:.o=.d) $(TARGET) $(TEST_TARGET)

# Rebuild and run tests with DEBUG logging enabled.
.PHONY: debug

debug:
	$(MAKE) clean
	$(MAKE) test CFLAGS="$(CFLAGS) -DDEBUG"

# Rebuild and run tests with memory and undefined-behavior sanitizers.
.PHONY: sanitize

sanitize:
	$(MAKE) clean
	$(MAKE) test CFLAGS="$(CFLAGS) -O1 -fno-omit-frame-pointer -fsanitize=address,undefined" LDFLAGS="$(LDFLAGS) -fsanitize=address,undefined"
