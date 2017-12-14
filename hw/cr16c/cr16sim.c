/*
 * QEMU/moxiesim emulation
 *
 * Emulates a very simple machine model similar to the one used by the
 * GDB moxie simulator.
 *
 * Copyright (c) 2008, 2009, 2010, 2013 Anthony Green
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "cpu.h"
#include "hw/sysbus.h"
#include "hw/hw.h"
#include "hw/i386/pc.h"
#include "hw/isa/isa.h"
#include "net/net.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/char/serial.h"
#include "exec/address-spaces.h"
#include "elf.h"

#define PHYS_MEM_BASE 0x80000000

typedef struct {
    uint64_t ram_size;
    const char *kernel_filename;
    const char *kernel_cmdline;
    const char *initrd_filename;
} LoaderParams;

static void load_kernel(CPUState *cpu, LoaderParams *loader_params)
{
    uint64_t entry, kernel_low, kernel_high;
    long kernel_size;
    long initrd_size;
    ram_addr_t initrd_offset;

    kernel_size = load_elf(loader_params->kernel_filename,  NULL, NULL,
                           &entry, &kernel_low, &kernel_high, 1, 177,
                           0, 0);

    if (kernel_size <= 0) {
        fprintf(stderr, "qemu: could not load kernel '%s'\n",
                loader_params->kernel_filename);
        exit(1);
    }

    /* load initrd */
    initrd_size = 0;
    initrd_offset = 0;
    if (loader_params->initrd_filename) {
        initrd_size = get_image_size(loader_params->initrd_filename);
        if (initrd_size > 0) {
            initrd_offset = (kernel_high + ~TARGET_PAGE_MASK)
              & TARGET_PAGE_MASK;
            if (initrd_offset + initrd_size > loader_params->ram_size) {
                fprintf(stderr,
                        "qemu: memory too small for initial ram disk '%s'\n",
                        loader_params->initrd_filename);
                exit(1);
            }
            initrd_size = load_image_targphys(loader_params->initrd_filename,
                                              initrd_offset,
                                              ram_size);
        }
        if (initrd_size == (target_ulong)-1) {
            fprintf(stderr, "qemu: could not load initial ram disk '%s'\n",
                    loader_params->initrd_filename);
            exit(1);
        }
    }
}

static void main_cpu_reset(void *opaque)
{
    CR16CCPU *cpu = opaque;

    cpu_reset(CPU(cpu));
}

static void cr16sim_init(MachineState *machine)
{
	fprintf(stderr, "sim init\n");
    CPUState *cpu = NULL;
    ram_addr_t ram_size = machine->ram_size;
    const char *cpu_model = machine->cpu_model;
    const char *kernel_filename = machine->kernel_filename;
    const char *kernel_cmdline = machine->kernel_cmdline;
    const char *initrd_filename = machine->initrd_filename;
    //CPUCR16CState *env;
    MemoryRegion *address_space_mem = get_system_memory();
    MemoryRegion *ram = g_new(MemoryRegion, 1);
    hwaddr ram_base = 0x0;
    LoaderParams loader_params;

    /* Init CPUs. */
    if (cpu_model == NULL) {
        cpu_model = "cr16c-cpu";
    }
	fprintf(stderr, "cpu init start\n");
    cpu = cpu_init(cpu_model);
	fprintf(stderr, "cpu init finish\n");
    if (!cpu) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }
    //env = &cpu->env;

    qemu_register_reset(main_cpu_reset, cpu);

    /* Allocate RAM. */
    memory_region_init_ram(ram, NULL, "cr16sim.ram", ram_size, &error_fatal);
    vmstate_register_ram_global(ram);
    memory_region_add_subregion(address_space_mem, ram_base, ram);

    if (kernel_filename) {
        loader_params.ram_size = ram_size;
        loader_params.kernel_filename = kernel_filename;
        loader_params.kernel_cmdline = kernel_cmdline;
        loader_params.initrd_filename = initrd_filename;
        load_kernel(cpu, &loader_params);
    }

    while(1);

    /* TODO: add at least some peripheral devices */
}

static void cr16sim_machine_init(MachineClass *mc)
{
    mc->desc = "CR16C simulator platform";
    mc->init = cr16sim_init;
    mc->is_default = 1;
}

DEFINE_MACHINE("cr16sim", cr16sim_machine_init)
