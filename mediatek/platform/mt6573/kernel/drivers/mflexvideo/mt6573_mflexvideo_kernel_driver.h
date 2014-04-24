
#ifndef __MT6573_MFLEXVIDEO_H__
#define __MT6573_MFLEXVIDEO_H__

#define MFV_IOC_MAGIC    'M'

//below is control message
#define MFV_TEST_CMD               _IO(MFV_IOC_MAGIC,  0x00)
#define MFV_INIT_CMD               _IO(MFV_IOC_MAGIC, 0x01)
#define MFV_DEINIT_CMD             _IO(MFV_IOC_MAGIC, 0x02)
#define MFV_SET_CMD_CMD            _IOW(MFV_IOC_MAGIC, 0x03, P_MFV_DRV_CMD_QUEUE_T)
#define MFV_SET_PWR_CMD            _IOW(MFV_IOC_MAGIC, 0x04, HAL_POWER_T *)
#define MFV_SET_ISR_CMD            _IOW(MFV_IOC_MAGIC, 0x05, HAL_ISR_T *)
#define MFV_ALLOC_MEM_CMD          _IOW(MFV_IOC_MAGIC, 0x06, unsigned int)
#define MFV_FREE_MEM_CMD           _IOW(MFV_IOC_MAGIC, 0x07, unsigned int)
#define MFV_MAKE_PMEM_TO_NONCACHED _IOW(MFV_IOC_MAGIC, 0x08, unsigned int*)
#define MFV_ALLOC_INT_MEM_CMD      _IOW(MFV_IOC_MAGIC, 0x09, VAL_INTMEM_T*)
#define MFV_FREE_INT_MEM_CMD       _IOW(MFV_IOC_MAGIC, 0x0a, VAL_INTMEM_T*)
#define MFV_SET_SYSRAM_FLAG_CMD    _IOW(MFV_IOC_MAGIC, 0x0b, unsigned int)
#define MFV_GET_SYSRAM_USER_NUM_CMD _IOW(MFV_IOC_MAGIC, 0x0c, unsigned int*)

//#define MFV_GET_CACHECTRLADDR_CMD  _IOR(MFV_IOC_MAGIC, 0x06, int)

#endif //__MT6573_MFLEXVIDEO_H__
