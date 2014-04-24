#ifndef __MT6573_EMI_MPU__
#define __MT6573_EMI_MPU__

/* EMI Memory Protect Unit */
#define EMI_MPUA                 (EMI_BASE+0x0160)
#define EMI_MPUB                 (EMI_BASE+0x0168)
#define EMI_MPUC                 (EMI_BASE+0x0170)
#define EMI_MPUD                 (EMI_BASE+0x0178)
#define EMI_MPUE                 (EMI_BASE+0x0180)
#define EMI_MPUF	                (EMI_BASE+0x0188)
#define EMI_MPUG	                (EMI_BASE+0x0190)
#define EMI_MPUH	                (EMI_BASE+0x0198)
  
#define EMI_MPUI	           (EMI_BASE+0x01A0)
#define EMI_MPUJ	           (EMI_BASE+0x01A8)
#define EMI_MPUK            (EMI_BASE+0x01B0)
#define EMI_MPUL            (EMI_BASE+0x01B8)
#define EMI_MPUM            (EMI_BASE+0x01C0)
#define EMI_MPUN            (EMI_BASE+0x01C8)
#define EMI_MPUO            (EMI_BASE+0x01D0)
#define EMI_MPUP            (EMI_BASE+0x01D8)
#define EMI_MPUQ            (EMI_BASE+0x01E0)
#define EMI_MPUR            (EMI_BASE+0x01E8)
#define EMI_MPUS            (EMI_BASE+0x01F0)
#define EMI_MPUT            (EMI_BASE+0x01F8)

#define DEVAPC0_D0_VIO_MASK     (DEVICE_APC0_BASE+0x0020)
#define DEVAPC1_DXS_VIO_MASK   (DEVICE_APC1_BASE+0x0080)

#define DEVAPC0_D0_VIO_STA   (DEVICE_APC0_BASE+0x0030)
#define DEVAPC1_DXS_VIO_STA (DEVICE_APC1_BASE+0x0084)

/* Define access permission */
#define NO_PRETECTION     0
#define SEC_RW                    1
#define SEC_RW_NSEC_R    2
#define SEC_RW_NSEC_W   3
#define SEC_R_NSEC_R       4
#define FORBIDDEN              5

/*Domain2, Domain1, Domain0*/
#define SET_ACCESS_PERMISSON(d2, d1, d0)    ((d2 << 6) | (d1 << 3) | (d0))

#define EN_MPU_STR "ON"
#define DIS_MPU_STR "OFF"

#define MD_BIN    0
#define AP_CCCI_MPU_REGION    1
#define CCCI_MPU_REGION           2
#define CCCI_AP_MPU_REGION    3
#define DSP_BIN    4



extern int emi_mpu_set_region_protection(unsigned int start_addr, unsigned int end_addr, int region, unsigned int access_permission);
extern void start_mm_mau_protect(unsigned int mau_entry_1_start, unsigned int mau_entry_1_end, 
                                                            unsigned int mau_entry_2_start, unsigned int mau_entry_2_end);


#endif
