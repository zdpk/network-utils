CC = gcc
CFLAGS = -Wall -Wextra -I./include

# executable files
TARGETS = bin/arping bin/icmp

all: $(TARGETS)

# build arping
bin/arping: src/arping/main.c src/common/*.c
	@mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@

# build icmp
bin/icmp: src/icmp/main.c src/common/*.c
	@mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf bin

.PHONY: all clean