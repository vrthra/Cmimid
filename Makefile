.SECONDARY:

PYTHON=python3
pfuzzer=../checksum-repair

EXAMPLE_C_SOURCE=example3.c

LIBCLANG_PATH=/usr/local/Cellar/llvm/8.0.1/lib/libclang.dylib
LIBCLANG_PATH=/usr/lib/llvm-8/lib/libclang.so

CLANG_FORMAT=/usr/local/Cellar/llvm/8.0.1/bin/clang-format
CLANG_FORMAT=/usr/lib/llvm-8/bin/clang-format

instrument: | build
	LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py examples/$(EXAMPLE_C_SOURCE) | $(CLANG_FORMAT) > build/$(EXAMPLE_C_SOURCE)


build: ; mkdir -p $@

build/%.c: examples/%.c src/instrument.py | build
	LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py $< | $(CLANG_FORMAT) > $@


build/%.x: build/%.c
	gcc -g -o $@ $< -I ./examples

build/%.d: examples/%.c src/instrument.py | build
	LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py $<

build/%.input: examples/%.input build/%.x
	cat $< > $@

build/%.json: build/%.input
	rm -rf $(pfuzzer)/build
	cp examples/calc_parse.h build
	cp -r build $(pfuzzer)/build
	cd $(pfuzzer) && $(MAKE) build/$*.taint
	cp $(pfuzzer)/build/pygmalion.json build/$*.json_
	mv build/$*.json_ build/$*.json

build/%.events: build/%.json
	$(PYTHON) ./src/events.py build/$*.json build/$*.input > $@_
	mv $@_ $@


build/%.grammar: build/%.events
	$(PYTHON) ./src/grammar-miner.py $<


view:
	${PYTHON} ./bin/pyclasvi.py -l $(LIBCLANG_PATH)

clean:
	rm -rf build/*
	cd $(pfuzzer) && $(MAKE) clean
