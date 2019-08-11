EXAMPLE_C_SOURCE := 'examples/example.c'

LIBCLANG_PATH := '/usr/lib/llvm-8/lib/libclang-8.0.0.so'

instrument:
	python3.7 ./src/instrument.py ${EXAMPLE_C_SOURCE} ${LIBCLANG_PATH}

view:
	python3.7 ./bin/pyclasvi.py -l ${LIBCLANG_PATH}
