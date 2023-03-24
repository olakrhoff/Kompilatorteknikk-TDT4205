.PHONY: all

SRCS = $(wildcard hello.s)

BINS = $(patsubst %.s,%,$(SRCS))

all: $(BINS)

clean:
	rm $(BINS)

%: %.s
	$(CC) $< -o $@
