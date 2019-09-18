.SECONDARY:

PYTHON := python3

EXAMPLE_C_SOURCE := example3.c

LIBCLANG_PATH=/usr/local/Cellar/llvm/8.0.1/lib/libclang.dylib
LIBCLANG_PATH=/usr/lib/llvm-8/lib/libclang.so

CLANG_FORMAT=/usr/lib/llvm-8/bin/clang-format

instrument: | instrumented
	LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py examples/$(EXAMPLE_C_SOURCE) | $(CLANG_FORMAT) > instrumented/$(EXAMPLE_C_SOURCE)


instrumented: ; mkdir -p $@

instrumented/%.c: examples/%.c src/instrument.py | instrumented
	LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py $< | $(CLANG_FORMAT) > $@


instrumented/%.x: instrumented/%.c
	gcc -o $@ $< -I ./examples

instrumented/%.d: examples/%.c src/instrument.py | instrumented
	LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py $<

view:
	${PYTHON} ./bin/pyclasvi.py -l $(LIBCLANG_PATH)

clean:
	rm -rf instrumented/*
