instrument:
	python ./src/instrument.py examples/example.c


view:
	python ./bin/pyclasvi.py -l '/usr/lib/llvm-8/lib/libclang-8.0.0.so'
