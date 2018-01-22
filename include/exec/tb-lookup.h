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

static inline bool check_pc(TranslationBlock *tb, target_ulong *pc)
{
    target_ulong new_pc = tb->pc;
    bool check_pc = new_pc == *pc;
//#ifdef ENABLE_BIG_TB
//    for(int i = 0; i < tb->cur_free_entry && !check_pc; i++) {
//        new_pc = tb->mid_entries[i];
//        check_pc |= (new_pc == *pc);
//    }
//#endif
    return check_pc;
}

/* Might cause an exception, so have a longjmp destination ready */
static inline TranslationBlock *
tb_lookup__cpu_state(CPUState *cpu, target_ulong *pc, target_ulong *cs_base,
                     uint32_t *flags, uint32_t cf_mask)
{
    CPUArchState *env = (CPUArchState *)cpu->env_ptr;
    TranslationBlock *tb;
    uint32_t hash;

    cpu_get_tb_cpu_state(env, pc, cs_base, flags);
    hash = tb_jmp_cache_hash_func(*pc);
    tb = atomic_rcu_read(&cpu->tb_jmp_cache[hash]);
    if (likely(tb &&
               tb->pc == *pc &&
//               check_pc(tb, pc) &&
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
