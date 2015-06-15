tests: test.o block.o strings.o
	$(CC) $(LDFLAGS) $^ -o $@

check: tests
	@./tests

clean:
	rm -f tests *.o

.PHONY: clean
