PYTHON := python3

EXAMPLE_C_SOURCE := example3.c

LIBCLANG_PATH := /usr/lib/llvm-8/lib/libclang-8.0.0.so

instrument: | instrumented
	LIBCLANG_PATH=${LIBCLANG_PATH} ${PYTHON} ./src/instrument.py examples/$(EXAMPLE_C_SOURCE) | clang-format-8 > instrumented/$(EXAMPLE_C_SOURCE)


instrumented: ; mkdir -p $@

view:
	${PYTHON} ./bin/pyclasvi.py -l ${LIBCLANG_PATH}
