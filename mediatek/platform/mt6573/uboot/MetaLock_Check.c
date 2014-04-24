

#include <common.h>
#include <command.h>
#include <image.h>
#include <malloc.h>
#include <fat.h>
#include <asm/errno.h>
#include <asm/byteorder.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_image.h>
#include <asm/arch/boot_mode.h>
#include <asm/arch/bootimg.h>
#include <asm/mach-types.h>

#include "mt65xx_partition.h"
#include <asm/arch/MetaLock_Check.h>
#include <linux/mtd/mtd.h>
#include <nand.h>

//-----------------------------------------------------------------------------
#define END_BLOCK 23*128*1024  //Magic Num is in the last block of LOGO part,so the offset addr of end block is 23*128*1024
#define BLOCK_SIZE 128*1024    //block size of LOGO part
#define LOGO_PART   "LOGO"
#define LOGO_PART_SIZE 24*128*1024 //size of LOGO part,num of LOGO bolck is 24
#define MODULE_NAME "MBOOT"



static int mboot_common_load_part_info(part_dev_t *dev, char *part_name, part_hdr_t *part_hdr)
{
    long len;
    ulong addr;
    part_t *part;    

    part = mt6573_part_get_partition(part_name);   
    addr = part->startblk * BLK_SIZE;
    
    //***************
    //* read partition header
    //*
    len = dev->read(dev, addr, (uchar*)part_hdr, sizeof(part_hdr_t));
    if (len < 0) {
        printf("[%s] %s partition read error. LINE: %d\n", MODULE_NAME, part_name, __LINE__);
        return -1;
    }

	  printf("\n=========================================\n");
    printf("[%s] %s magic number : 0x%x\n",MODULE_NAME,part_name,part_hdr->info.magic);
    part_hdr->info.name[31]='\0'; //append end char
    printf("[%s] %s name         : %s\n",MODULE_NAME,part_name,part_hdr->info.name);
    printf("[%s] %s size         : %d\n",MODULE_NAME,part_name,part_hdr->info.dsize);
	  printf("=========================================\n");

	//***************
    //* check partition magic
	//*
    if (part_hdr->info.magic != PART_MAGIC) {
        printf("[%s] %s partition magic error\n", MODULE_NAME, part_name);
        return -1;
    }

	//***************
    //* check partition name
    //*
    if (strncmp(part_hdr->info.name, part_name, sizeof(part_hdr->info.name))) {
        printf("[%s] %s partition name error\n", MODULE_NAME, part_name);
        return -1;
    }

	//***************
    //* check partition data size
    //*
    if (part_hdr->info.dsize > part->blknum * BLK_SIZE) {
        printf("[%s] %s partition size error\n", MODULE_NAME, part_name);
        return -1;
    }
    
    return 0;
}

static int uboot_load_part_meta_check(char *part_name, unsigned long addr)
{
    long len;
	  unsigned long begin;
	  unsigned long start_addr;	
    part_t *part;	
    part_dev_t *dev;
    part_hdr_t *part_hdr;
    int iReadSize=BLOCK_SIZE;
    printf("enter uboot_load_part_meta_check\n");
    dev = mt6573_part_get_device();
    if (!dev)
    {
     printf("mt6573_part_get_device() failed\n");
    return -1;
    }

    part = mt6573_part_get_partition(part_name);
    if (!part)
    {
    printf("mt6573_part_get_partition()failed \n");
    return -1;
    }

    start_addr = part->startblk * BLK_SIZE;   

    part_hdr = (part_hdr_t*)malloc(sizeof(part_hdr_t));

    
    if (!part_hdr)
    {
    printf("malloc failed\n");
    return -1;
    }
    len = mboot_common_load_part_info(dev, part_name, part_hdr);
    if (len < 0) {
        len = -1;        
        goto exit;
    }
	//****************
    //* read image data
	//*
	  printf("read the data of %s\n", part_name);
    len = dev->read(dev, start_addr+END_BLOCK , (uchar*)addr, iReadSize);    
    if (len < 0) {
		    printf("read failed\n");
        len = -1;
        goto exit;
    }
    
   
exit:
    if (part_hdr) 
        free(part_hdr);

    return len;
}

int UBoot_MetaLock_Check()
{
	int result;
	unsigned int iReadValue;
	unsigned char *buf=NULL;
	printf("enter meta mode check!!!!!\n");
	buf=(unsigned char*)malloc(BLOCK_SIZE);
	if(buf==NULL)
		{ printf("buffer malloced error!!!!!!!!!!!!!!!!\n");
			return 0;
		}
	memset(buf,0,BLOCK_SIZE);
	result=uboot_load_part_meta_check(LOGO_PART, (unsigned long)buf);
	printf("read len:%d\n",result);
	memcpy(&iReadValue,buf,sizeof(unsigned int));
	printf("iReadValue1:%d\n",iReadValue);
	if(iReadValue!=META_LOCK_MAGICNUM)
	{   
	     //META_LOG("[META_LOCK]:meta not locked,iReadValue:%d,iMagicNum:%d\r\n",iReadValue,META_LOCK_MAGIC_NUM);
	     printf("meta not locked,iReadValue:%d,iMagicNum:%d\n",iReadValue,META_LOCK_MAGICNUM);
	     free(buf);
	     return 0;
	}
  printf("meta locked\n");
  free(buf);
	return 1;
}

