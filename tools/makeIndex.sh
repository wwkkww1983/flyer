#!/bin/sh

echo "find"
find . -name "*.h" -o -name "*.c" -o -name "*.cc" > cscope.files

echo "cscope"
cscope -bkq -i cscope.files

echo "ctags"
ctags -R

echo "done"
