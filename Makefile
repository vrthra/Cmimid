.SECONDARY: instrumented/calc_parse.c

PYTHON := python3

EXAMPLE_C_SOURCE := example3.c

LIBCLANG_PATH := /usr/local/Cellar/llvm/8.0.1/lib/libclang.dylib

instrument: | instrumented
	LIBCLANG_PATH=${LIBCLANG_PATH} $(PYTHON) ./src/instrument.py examples/$(EXAMPLE_C_SOURCE) | clang-format > instrumented/$(EXAMPLE_C_SOURCE)


instrumented: ; mkdir -p $@

instrumented/%.c: examples/%.c src/instrument.py | instrumented
	LIBCLANG_PATH=${LIBCLANG_PATH} $(PYTHON) ./src/instrument.py $< | clang-format > $@

instrumented/%.x: instrumented/%.c
	gcc -o $@ $< -I ./examples


view:
	${PYTHON} ./bin/pyclasvi.py -l ${LIBCLANG_PATH}

clean:
	rm -rf instrumented/*
