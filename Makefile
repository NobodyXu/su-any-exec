
CFLAGS ?= -Wall -Werror -std=c11
LDFLAGS ?=

PROG := su-any-exec
SRCS := $(PROG).c

all: $(PROG)

$(PROG): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROG)-static: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ -static $(LDFLAGS)

clean:
	rm -f $(PROG) $(PROG)-static
