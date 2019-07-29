#!/bin/bash

cd res
objcopy --input binary --output elf64-x86-64 --binary-architecture i386:x86-64 icon.png icon.o
