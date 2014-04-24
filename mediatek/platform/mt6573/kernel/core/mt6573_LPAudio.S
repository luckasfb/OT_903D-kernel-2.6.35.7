
#include <linux/autoconf.h>
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <mach/hardware.h>

//#define MT6573_SLP_ITCM_PA 0xF0400000
//#define MT6573_EMI_GENA_PA 0xF0020078
//#define MT6573_EMI_CONN_PA 0xF0020068
//#define MT6573_SLP_REGS_PA 0xF0035200
//#define MT6573_PMU_REGS_PA 0xF0061300

#define MT6573_SYSRAM_BASE 0xF4000000

//#define I_BIT      0x80
//#define F_BIT      0x40
//#define C9_ENABLE_BIT 0x00000001

	.text
ENTRY(MT6573_EMI_switch)  
		stmdb   r13!,{r4-r8,r14}

//	pITCM_Base = MT6573_SLP_ITCM_PA;
		ldr     r4,=MT6573_SYSRAM_BASE

//	pEMI_CONN =  (MT6573_EMI_CONN_PA;
		mov     r5,#0

//	pSLP_REGS =  (MT6573_SLP_REGS_PA;
		mov     r6,#0

//	pPMU_REGS =  (MT6573_PMU_REGS_PA;
		mov     r7,#0
//	pEMI_GENA =  (MT6573_EMI_GENA_PA;
		mov     r8,#0

//	copy suspend code to non_EMI region
		mov     r1,r4
		ldr     r0,=switch_start
		mov     r2, r1
		add     r2, r2, #0x300
		b      copy
Done:		
//		mov     pc, r4
		nop

copy:  
// loop whilst r1 < r2
        CMP     r1, r2          
        LDRLO   r3, [r0], #4
        STRLO   r3, [r1], #4
        BLO     copy
//		b       Done   


switch_start:
//	*(pSLP_REGS+0x18) |= 0x2
//		ldr		r3,[r6,#0x18]
//		orr     r3,r3,#0x02
//		str		r3,[r6,#0x18]
//	save PMU_CON4 to r0
//		ldr		r0,[r7,#0x10]
//  PMU_CON4 = 0x7380
//		bic		r3, r0, #0xF000
//		orr		r3, r3, #0x7000
//		str		r3, [r7,#0x10]
//	save PMU_CON5 to r1
//		ldr		r1,[r7,#0x14]
//  PMU_CON5 = 0xD4F7
//		bic		r3, r1, #0xF000
//		orr		r3, r3, #0xD000
//		orr		r3, r3, #0x00F0
//		str		r3, [r7,#0x14]
//	save PMU_CON7 to r2
//		ldr		r2, [r7,#0x1C]
//  PMU_CON7 = 0x781F
//		orr		r3, r2, #0x7000
//		orr		r3, r3, #0x000F
//		str		r3, [r7,#0x1C]
//  pmu switches to internal ring osc
//	(VCORE1)PMU_CON3, *(pPMU_REGS+0x0C) &= ~0x1000
//		ldr		r3,[r7,#0x0C]
//		bic     r3,r3,#0x1000
//		str		r3,[r7,#0x0C]
//	(VCORE2)PMU_CON9, *(pPMU_REGS+0x24) &= ~0x1000
//		ldr		r3,[r7,#0x24]
//		bic     r3,r3,#0x1000
//		str		r3,[r7,#0x24]
//	(VM)PMU_CONF, *(pPMU_REGS+0x3C) &= ~0x1000
//		ldr		r3,[r7,#0x3C]
//		bic     r3,r3,#0x1000
//		str		r3,[r7,#0x3C]
//	*pEMI_CONN |= 0x20;				// set DRAM into self refresh mode
//		ldr     r3,[r5]
//		orr     r3,r3,#0x20
//		str     r3,[r5]		

// Todo: fine tune delay at least 20T
		mov		r3,#0xFF
switch_delay:
		sub		r3,r3,#0x1
		cmp     r3,#0x0
		bgt     switch_delay
//DRV_WriteReg32(0x80020078,0x80000702); 
//	*pEMI_GENA &= ~0x1;				// Bank0~1 switch, 0x00000000 NOR, 0x10000000 DDR
//		ldr     r3,[r8]
//		bic     r3,r3,#0x1
//		str     r3,[r8]		    
//arm sleep mode
//		mov		r3, #0
//		mcr		p15, 0, r3, c7, c0, 4
//DRV_WriteReg32(0x80020078,0x80000703); 
//	*pEMI_GENA |= 0x1;				// Bank0~1 switch, 0x00000000 NOR, 0x10000000 DDR
//		ldr     r3,[r8]
//		orr     r3,r3,#0x1
//		str     r3,[r8]		    		
//	*pEMI_CONN &= ~0x20;			//disable self refresh mode 
//		ldr     r3,[r5]
//		bic     r3,r3,#0x20
//		str     r3,[r5]
// Todo: fine tune delay
//		mov		r3,#0xFF
//resume_delay:
//		sub		r3,r3,#0x1
//		cmp     r3,#0x0
//		bgt     resume_delay
// pmu switches to digital 26M clock
//	(VCORE1)PMU_CON3, *(pPMU_REGS+0x0C) |= 0x1000
//		ldr		r3,[r7,#0x0C]
//		orr     r3,r3,#0x1000
//		str		r3,[r7,#0x0C]
//	(VCORE2)PMU_CON9, *(pPMU_REGS+0x24) |= 0x1000
//		ldr		r3,[r7,#0x24]
//		orr     r3,r3,#0x1000
//		str		r3,[r7,#0x24]
//	(VM)PMU_CONF, *(pPMU_REGS+0x3C) |= 0x1000
//		ldr		r3,[r7,#0x3C]
//		orr     r3,r3,#0x1000
//		str		r3,[r7,#0x3C]
//	restore PMU_CON4 from r0
//		str		r0,[r7,#0x10]
//	restore PMU_CON5 from r1
//		str		r1,[r7,#0x14]
//	restore PMU_CON7 from r2
//		str		r2,[r7,#0x1C]
suspend_end:
		ldmia   r13!,{r4-r8,r14}
		bx      r14


        




