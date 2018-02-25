/*
 * Copyright (C) 2017, Emilio G. Cota <cota@braap.org>
 *
 * License: GNU GPL, version 2 or later.
 *   See the COPYING file in the top-level directory.
 */
#ifndef EXEC_TB_LOOKUP_H
#define EXEC_TB_LOOKUP_H

#include "qemu/osdep.h"

#ifdef NEED_CPU_H
#include "cpu.h"
#else
#include "exec/poison.h"
#endif

#include "exec/exec-all.h"
#include "exec/tb-hash.h"

/* Might cause an exception, so have a longjmp destination ready */
static inline TranslationBlock *
tb_lookup__cpu_state(CPUState *cpu, target_ulong *pc, target_ulong *cs_base,
                     uint32_t *flags, uint32_t cf_mask)
{
    CPUArchState *env = (CPUArchState *)cpu->env_ptr;
    TranslationBlock *tb;
    uint32_t hash;
//    fprintf(stderr, "tb_lookup_state in1: pc = %lx\n", *pc);

    cpu_get_tb_cpu_state(env, pc, cs_base, flags);
//    fprintf(stderr, "tb_lookup_state in2: pc = %lx\n", *pc);
    hash = tb_jmp_cache_hash_func(*pc);
    tb = atomic_rcu_read(&cpu->tb_jmp_cache[hash]);
    if (likely(tb &&
               tb->pc == *pc &&
               tb->cs_base == *cs_base &&
               tb->flags == *flags &&
               tb->trace_vcpu_dstate == *cpu->trace_dstate &&
               (tb_cflags(tb) & (CF_HASH_MASK | CF_INVALID)) == cf_mask)) {
        return tb;
    }
    tb = tb_htable_lookup(cpu, *pc, *cs_base, *flags, cf_mask);
    if (tb == NULL) {
        return NULL;
    }
    atomic_set(&cpu->tb_jmp_cache[hash], tb);
    return tb;
}

#endif /* EXEC_TB_LOOKUP_H */
