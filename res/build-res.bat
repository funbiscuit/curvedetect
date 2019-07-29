#!/bin/bash

windres win-res.rc -O coff gen/win-res.res
objcopy -I binary -O pe-x86-64 -B i386:x86-64 icon.png gen/icon.o
