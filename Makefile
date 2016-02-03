CFLAGS ?= -std=c99 -pedantic -Wall

ifeq ($(inline_unsigned), 1)
 CFLAGS += -DINLINE_UNSIGNED
endif

ifeq ($(mmap), 1)
 CFLAGS += -DBLOCK_MMAP
endif

ifdef page_size
 CFLAGS += -DBLOCK_PAGE_SIZE=$(page_size)
endif

ifdef optimize
 CFLAGS += -O$(optimize)
endif

tests: test.o block.o strings.o
	$(CC) $(LDFLAGS) $^ -o $@

check: tests
	@./tests

clean:
	rm -f tests *.o

.PHONY: clean
