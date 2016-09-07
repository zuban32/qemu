/*
 * helper.c
 *
 *  Created on: Sep 7, 2016
 *      Author: zuban32
 */

#include "qemu/osdep.h"

#include "cpu.h"
#include "exec/exec-all.h"
#include "exec/cpu_ldst.h"
#include "qemu/host-utils.h"
#include "exec/helper-proto.h"

int arc4_cpu_handle_mmu_fault(CPUState *cs, vaddr address,
                               int rw, int mmu_idx)
{
	int r = 0;

	return r;
}


/* Try to fill the TLB and return an exception if error. If retaddr is
   NULL, it means that the function was called in C code (i.e. not
   from generated code or from helper.c) */
void tlb_fill(CPUState *cs, target_ulong addr, MMUAccessType access_type,
              int mmu_idx, uintptr_t retaddr)
{
    int ret;

    ret = arc4_cpu_handle_mmu_fault(cs, addr, access_type, mmu_idx);
    if (unlikely(ret)) {
        if (retaddr) {
            cpu_restore_state(cs, retaddr);
        }
    }
    cpu_loop_exit(cs);
}
