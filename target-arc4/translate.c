#include "qemu/osdep.h"
#include "cpu.h"
#include "disas/disas.h"
#include "exec/exec-all.h"
#include "tcg-op.h"
#include "exec/helper-proto.h"
//#include "mmu.h"
#include "exec/cpu_ldst.h"

#include "exec/helper-gen.h"

#include "trace-tcg.h"
#include "exec/log.h"

void gen_intermediate_code(CPUARC4State *env, struct TranslationBlock *tb)
{

}

void restore_state_to_opc(CPUARC4State *env, TranslationBlock *tb,
                          target_ulong *data)
{
    env->pc = data[0];
}
