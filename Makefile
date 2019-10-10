.SECONDARY:

PYTHON=python3
pfuzzer=../checksum-repair
CC=clang-8

CFLAGS=-xc++ -std=c++14
CFLAGS=-I/usr/lib/llvm-8/lib/clang/8.0.0/include

EXAMPLE_C_SOURCE=example3.c

LIBCLANG_PATH=/usr/local/Cellar/llvm/8.0.1/lib/libclang.dylib
LIBCLANG_PATH=/usr/lib/llvm-8/lib/libclang.so

CLANG_FORMAT=/usr/local/Cellar/llvm/8.0.1/bin/clang-format
CLANG_FORMAT=/usr/lib/llvm-8/bin/clang-format

instrument: | build
	CFLAGS=$(CFLAGS) LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py examples/$(EXAMPLE_C_SOURCE) | $(CLANG_FORMAT) > build/$(EXAMPLE_C_SOURCE)

build/%.out: examples/%.c
	$(CC) $(CFLAGS) -g -o $@ $< -I ./examples

build/json.out: examples/json.c | build
	$(CC) $(CFLAGS) -g -o $@ $^ -I ./examples

build: ; mkdir -p $@

build/%.c: examples/%.c build/%.out src/instrument.py | build
	CFLAGS=$(CFLAGS) LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py $< > $@_.tmp
	cat $@_.tmp | $(CLANG_FORMAT) > $@_
	mv $@_ $@

build/%.x: build/%.c
	$(CC) $(CFLAGS) -g -o $@ $^ -I ./examples

build/%.d: examples/%.c src/instrument.py | build
	CFLAGS=$(CFLAGS) LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py $<

build/%.input: examples/%.input
	cat $< > $@

build/%.json.done: build/%.x
	mkdir -p $(pfuzzer)/build/
	rm -rf $(pfuzzer)/build/*
	cp examples/*.h build
	cp -r build/* $(pfuzzer)/build
	mkdir -p build/$*
	for i in examples/$*.input.*; \
	do\
	  echo $$i; \
	  cp $$i $(pfuzzer)/build/$*.input; \
	  (cd $(pfuzzer) && $(MAKE) build/$*.taint;) ; \
		cp $(pfuzzer)/build/pygmalion.json build/$*/$$(basename $$i).json; \
		cp $$i build/$*/ ; \
	done

build/%.events: build/%.json.done
	$(PYTHON) ./src/events.py build/$* > $@_
	mv $@_ $@


build/%.grammar: build/%.events
	$(PYTHON) ./src/grammar-miner.py $<
	cp build/g.json $@


view:
	CFLAGS=$(CFLAGS) ${PYTHON} ./bin/pyclasvi.py -l $(LIBCLANG_PATH)

clean:
	rm -rf build/*
	cd $(pfuzzer) && $(MAKE) clean

dump:
	clang -Xclang -ast-dump -fsyntax-only $(src) -I examples

build/%.fuzz: build/%.grammar build/%.out
	$(PYTHON) ./src/fuzz.py $^
