/*
 * cpu.h
 *
 *  Created on: Sep 7, 2016
 *      Author: zuban32
 */

#ifndef ARC4_CPU_H
#define ARC4_CPU_H

#include "qemu-common.h"
#include "cpu-qom.h"
#include "qemu/osdep.h"

#define TARGET_LONG_BITS 32
#define TARGET_PAGE_BITS 8

#define CPUArchState struct CPUARC4State

#define TARGET_PHYS_ADDR_SPACE_BITS 16
#define TARGET_VIRT_ADDR_SPACE_BITS 16
#define NB_MMU_MODES 1

#include "exec/cpu-defs.h"
#include "exec/cpu-all.h"

typedef struct CPUARC4State {

    uint32_t flags;               /* general execution flags */
    uint32_t pc;                  /* program counter */

    CPU_COMMON

} CPUARC4State;

/**
 * ARC4CPU:
 * @env: #CPUARC4State
 *
 * An ARC4 CPU.
 */
struct ARC4CPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUARC4State env;
};

static inline ARC4CPU *arc4_env_get_cpu(CPUARC4State *env)
{
    return container_of(env, ARC4CPU, env);
}

#define ENV_GET_CPU(e) CPU(arc4_env_get_cpu(e))

static inline int cpu_mmu_index(CPUARC4State *env, bool ifetch)
{
    return 0;
}

int arc4_cpu_handle_mmu_fault(CPUState *cpu, vaddr address, int rw,
                              int mmu_idx);

static inline void cpu_get_tb_cpu_state(CPUARC4State *env, target_ulong *pc,
                                        target_ulong *cs_base, uint32_t *flags)
{
	/* INSERT YOUR CODE HERE */
	*pc = 0;
	*cs_base = 0;
	*flags = 0;
}

#endif /* TARGET_ARC4_CPU_H_ */
