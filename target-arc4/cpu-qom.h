/*
 * cpu-qom.h
 *
 *  Created on: Sep 7, 2016
 *      Author: zuban32
 */

#ifndef TARGET_ARC4_CPU_QOM_H_
#define TARGET_ARC4_CPU_QOM_H_

#include "qom/cpu.h"

#define TYPE_ARC4_CPU "arc4-cpu"

#define ARC4_CPU_CLASS(klass) \
    OBJECT_CLASS_CHECK(ARC4CPUClass, (klass), TYPE_ARC4_CPU)
#define ARC4_CPU(obj) \
    OBJECT_CHECK(ARC4CPU, (obj), TYPE_ARC4_CPU)
#define ARC4_CPU_GET_CLASS(obj) \
    OBJECT_GET_CLASS(ARC4CPUClass, (obj), TYPE_ARC4_CPU)

/**
 * ARC4CPUClass:
 * @parent_realize: The parent class' realize handler.
 * @parent_reset: The parent class' reset handler.
 * @vr: Version Register value.
 *
 * An ARC4 CPU model.
 */
typedef struct ARC4CPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/

    DeviceRealize parent_realize;
    void (*parent_reset)(CPUState *cpu);

    uint32_t vr;
} ARC4CPUClass;

typedef struct ARC4CPU ARC4CPU;

#endif /* TARGET_ARC4_CPU_QOM_H_ */
