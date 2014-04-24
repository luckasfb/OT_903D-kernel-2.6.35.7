
    .text

    .global enable_arm11_perf_mon
enable_arm11_perf_mon:
    MRC p15, 0, r0, c15, c12, 0
    ORR r0, r0, #1
    MCR p15, 0, r0, c15, c12, 0
    BX lr

    .global disable_arm11_perf_mon
disable_arm11_perf_mon:
    MRC p15, 0, r0, c15, c12, 0
    BIC r0, r0, #1
    MCR p15, 0, r0, c15, c12, 0
    BX lr

    .global reset_arm11_perf_mon_cnt
reset_arm11_perf_mon_cnt:
    MRC p15, 0, r0, c15, c12, 0
    ORR r0, r0, #6
    MCR p15, 0, r0, c15, c12, 0
    BX lr

    .global get_arm11_perf_mon_cyc_cnt
get_arm11_perf_mon_cyc_cnt:
    MRC p15, 0, r0, c15, c12, 1 
    BX lr

    .global set_arm11_perf_mon_cyc_cnt
set_arm11_perf_mon_cyc_cnt:
    MCR p15, 0, r0, c15, c12, 1 
    BX lr

    .global get_arm11_perf_mon_cnt0
get_arm11_perf_mon_cnt0:
    MRC p15, 0, r0, c15, c12, 2 
    BX lr

    .global set_arm11_perf_mon_cnt0
set_arm11_perf_mon_cnt0:
    MCR p15, 0, r0, c15, c12, 2 
    BX lr

    .global get_arm11_perf_mon_cnt1
get_arm11_perf_mon_cnt1:
    MRC p15, 0, r0, c15, c12, 3 
    BX lr

    .global set_arm11_perf_mon_cnt1
set_arm11_perf_mon_cnt1:
    MCR p15, 0, r0, c15, c12, 3 
    BX lr

    .global set_arm11_perf_mon_cnt0_evt
set_arm11_perf_mon_cnt0_evt:
    MOV r2, #0xff
    MRC p15, 0, r1, c15, c12, 0
    BIC r1, r1, r2, lsl #20
    ORR r1, r1, r0, lsl #20
    MCR p15, 0, r1, c15, c12, 0
    BX lr

    .global set_arm11_perf_mon_cnt1_evt
set_arm11_perf_mon_cnt1_evt:
    MOV r2, #0xff
    MRC p15, 0, r1, c15, c12, 0
    BIC r1, r1, r2, lsl #12
    ORR r1, r1, r0, lsl #12
    MCR p15, 0, r1, c15, c12, 0
    BX lr

    .end

