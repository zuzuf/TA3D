#!/bin/sh

echo "files to create:" > TODO
for i in `cat files.txt | tr " " "²"`; do test -e "`echo $i | tr "²" " "`" || echo $i >> TODO; done  
