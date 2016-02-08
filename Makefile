CFLAGS ?= -std=c99 -pedantic -Wall

ifeq ($(inline_unsigned), 1)
 CFLAGS += -DINLINE_UNSIGNED
endif

ifeq ($(page_mmap), 1)
 CFLAGS += -DPAGE_MMAP
endif

ifdef page_size
 CFLAGS += -DPAGE_SIZE=$(page_size)
endif

ifdef optimize
 CFLAGS += -O$(optimize)
else
 CFLAGS += -Os
endif

libintern.a: intern.o block.o optimize.o
	$(AR) rs $@ $^

tests: test.o libintern.a
	$(CC) $(LDFLAGS) $^ -o $@

check: tests
	@./tests

clean:
	rm -f tests *.o *.a *.plist

.PHONY: clean
