.SECONDARY:

PYTHON=python3
pfuzzer=../checksum-repair

EXAMPLE_C_SOURCE=example3.c

LIBCLANG_PATH=/usr/local/Cellar/llvm/8.0.1/lib/libclang.dylib
LIBCLANG_PATH=/usr/lib/llvm-8/lib/libclang.so

CLANG_FORMAT=/usr/local/Cellar/llvm/8.0.1/bin/clang-format
CLANG_FORMAT=/usr/lib/llvm-8/bin/clang-format

instrument: | instrumented
	LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py examples/$(EXAMPLE_C_SOURCE) | $(CLANG_FORMAT) > instrumented/$(EXAMPLE_C_SOURCE)


instrumented: ; mkdir -p $@

instrumented/%.c: examples/%.c src/instrument.py | instrumented
	LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py $< | $(CLANG_FORMAT) > $@


instrumented/%.x: instrumented/%.c
	gcc -g -o $@ $< -I ./examples

instrumented/%.d: examples/%.c src/instrument.py | instrumented
	LIBCLANG_PATH=$(LIBCLANG_PATH) $(PYTHON) ./src/instrument.py $<

#instrumented/calc_parse.input: instrumented/calc_parse.x
#	cat examples/calc_parse.i > $@


instrumented/urlparse.input: instrumented/urlparse.x
	echo 'http://www.google.com:80/q?search=me+you&test=last#fragment' > $@


instrumented/%.input: examples/%.i instrumented/%.x
	cat $< > $@


instrumented/%.json: instrumented/%.input
	rm -rf $(pfuzzer)/instrumented
	cp examples/calc_parse.h instrumented
	cp -r instrumented $(pfuzzer)/instrumented
	cd $(pfuzzer) && $(MAKE) instrumented/$*.taint
	cp $(pfuzzer)/instrumented/pygmalion.json instrumented/$*.json_
	mv instrumented/$*.json_ instrumented/$*.json

instrumented/%.events: instrumented/%.json
	python3 ./src/events.py instrumented/$*.json instrumented/$*.input > $@_
	mv $@_ $@


view:
	${PYTHON} ./bin/pyclasvi.py -l $(LIBCLANG_PATH)

clean:
	rm -rf instrumented/*
