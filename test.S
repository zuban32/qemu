    mov         r0, #0xb0000000
    vmov.i32	d2, #0x01
    vmov.i32	d3, #0x00
    vmov.i32	d4, #0x02
    vmov.i32	d5, #0x00
loop:
    vadd.i32	q0, q0, q1
    vmul.i32	q0, q2, q1
    vadd.i32	q0, q0,	q1
    subs		r0, r0, #1
    bne         loop
    