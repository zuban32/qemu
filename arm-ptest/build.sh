#!/bin/bash
arm-linux-gnueabi-as -mfpu=neon -o test.elf test.S
arm-linux-gnueabi-ld -o test.o test.elf
# arm-linux-gnueabi-gcc -mfpu=neon testdbg.S -o testdbg.o
arm-linux-gnueabi-gcc -mfpu=neon testdbg.S -o testdbg.o