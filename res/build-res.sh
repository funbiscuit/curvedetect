#!/bin/bash

objcopy -I binary -O elf64-x86-64 -B i386:x86-64 icon.png gen/icon.o
