#!/usr/bin/env bash

filename=$PWD/$1
prog=$2
cp $filename ../checksum-repair/build/my.input
( cd ../checksum-repair/;
cat build/my.input | build/$prog.c.instrumented
gzip -c output > build/output.gz
./install/bin/trace-taint -me build/metadata -po build/pygmalion.json -t build/output.gz
)

cp ../checksum-repair/build/pygmalion.json $filename.json
python src/events.py $filename.json
