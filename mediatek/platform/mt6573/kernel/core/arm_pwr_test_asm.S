
    .text

    .global apmcu_disable_irq 
apmcu_disable_irq:
   MRS     r0, CPSR
   ORR     r0, r0, #0x80
   MSR     CPSR_c, r0
   BX      lr 


    .global apmcu_enable_irq 
apmcu_enable_irq:
   MRS     r0, CPSR
   AND     r0, r0, #0x7F
   MSR     CPSR_c, r0
   BX      lr 


    .global apmcu_clean_dcache
apmcu_clean_dcache:
   MOV     r0, #0
   MCR     p15, 0, r0, c7, c10, 0
   BX      lr 


    .global apmcu_clean_invalidate_dcache
apmcu_clean_invalidate_dcache:
   MOV     r0, #0
   MCR     p15, 0, r0, c7, c14, 0
   BX      lr 


    .global apmcu_invalidate_tlb
apmcu_invalidate_tlb:
   MCR     p15, 0, r0, c8, c5, 0
   MCR     p15, 0, r0, c8, c6, 0
   MCR     p15, 0, r0, c8, c7, 0
   BX      lr 


    .global apmcu_dsb
apmcu_dsb:
   MCR     p15, 0, r0, c7, c10, 4
   BX      lr 



    .global apmcu_invalidate_icache
apmcu_invalidate_icache:
   MOV     r0, #0
   MCR     p15, 0, r0, c7, c5, 0
   BX      lr 


    .global apmcu_disable_prefetch_halt
apmcu_disable_prefetch_halt:
   MRC     p15, 0, r0, c1, c0, 1
   ORR     r0, r0, #0x10000000
   MCR     p15, 0, r0, c1, c0, 1
   BX      lr 


    .global apmcu_enable_prefetch_halt
apmcu_enable_prefetch_halt:
   MRC     p15, 0, r0, c1, c0, 1
   BIC     r0, r0, #0x1c
   MCR     p15, 0, r0, c1, c0, 1
   BX      lr 


    .global apmcu_enable_branch_prediction
apmcu_enable_branch_prediction:
   MRC     p15, 0, r0, c1, c0, 0
   ORR     r0, r0, #0x800
   MCR     p15, 0, r0, c1, c0, 0
   BX      lr 


    .global apmcu_disable_branch_prediction
apmcu_disable_branch_prediction:
   MRC     p15, 0, r0, c1, c0, 0
   BIC     r0, r0, #0x800
   MCR     p15, 0, r0, c1, c0, 0
   BX      lr 


#    .global apmcu_tlb_lockdown
#apmcu_tlb_lockdown:
#   MCR     p15, 0, r0, c8, c5, 0
#   MCR     p15, 0, r0, c8, c6, 0
#   MCR     p15, 0, r0, c8, c7, 0
#   MOV     r0, #0xFFFF
#   MCR     p15, 0, r0, c10, c0, 0
#   BX      lr 

    .global load_dword_test
load_dword_test:
   MOV     r0, #0x50000000
   ADD     r0, r0, #0x100000
   ADD     r0, r0, #0x2000
   LDRD    r2, [r0]
   BX      lr



    .global vfp_setup
vfp_setup:
                STMDB   sp!, {r3-r4}
                MOV     r3, #0x40000000
                FMXR    FPEXC, r3
                FMXR    FPSID, r3
                MOV     r3, #0
                FMXR    FPSCR, r3
                FMRX    r3, FPSCR
                BIC     r3, r3, #0x00c00000
                BIC     r3, r3, #0x01000000
                MOV     r4, #3
                MOV     r4, r4, LSL #16
                BIC     r3, r3, #0x00070000
                ORR     r3, r3, r4
                BIC     r3, r3, #0x00300000
                FMXR    FPSCR, r3
                VLDM    r1, {d4-d15}
                VLDM    r1, {d0-d3}
                FLDD    d1, [r0]
                FMXR    FPSID, r3

                LDMIA   sp!, {r3-r4}
                BX lr


    .global power_test2
power_test2:
                STMDB   sp!, {r3-r12}
                LDMIA   r0,{r4-r12}
		ADDS    r0, r0, #0

loop_2:
		VSTM   r1, {d0-d8}
                FMACD   d12, d4, d8
		SMLAL   r4, r5, r6, r7
                ADCS    r3, r6, r7, ASR #0x18
                STMIA   r1,{r6-r12}
		SMLAL   r4, r5, r7, r8
                ADCS    r3, r3, r8, ASR #0x18
		VSTM   r1, {d0-d8}
		SMLAL   r4, r5, r8, r9
                ADCS    r3, r3, r9, ASR #0x10
                FMACD   d12, d4, d8
                STMIA   r1,{r6-r12}
		SMLAL   r4, r5, r9, r10
                ADCS    r3, r3, r10, ASR #0x10
		VSTM   r1, {d0-d8}
		SMLAL   r4, r5, r10, r11
                ADCS    r3, r3, r11, ASR #0x14
                FMACD   d12, d4, d8
                STMIA   r1,{r6-r12}
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, ASR #0x14
		SMLAL   r4, r5, r11, r12
		VSTM   r1, {d0-d8}
                ADCS    r3, r3, r12, ASR #0x4
                FMACD   d12, d4, d8
                STMIA   r1,{r6-r12}
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, ASR #0x4
		SMLAL   r4, r5, r11, r12
		VSTM   r1, {d0-d8}
                ADCS    r3, r3, r12, ASR #0x14
                FMACD   d12, d4, d8
                VLDM    r1, {d0-d8}
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, ASR #0x4
		FADDD   d0, d0, d1
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, LSL #0x4
		FADDD   d0, d0, d2
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, ASR #0x4
		FADDD   d0, d0, d3
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, LSR #0x4
		FADDD   d0, d0, d4
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, ASR #0x4
		FADDD   d0, d0, d5
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, LSL #0x4
		FADDD   d0, d0, d6
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, ASR #0x4
		FADDD   d0, d0, d7
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, LSL #0x4
		FADDD   d0, d0, d8
		SMLAL   r4, r5, r11, r12
                ADCS    r4, r4, r3, ASR #0x4
                ADCS    r5, r5, r4, ASR #0x4
		FMRRD   r3, r4, d0
                ADCS    r4, r3, r4
                FSQRTD  d9, d9
		LDR     r0, =0xF9018000
		MOV     r3, lr
		BLX     r0
		MOV     lr, r3
		LDR     r3, =0x03118484
                SUBS    r2, r2, #1
                MCR     p15, 0, r0, c8, c5, 0
                BNE     loop_2

                #MCR     p15, 0, r0, c8, c6, 0
                #MCR     p15, 0, r0, c8, c7, 0
                VLDM    r1, {d0-d8}
		ADCS    r4, r4, r5
		FADDD   d0, d0, d1
		ADCS    r0, r3, r4
		FADDD   d0, d0, d2
		FMRRD   r1, r2, d10
	        ADCS    r0, r0, r1
		FADDD   d0, d0, d3
	        ADCS    r0, r0, r2
		FADDD   d0, d0, d4
		FMRRD   r1, r2, d12
	        ADCS    r0, r0, r1
		FADDD   d0, d0, d5
	        ADCS    r0, r0, r2
		FADDD   d0, d0, d6
		FMRRD   r1, r2, d9
	        ADCS    r0, r0, r1
		FADDD   d0, d0, d7
	        ADCS    r0, r0, r2
		FADDD   d0, d0, d8
		FMRRD   r2, r3, d0
	        ADCS    r0, r0, r2

                LDMIA   sp!, {r3-r12}
                BX lr


    .global power_test3
power_test3:
                STMDB   sp!, {r3-r12}
                LDMIA   r0,{r4-r12}
		ADDS    r0, r0, #0

loop_3:
		VSTM   r1, {d0-d8}
                FMACD   d12, d4, d8
		SMLAL   r4, r5, r6, r7
                ADCS    r3, r6, r7, ASR #0x18
                STMIA   r1,{r6-r12}
		SMLAL   r4, r5, r7, r8
                ADCS    r3, r3, r8, ASR #0x18
		VSTM   r1, {d0-d8}
		SMLAL   r4, r5, r8, r9
                ADCS    r3, r3, r9, ASR #0x10
                FMACD   d12, d4, d8
                STMIA   r1,{r6-r12}
		SMLAL   r4, r5, r9, r10
                ADCS    r3, r3, r10, ASR #0x10
		VSTM   r1, {d0-d8}
		SMLAL   r4, r5, r10, r11
                ADCS    r3, r3, r11, ASR #0x14
                FMACD   d12, d4, d8
                STMIA   r1,{r6-r12}
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, ASR #0x14
		SMLAL   r4, r5, r11, r12
		VSTM   r1, {d0-d8}
                ADCS    r3, r3, r12, ASR #0x4
                FMACD   d12, d4, d8
                STMIA   r1,{r6-r12}
		SMLAL   r4, r5, r11, r12
                ADCS    r3, r3, r12, ASR #0x4
		SMLAL   r4, r5, r11, r12
		VSTM   r1, {d0-d8}
                ADCS    r3, r3, r12, ASR #0x14
                FMACD   d12, d4, d8
                VLDM   r1, {d0-d8}
                B     loop_3

                LDMIA   sp!, {r3-r12}
                BX lr


    .end

