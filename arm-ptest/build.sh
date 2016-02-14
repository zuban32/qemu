#!/bin/bash
# arm-linux-gnueabi-as -mfpu=neon -o test.elf test.S
# arm-linux-gnueabi-ld -lc -o test.o test.elf
arm-linux-gnueabi-gcc -mfpu=neon test.S -o test.o