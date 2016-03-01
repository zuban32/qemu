    mov         r7, #4@0xf000000
    vmov.i32    d0, #0x01
    vmov.i32    d1, #0x00
    vmov.i32	d2, #0x01
    vmov.i32	d3, #0x00
    vmov.i32	d4, #0x02
    vmov.i32	d5, #0x00
loop:
    vadd.i32	q0, q0, q1
    vadd.i32    q3, q3, q0
    vmul.i32	q3, q3, q2
    vmul.i32    q3, q3, q1
    vadd.i32	q3, q3,	q0

    @cmp r7, #1
    @bne loop_end
loop_end:
    subs		r7, r7, #1
    bne         loop
    # pop {pc}
    