CC = gcc
CFLAGS = -Wall -Wextra -I./include

# executable files
TARGETS = bin/arping bin/icmping

all: $(TARGETS)

# Target: bin/arping
# Dependencies:
#   - src/bin/arping.c (main source)
#   - src/common/*.c (common utilities)
# $@ - target name
# $^ - all dependencies
# filter-out src/bin/% - exclude all the files belongs to `src/bin/`(only src/bin)
# wildcard src/**/*.c - look for all the `.c` files in `src` and below it(recursive)
bin/arping: src/bin/arping.c $(filter-out src/bin/%,$(wildcard src/**/*.c))
	@mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@

# build icmp
bin/icmping: src/bin/icmping.c $(filter-out src/bin/%,$(wildcard src/**/*.c))
	@mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf bin

.PHONY: all clean