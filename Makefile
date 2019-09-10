PYTHON := 'python3'

EXAMPLE_C_SOURCE := 'examples/example3.c'

LIBCLANG_PATH := '/usr/lib/llvm-8/lib/libclang-8.0.0.so'

instrument:
	LIBCLANG_PATH=${LIBCLANG_PATH} ${PYTHON} ./src/instrument.py ${EXAMPLE_C_SOURCE} ${EXAMPLE_C_SOURCE}.instrumented.c


view:
	${PYTHON} ./bin/pyclasvi.py -l ${LIBCLANG_PATH}
