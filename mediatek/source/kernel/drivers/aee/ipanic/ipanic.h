
#if !defined(__AEE_IPANIC_H__)
#define __AEE_IPANIC_H__

#include <linux/autoconf.h>
#include <linux/kallsyms.h>


#define IPANIC_MODULE_TAG "AEE-IPANIC"
#define IPANIC_OOPS_BLOCK_COUNT 64

#define AEE_IPANIC_MAGIC 0xaee0dead
#define AEE_IPANIC_PHDR_VERSION   0x03
#define AEE_IPANIC_DATALENGTH_MAX (64 * 1024)

struct ipanic_header {
	/* The magic/version field cannot be moved or resize */
	u32 magic;
	u32 version;

	u32 oops_header_offset;
	u32 oops_header_length;

	u32 oops_detail_offset;
	u32 oops_detail_length;

	u32 console_offset;
	u32 console_length;
};

#define IPANIC_OOPS_HEADER_PROCESS_NAME_LENGTH 256
#define IPANIC_OOPS_HEADER_BACKTRACE_LENGTH 3840

struct ipanic_oops_header 
{
	char process_path[IPANIC_OOPS_HEADER_PROCESS_NAME_LENGTH];
	char backtrace[IPANIC_OOPS_HEADER_BACKTRACE_LENGTH];
};

struct ipanic_data {
	struct mtd_info		*mtd;
	struct ipanic_header	curr;
	void			*bounce;
	u32 blk_offset[IPANIC_OOPS_BLOCK_COUNT];

	struct proc_dir_entry	*oops;
};

struct aee_oops *ipanic_oops_copy(void);

void ipanic_oops_free(struct aee_oops *oops, int erase);

#endif
