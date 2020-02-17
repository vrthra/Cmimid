#!/usr/bin/env bash

filename=$PWD/$1
prog=$2
cp $filename ../checksum-repair/build/my.input
( cd ../checksum-repair/;
cat build/my.input | $prog.instrumented
gzip -c output > build/output.gz
./install/bin/trace-taint -me build/metadata -po build/pygmalion.json -t build/output.gz
) 2>build/err 1>build/out

mv ../checksum-repair/build/pygmalion.json $filename.json
python src/events.py $filename.json > $filename.trace 2>/dev/null
PARSE=1 python src/treeminer.py $filename.trace 2>/dev/null
