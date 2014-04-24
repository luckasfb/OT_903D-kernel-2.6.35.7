





#ifndef __METALOCK_CHECK_H__
#define __METALOCK_CHECK_H__


#define META_LOCK_MAGICNUM		0x58D12AB4

#ifdef __cplusplus
extern "C"
{
#endif
    

    /********************************************************************************
    //FUNCTION:
    //		UBoot_MetaLock_Check
    //DESCRIPTION:
    //		this function is called to check meta mode lock.
    //
    //PARAMETERS:
    //		req:
    //
    //RETURN VALUE:
    //		1: is locked, otherwise is not locked. 
    //
    //DEPENDENCY:
    //		
    //
    //GLOBALS AFFECTED
    ********************************************************************************/
    
    int UBoot_MetaLock_Check();
    static int uboot_load_part_meta_check(char *part_name, unsigned long addr);
    static int mboot_common_load_part_info(part_dev_t *dev, char *part_name, part_hdr_t *part_hdr);
#ifdef __cplusplus
}
#endif

#endif
