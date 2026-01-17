#!/bin/sh

# moves the asterisk of pointer variables to the type name
# cause this is clearly the superior way

for file in src/*.c
do
    sed -i 's/ \*\([^ /]\)/* \1/g' $file
done

for file in include/*.h
do
    sed -i 's/ \*\([^ /]\)/* \1/g' $file
done