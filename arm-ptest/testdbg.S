.text
.global main
.extern printf
.extern exit
main:
    mov         r7, #len
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
    vmul.i32	q3, q3, q1
    vadd.i32	q3, q3,	q1

    @push {r7, ip}
    @ldr r0, =string0
    @mov r1, r7
    @bl printf
    @pop {r7, ip}

    @cmp r7, #len
    @bne loop_end

    @push {r7, ip}
    @ldr r0, =string1
    @vmov.i32 r1, d6[0]
    @bl printf
    @pop {r7, ip}
loop_end:
    subs		r7, r7, #1
    bne         loop

    mov r0, #0
    bl exit


.data
len = 0xf000000
string0: .asciz "Counter = %x\n"
string1: .asciz "Result = %u\n"
    