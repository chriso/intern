block.o: block.c block.h
strings.o: strings.c strings.h rb.h
tests.o: test.c block.h strings.h

tests: test.o block.o strings.o
	$(CC) $(LDFLAGS) $^ -o $@

check: tests
	@./tests

clean:
	rm -f tests *.o

.PHONY: clean
