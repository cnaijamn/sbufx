#!/bin/csh

set diy = $HOME/.diy
set name = sbufx

mkdir -p ${diy}/{bin,include,lib,man/{man1,man2},libdata/pkgconfig}

cp src/lib/lib${name}.so{,.1} ${diy}/lib/
cp src/lib/${name}.h ${diy}/include/
cp src/lib/lib${name}.pc ${diy}/libdata/pkgconfig/

rehash
