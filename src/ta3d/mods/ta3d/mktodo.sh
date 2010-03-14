#!/bin/sh

echo "files to create:" > TODO
for i in `cat files.txt | tr " " "²"`; do
	test -e "`echo $i | tr "²" " "`" || test -e "`echo $i | tr "²" " " | sed -e s/\.3do$/\.s3o/`" || test -e "`echo $i | tr "²" " " | sed -e s/\.gaf$//`" || test -e "`echo $i | tr "²" " " | sed -e s/\.cob$/\.lua/`" || echo $i >> TODO;
done
