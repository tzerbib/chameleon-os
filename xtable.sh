#!/usr/bin/env bash

ADDR=$(objdump -t kernel | grep 'xtable' | cut -d ' ' -f 1)
echo -e "#ifndef XTABLE_ADDR\n\n#define XTABLE_ADDR 0x$ADDR\n\n#endif" > xtable.h
