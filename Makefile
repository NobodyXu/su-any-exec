
CFLAGS ?= -Wall -Werror -std=gnu11 $(shell echo 'int main(int argc, char* argv[]) { return 0; }' | $(CC) -x c -Oz - >/dev/null 2>&1 && echo -Oz || echo -Os)
LDFLAGS ?=

PROG := su-any-exec
SRCS := $(PROG).c

all: $(PROG)

$(PROG): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROG)-static: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ -static $(LDFLAGS)

clean:
	rm -f $(PROG) $(PROG)-static a.out
