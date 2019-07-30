#!/bin/bash

objcopy -I binary -O elf64-x86-64 -B i386:x86-64 icon_16.png gen/icon_16.o
objcopy -I binary -O elf64-x86-64 -B i386:x86-64 icon_32.png gen/icon_32.o
objcopy -I binary -O elf64-x86-64 -B i386:x86-64 icon_64.png gen/icon_64.o
objcopy -I binary -O elf64-x86-64 -B i386:x86-64 icon_128.png gen/icon_128.o
objcopy -I binary -O elf64-x86-64 -B i386:x86-64 icon_256.png gen/icon_256.o
