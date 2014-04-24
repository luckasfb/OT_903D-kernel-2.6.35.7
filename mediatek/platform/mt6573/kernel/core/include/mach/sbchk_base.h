
#ifndef _SBCHK_BASE_H
#define _SBCHK_BASE_H

#define HASH_OUTPUT_LEN                     (20)

#define SBCHK_ENGINE_PATH                   "/system/bin/sbchk"

#define SBCHK_ENGINE_HASH_CHECK             (0)

#if SBCHK_ENGINE_HASH_CHECK

#error must fill the hash value of '/system/bin/sbchk'


//#define SBCHK_ENGINE_HASH "3a816d2e275818cb12b839a10e838a1e10d729f7"
#define SBCHK_ENGINE_HASH ?????????????????????????????????????

#endif

#define SEC_OK                              (0x0000)
#define SBCHK_BASE_ENGINE_OPEN_FAIL         (0x1000)
#define SBCHK_BASE_ENGINE_READ_FAIL         (0x1001)
#define SBCHK_BASE_HASH_INIT_FAIL           (0x1002)
#define SBCHK_BASE_HASH_DATA_FAIL           (0x1003)
#define SBCHK_BASE_HASH_CHECK_FAIL          (0x1004)

extern void sbchk_base(void);

#endif   /*_SBCHK_BASE*/
