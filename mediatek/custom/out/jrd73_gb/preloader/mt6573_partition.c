
#include "nand.h"
#include "mt6573_typedefs.h"
#include "mt6573.h"
#include "mt6573_utils.h"
#include "mt6573_partition.h"
#include "partition_define.h"
#include "pmt.h"
extern struct nand_chip g_nand_chip;

u32 PAGE_SIZE;
u32 BLOCK_SIZE;
part_t mt6573_parts[PART_MAX_COUNT];
part_t mt6516_parts[PART_MAX_COUNT];

#define PMT 1
#ifdef PMT
pt_resident new_part[PART_MAX_COUNT];
pt_resident lastest_part[PART_MAX_COUNT];


#endif
//******************************************************
//* Modification History :
//*
//* 20100430  : (1) add secure partition
//* 20100513  : (2) add apanic partition
//* 20100601  : (3) add secstatic partition
//*
//****************************************************** 
void mt6573_part_init (void)
{
    u32 index = 0;

    part_t *part = &mt6573_parts[0];
    unsigned long lastblk;
    // init page size and block size
    PAGE_SIZE = (u32) g_nand_chip.page_size;
    BLOCK_SIZE = (u32) g_nand_chip.erasesize;

    // init partition size
#ifdef CFG_USE_MBL_PARTITION
    {
        mt6573_parts[index].name = PART_PRELOADER;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_PRELOADER);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        mt6573_parts[index].startblk = 0;
        index++;
    }
    {
        mt6573_parts[index].name = PART_DSP_DL;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_DSP_BL);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif
    {
        mt6573_parts[index].name = PART_PRO_INFO;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_PRO_INFO);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#ifdef CFG_USE_NVRAM_PARTITION
    {
        mt6573_parts[index].name = PART_NVRAM;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_NVRAM);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif

#ifdef CFG_USE_SECURE_PARTITION
    {
        mt6573_parts[index].name = PART_SECURE;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_SECCFG);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif

#ifdef CFG_USE_UBOOT_PARTITION
    {
        mt6573_parts[index].name = PART_UBOOT;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_UBOOT);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif

    //-------------------------------------------
    //-------------------------------------------
    // use Android Boot Image instead of kernel and rootfs
#ifdef CFG_USE_BOOTIMG_PARTITION
    {
        mt6573_parts[index].name = PART_BOOTIMG;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_BOOTIMG);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif
    //-------------------------------------------

#ifdef CFG_USE_RECOVERY_PARTITION
    {
        mt6573_parts[index].name = PART_RECOVERY;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_RECOVERY);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif

#ifdef CFG_USE_SECSTATIC_PARTITION
    {
        mt6573_parts[index].name = PART_SECSTATIC;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_SEC_RO);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif

#ifdef CFG_USE_MISC_PARTITION
    {
        mt6573_parts[index].name = PART_MISC;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_MISC);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif
#ifdef CFG_USE_LOGO_PARTITION
    {
        mt6573_parts[index].name = PART_LOGO;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_LOGO);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif
#ifdef CFG_USE_EXPDB_PARTITION
    {
        mt6573_parts[index].name = PART_EXPDB;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_EXPDB);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif
#ifdef CFG_USE_ANDROID_SYSIMG_PARTITION
    {
        mt6573_parts[index].name = PART_ANDSYSIMG;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_ANDROID);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif

#ifdef CFG_USE_CUSTPACK_PARTITION
    {
        mt6573_parts[index].name = PART_CUSTPACK;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_CUSTPACK);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif

#ifdef CFG_USE_CACHE_PARTITION
    {
        mt6573_parts[index].name = PART_CACHE;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_CACHE);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif

#ifdef CFG_USE_USER_PARTITION
    {
        mt6573_parts[index].name = PART_USER;
        mt6573_parts[index].pgnum = PAGE_NUM (PART_SIZE_USRDATA);
        mt6573_parts[index].flags = PART_FLAG_NONE;
        index++;
    }
#endif

    {
        mt6573_parts[index].name = NULL;
        mt6573_parts[index].pgnum = NULL;
        mt6573_parts[index].flags = PART_FLAG_END;
        index++;
    }
    lastblk = part->startblk + part->pgnum;

    while (1)
    {
        part++;
        if (!part->name)
            break;

        part->startblk = lastblk;
        lastblk = part->startblk + part->pgnum;;
    }
#ifdef PMT
	//watch out mt6516_parts[index].sechead must same as before. 
	//only change 
	{
		int retval=0;
		int i=0;
		//
		memset(&new_part,0,PART_MAX_COUNT*sizeof(pt_resident));
		memset(&lastest_part,0,PART_MAX_COUNT*sizeof(pt_resident));
		retval=load_exist_part_tab((u8 *)&lastest_part);
		if (retval==ERR_NO_EXIST) //first run preloader before dowload
		{
			//and valid mirror last download or first download 
			MSG (INIT, "no pt \n");
			get_part_tab_from_complier(); //get from complier
		}
		else
		{
			MSG (INIT, "Find pt \n");
			for(i=0;i<14;i++)
			{	
				MSG (INIT, "partition %s size %x %x \n",lastest_part[i].name,lastest_part[i].offset,lastest_part[i].size);
			}
		}
	}
#endif
}

#ifdef PMT
part_t tempart;
#endif
part_t * mt6573_part_get_partition (char *name)
{
	int index=0;
	part_t *part = &mt6573_parts[0];
#ifdef PMT
	
	MSG (INIT, "mt6516_part_get_partition %s\n", name);
#endif
    while (part->name)
      {
          if (!strcmp (name, part->name))
            {
#ifdef PMT
			tempart.name=part->name;
			//when download get partitin used new,orther wise used latest
			if( new_part[0].size!=0)
			{
				tempart.startblk = PAGE_NUM(new_part[index].offset);
				tempart.pgnum=PAGE_NUM(new_part[index].size);
			}
			else
			{
				tempart.startblk = PAGE_NUM(lastest_part[index].offset);
				tempart.pgnum=PAGE_NUM(lastest_part[index].size);
			}
			tempart.flags=part->flags;
			tempart.secHeader=part->secHeader;
			//MSG (INIT, "mt6516_part_get_partition %s\n", (&tempart)->name);
			MSG (INIT, "mt6516_part_get_partition %x\n", tempart.startblk);
			//MSG (INIT, "mt6516_part_get_partition %x\n", tempart.pgnum);
			//MSG (INIT, "mt6516_part_get_partition %x\n", tempart.flags);
			return &tempart;
#endif
                return part;
            }
		index++;
          part++;
      }
    return NULL;
}
