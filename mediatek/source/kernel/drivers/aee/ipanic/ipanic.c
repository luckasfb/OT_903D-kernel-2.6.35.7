

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/mtd/mtd.h>
#include <linux/notifier.h>
#include <linux/mtd/mtd.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/preempt.h>
#include <linux/aee.h>

#include "ipanic.h"

#ifdef CONFIG_ARCH_MT6516
#include <mach/mt6516_wdt.h>
#endif

extern unsigned log_start;
extern unsigned log_end;

#define AEE_IPANIC_PLABEL "expdb"

static struct ipanic_data drv_ctx;

static struct ipanic_oops_header oops_header;

static unsigned ipanic_detail_start;
static unsigned ipanic_detail_end;

static u32 ipanic_iv = 0xaabbccdd;

// QHQ patch for without AEE_AED, the 2nd panic data will croupt first one.
static have_panic = 0 ;



#if 1
static void ipanic_block_scramble(u8 *buf, int buflen) 
{
	int i;
	u32 *p = (u32 *)buf;
	for (i = 0; i < buflen; i += 4, p++) {
		*p = *p ^ ipanic_iv;
	}
}
#else 
static void ipanic_block_scramble(u8 *buf, int buflen) 
{
}
#endif

static int ipanic_block_scan(struct ipanic_data *ctx) 
{
	int index = 0, offset;
	
	for (offset = 0; (index < IPANIC_OOPS_BLOCK_COUNT) && (offset < ctx->mtd->size); offset += ctx->mtd->writesize) {
		if (!ctx->mtd->block_isbad(ctx->mtd, offset)) {
			ctx->blk_offset[index++] = offset;
		}
	}
	if (index != IPANIC_OOPS_BLOCK_COUNT) {
		printk(KERN_ERR "aee-ipanic: No enough partition space\n");
		return 0;
	}
	printk(KERN_INFO "aee-ipanic: blocks: ");
	for (index = 0; index < IPANIC_OOPS_BLOCK_COUNT; index++) {
		printk(" %d", ctx->blk_offset[index]);
	}
	printk("\n");
	return 1;
}

static int ipanic_block_read_single(struct ipanic_data *ctx, loff_t offset)
{
	int rc, len;
	int index = offset >> ctx->mtd->writesize_shift;

	if ((index < 0) || (index >= IPANIC_OOPS_BLOCK_COUNT)) {
		return -EINVAL;
	}

	rc = ctx->mtd->read(ctx->mtd, ctx->blk_offset[index], ctx->mtd->writesize, &len, ctx->bounce);
	ipanic_block_scramble(ctx->bounce, ctx->mtd->writesize);
#if 0
	if (rc == -EBADMSG) {
		printk(KERN_WARNING "Check sum error (ignore)\n");
		rc = 0;
	}
#endif
	if (rc == -EUCLEAN) {
		printk(KERN_WARNING "ECC Check sum error corrected %lld\n", offset);
		rc = 0;
	}
	if ((rc == 0) && (len != ctx->mtd->writesize)) {
		printk(KERN_WARNING "aee-ipanic: read size mismatch %d\n", len);
		return -EINVAL;
	}
	return rc;
}

static int ipanic_block_write(struct ipanic_data *ctx, loff_t to, int bounce_len)
{
	int rc;
	size_t wlen;
	int panic = in_interrupt() | in_atomic();
	int index = to >> ctx->mtd->writesize_shift;

	if ((index < 0) || (index >= IPANIC_OOPS_BLOCK_COUNT)) {
		return -EINVAL;
	}

	if (bounce_len > ctx->mtd->writesize) {
		printk(KERN_ERR "aee-ipanic(%s) len too large\n", __func__);
		return -EINVAL;
	}
	if (panic && !ctx->mtd->panic_write) {
		printk(KERN_EMERG "%s: No panic_write available\n", __func__);
		return 0;
	} else if (!panic && !ctx->mtd->write) {
		printk(KERN_EMERG "%s: No write available\n", __func__);
		return 0;
	}

	if (bounce_len < ctx->mtd->writesize)
		memset(ctx->bounce + bounce_len, 0, ctx->mtd->writesize - bounce_len);
	ipanic_block_scramble(ctx->bounce, ctx->mtd->writesize);

	if (panic)
		rc = ctx->mtd->panic_write(ctx->mtd, ctx->blk_offset[index], ctx->mtd->writesize, &wlen, ctx->bounce);
	else
		rc = ctx->mtd->write(ctx->mtd, ctx->blk_offset[index], ctx->mtd->writesize, &wlen, ctx->bounce);

	if (rc) {
		printk(KERN_EMERG
		       "%s: Error writing data to flash (%d)\n",
		       __func__, rc);
		return rc;
	}

	return wlen;
}

static int ipanic_block_read(struct ipanic_data *ctx, off_t file_offset, int file_length, void *buf)
{
#if 0
	printk("%s: ctx:%p file_offset:%d file_length:%lu\n", __func__, ctx, file_offset, file_length);
#endif
	while (file_length > 0) {
		unsigned int page_no;
		off_t page_offset;
		int rc;
		size_t count = file_length;

		/* We only support reading a maximum of a flash page */
		if (count > ctx->mtd->writesize)
			count = ctx->mtd->writesize;
		page_no = file_offset / ctx->mtd->writesize;
		page_offset = file_offset % ctx->mtd->writesize;

		rc = ipanic_block_read_single(ctx, page_no * ctx->mtd->writesize);
		if (rc < 0) {
			printk(KERN_ERR "aee-ipanic(%s): mtd read error page_no(%d) error(%d)\n", __func__, page_no, rc);
			goto error_return;
		}
		if (page_offset)
			count -= page_offset;
		memcpy(buf, ctx->bounce + page_offset, count);

		file_length -= count;
		buf += count;
		file_offset += count;
	}
	return 0;
error_return:
	return -1;
}

static void ipanic_block_erase_callback(struct erase_info *done)
{
	wait_queue_head_t *wait_q = (wait_queue_head_t *) done->priv;
	wake_up(wait_q);
}

static void ipanic_block_erase(void)
{
	struct ipanic_data *ctx = &drv_ctx;
	struct erase_info erase;
	DECLARE_WAITQUEUE(wait, current);
	wait_queue_head_t wait_q;
	int rc, i;
    
    // QHQ patch for without AEE_AED, the 2nd panic data will croupt first one.
    have_panic = 0 ;
	
	
	init_waitqueue_head(&wait_q);
	erase.mtd = ctx->mtd;
	erase.callback = ipanic_block_erase_callback;
	erase.len = ctx->mtd->erasesize;
	erase.priv = (u_long)&wait_q;
	for (i = 0; i < ctx->mtd->size; i += ctx->mtd->erasesize) {
		erase.addr = i;
		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&wait_q, &wait);

		rc = ctx->mtd->block_isbad(ctx->mtd, erase.addr);
		if (rc < 0) {
			printk(KERN_ERR
			       "aee-ipanic: Bad block check "
			       "failed (%d)\n", rc);
			goto out;
		}
		if (rc) {
			printk(KERN_WARNING
			       "aee-ipanic: Skipping erase of bad "
			       "block @%llx\n", erase.addr);
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&wait_q, &wait);
			continue;
		}

		rc = ctx->mtd->erase(ctx->mtd, &erase);
		if (rc) {
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&wait_q, &wait);
			printk(KERN_ERR
			       "aee-ipanic: Erase of 0x%llx, 0x%llx failed\n",
			       (unsigned long long) erase.addr,
			       (unsigned long long) erase.len);
			if (rc == -EIO) {
				if (ctx->mtd->block_markbad(ctx->mtd,
							    erase.addr)) {
					printk(KERN_ERR
					       "aee-ipanic: Err marking blk bad\n");
					goto out;
				}
				printk(KERN_INFO
				       "aee-ipanic: Marked a bad block"
				       " @%llx\n", erase.addr);
				continue;
			}
			goto out;
		}
		schedule();
		remove_wait_queue(&wait_q, &wait);
	}
	printk(KERN_DEBUG "aee-ipanic: %s partition erased\n",
	       AEE_IPANIC_PLABEL);
out:
	return;
}

void ipanic_oops_start(const char *str, int err, struct task_struct *tsk)
{
	int plen;

	memset(&oops_header, 0, sizeof(struct ipanic_oops_header));
	plen = 0;
	while (tsk && (tsk->pid != 0) && (tsk->pid != 1)) {
		plen += sprintf(oops_header.process_path + plen, "[%s, %d]", tsk->comm, tsk->pid);
	  tsk = tsk->real_parent;
	}
	/* Put in the back, in case of DAL/beep function output kernel message */
	ipanic_detail_start = log_end;
}

void ipanic_oops_end(void)
{
	ipanic_detail_end = log_end;
}

void ipanic_stack_store(unsigned long where, unsigned long from)
{
	char symbol_where[KSYM_SYMBOL_LEN], symbol_from[KSYM_SYMBOL_LEN];
	int off = strlen(oops_header.backtrace);

	/* Check backtrace overflow */
	if ((IPANIC_OOPS_HEADER_BACKTRACE_LENGTH - off) > KSYM_SYMBOL_LEN * 2 + 128) {
		sprint_symbol(symbol_where, where);
		sprint_symbol(symbol_from, from);
		sprintf(oops_header.backtrace + off, "where [<%08lx>] (%s) from [<%08lx>] (%s)\n", where, symbol_where, from, symbol_from);
	}
}

static int ipanic_proc_oops(char *page, char **start,
			    off_t off, int count,
			    int *eof, void *data)
{
	int len;
	struct aee_oops *oops = ipanic_oops_copy();
	len = sprintf(page, "aee-ipanic Oops\n");
	if (oops) {
	  len += sprintf(page + len, "Process\n%s\nBacktrace\n%s", oops->process_path,
			       oops->backtrace);
		ipanic_oops_free(oops, 0);
	}
	else {
		len += sprintf(page + len, "No available\n");
	}
	return len;
}

static void ipanic_oops_dump(void)
{
	struct aee_oops *oops = ipanic_oops_copy();
	if (oops != NULL) {
		char *log_buf;
		int i;

		printk(KERN_WARNING "[---- Panic Log ----]\n");
		printk(KERN_WARNING "Process path: %s\n", oops->process_path);
		printk(KERN_WARNING "Backtrace:\n%s", oops->backtrace);

                printk(KERN_WARNING "Detail:\n");
                log_buf = kmalloc(1024, GFP_KERNEL);
                 for (i = 0; i < oops->detail_len; i+= 1023)
                  {
		    int len = (oops->detail_len - i) > 1023 ? 1023 : oops->detail_len - i;
		    memcpy(log_buf, oops->detail + i, len);
		    log_buf[len] = 0;

		    printk("%s", log_buf);
                  }
                kfree(log_buf);
		printk(KERN_WARNING "\n[---- Panic Log End ----]\n");

		ipanic_oops_free(oops, 0);
	}
	else {
		printk(KERN_INFO "[---- No Panic Log available ----]\n");
	}
}

static int ipanic_check_header(const struct ipanic_header *hdr) 
{
	if (hdr->magic != AEE_IPANIC_MAGIC) {
		printk(KERN_INFO "aee-ipanic: No panic data available [Magic header]\n");
		return 1;
	}

	if (hdr->version != AEE_IPANIC_PHDR_VERSION) {
		printk(KERN_INFO "aee-ipanic: Version mismatch (%d != %d)\n",
		       hdr->version, AEE_IPANIC_PHDR_VERSION);
		return 2;
	}

	if ((hdr->oops_header_length < 16) || (hdr->oops_header_length > AEE_IPANIC_DATALENGTH_MAX)) {
		printk(KERN_INFO "aee-ipanic: No panic data available [Invalid oops header length - %d]\n", hdr->oops_header_length);
		return 2;
	}

	if ((hdr->oops_detail_length < 16) || (hdr->oops_detail_length > AEE_IPANIC_DATALENGTH_MAX)) {
		printk(KERN_INFO "aee-ipanic: No panic data available [Invalid oops detail length - %d]\n", hdr->oops_detail_length);
		return 2;
	}

	if ((hdr->console_length < 16) || (hdr->console_length > 64 * 1024)) {
		printk(KERN_INFO "aee-ipanic: No panic data available [Invalid oops console length - %d]\n", hdr->console_length);
		return 2;
	}
	return 0;
}

static void mtd_panic_notify_add(struct mtd_info *mtd)
{
	struct ipanic_data *ctx = &drv_ctx;
	struct ipanic_header *hdr = ctx->bounce;
	int rc;

	if (strcmp(mtd->name, AEE_IPANIC_PLABEL))
		return;

	ctx->mtd = mtd;

	if (!ipanic_block_scan(ctx))
		goto out_err;

	rc = ipanic_block_read_single(ctx, 0);
	if (rc < 0) {
		printk(KERN_ERR "aee-ipanic: Error reading block 0 (%d)\n", rc);
		ipanic_block_erase();
		goto out_err;
	}

	printk(KERN_INFO "aee-ipanic: Bound to mtd partition '%s'\n", mtd->name);

	switch (ipanic_check_header(hdr)) {
	case 0:
	    // QHQ patch for without AEE_AED, the 2nd panic data will croupt first one.
        have_panic = 1 ;
		break;
	case 1:
		return;
	case 2:
		ipanic_block_erase();
		return;
	}

	memcpy(&ctx->curr, hdr, sizeof(struct ipanic_header));

	printk(KERN_INFO "aee-ipanic: c(%u, %u) oh(%u, %u) od(%u, %u)\n",
	       hdr->console_offset, hdr->console_length,
	       hdr->oops_header_offset, hdr->oops_header_length,
	       hdr->oops_detail_offset, hdr->oops_detail_length);

	ctx->oops = create_proc_read_entry("aee_ipanic_oops", 
					   0444, NULL, 
					   ipanic_proc_oops,
					   NULL);
	if (ctx->oops == NULL) {
		printk(KERN_ERR " %s: failed crate procfile apanic_oops\n", __func__);
	}

	ipanic_oops_dump();
	return;

out_err:
	ctx->mtd = NULL;
}

static void mtd_panic_notify_remove(struct mtd_info *mtd)
{
	struct ipanic_data *ctx = &drv_ctx;
	if (mtd == ctx->mtd) {
		ctx->mtd = NULL;
		printk(KERN_INFO "aee-ipanic: Unbound from %s\n", mtd->name);
	}
}

static struct mtd_notifier mtd_panic_notifier = {
	.add	= mtd_panic_notify_add,
	.remove	= mtd_panic_notify_remove,
};

static int in_panic = 0;


extern int log_buf_copy2(char *dest, int dest_len, int log_copy_start, int log_copy_end);
extern int log_buf_get_len(void);
extern void log_buf_clear(void);

static int ipanic_write_log_buf(struct mtd_info *mtd, unsigned int off, int log_copy_start, int log_copy_end)
{
	struct ipanic_data *ctx = &drv_ctx;
	int saved_oip;
	int rc, rc2;
	unsigned int last_chunk = 0, copy_count = 0;

	while (!last_chunk) {
		saved_oip = oops_in_progress;
		oops_in_progress = 1;
		rc = log_buf_copy2(ctx->bounce, mtd->writesize, log_copy_start, log_copy_end);
		BUG_ON(rc < 0);
		log_copy_start += rc;
		copy_count += rc;
		if (rc != mtd->writesize)
			last_chunk = rc;

		oops_in_progress = saved_oip;
		if (rc <= 0)
			break;

		rc2 = ipanic_block_write(ctx, off, rc);
		if (rc2 <= 0) {
			printk(KERN_EMERG
			       "aee-ipanic: Flash write failed (%d)\n", rc2);
			return rc2;
		}
		off += rc2;
	}
	return copy_count;
}

static int ipanic_write_console(struct mtd_info *mtd, unsigned int off)
{
	return ipanic_write_log_buf(mtd, off, log_start, log_end);
}

static int ipanic_write_oops_header(struct mtd_info *mtd, unsigned int off)
{
	int wlen = 0, rc;
	struct ipanic_data *ctx = &drv_ctx;
	u8 *raw_oops_header = (u8 *)&oops_header;
	while (wlen < sizeof(struct ipanic_oops_header)) {
		int writesize = sizeof(struct ipanic_oops_header) - wlen;
		if (writesize > mtd->writesize)
			writesize = mtd->writesize;

		memcpy(ctx->bounce, raw_oops_header + wlen, writesize);
		rc = ipanic_block_write(ctx, off + wlen, writesize);
		if (rc < 0) {
			return rc;
		}
		wlen += writesize;
	}
	return wlen;
}

static int ipanic_write_oops_detail(struct mtd_info *mtd, unsigned int off)
{
	return ipanic_write_log_buf(mtd, off, ipanic_detail_start, ipanic_detail_end);
}

struct aee_oops *ipanic_oops_copy(void)
{
	struct ipanic_data *ctx = &drv_ctx;
	struct aee_oops *oops;

	if ((ctx->curr.magic != AEE_IPANIC_MAGIC) || (ctx->curr.version != AEE_IPANIC_PHDR_VERSION)) {
	  return NULL;
	}

	oops = aee_oops_create(AE_DEFECT_FATAL, IPANIC_MODULE_TAG);
	if (oops != NULL) {
		struct ipanic_oops_header *oops_header = kzalloc(sizeof(struct ipanic_oops_header), GFP_KERNEL);
		if (oops_header == NULL)
			goto error_return;

		if (ipanic_block_read(ctx, ctx->curr.oops_header_offset, ctx->curr.oops_header_length, oops_header) != 0) {
			printk(KERN_ERR "%s: mtd read header failed\n", __FUNCTION__);
			kfree(oops_header);
			goto error_return;
		}
		aee_oops_set_process_path(oops, oops_header->process_path);
		aee_oops_set_backtrace(oops, oops_header->backtrace);
		kfree(oops_header);

		oops->detail = kmalloc(ctx->curr.oops_detail_length, GFP_KERNEL);
		oops->detail_len = ctx->curr.oops_detail_length;
		if (oops->detail != NULL) {
			if (ipanic_block_read(ctx, ctx->curr.oops_detail_offset, ctx->curr.oops_detail_length, oops->detail) != 0) {
				printk(KERN_ERR "%s: mtd read detail failed\n", __FUNCTION__);
				kfree(oops->detail);
				goto error_return;
			}
		}
		else {
			printk(KERN_ERR "%s: kmalloc failed at detail\n", __FUNCTION__);
			kfree(oops);
			return NULL;
		}

		oops->console = kmalloc(ctx->curr.console_length, GFP_KERNEL);
		oops->console_len = ctx->curr.console_length;
		if (oops->console != NULL) {
			if (ipanic_block_read(ctx, ctx->curr.console_offset, ctx->curr.console_length, oops->console) != 0) {
				printk(KERN_ERR "%s: mtd read console failed\n", __FUNCTION__);
				kfree(oops->detail);
				goto error_return;
			}
		}
		else {
			printk(KERN_ERR "%s: kmalloc failed at detail\n", __FUNCTION__);
			kfree(oops);
			return NULL;
		}
		return oops;
	}
	else {
		printk(KERN_ERR "%s: kmalloc failed at header\n", __FUNCTION__);
		return NULL;
	}
error_return:
	kfree(oops);
	memset(&ctx->curr, 0, sizeof(struct ipanic_header));
	ipanic_block_erase();
	return NULL;
}
EXPORT_SYMBOL(ipanic_oops_copy);

void ipanic_oops_free(struct aee_oops *oops, int erase)
{
	if (oops) {
		struct ipanic_data *ctx = &drv_ctx;

		aee_oops_free(oops);

		if (erase) {
			memset(&ctx->curr, 0, sizeof(struct ipanic_header));
			ipanic_block_erase();
		}
	}
}
EXPORT_SYMBOL(ipanic_oops_free);

static int ipanic(struct notifier_block *this, unsigned long event,
			void *ptr)
{
	struct ipanic_data *ctx = &drv_ctx;
	struct ipanic_header *hdr = (struct ipanic_header *) ctx->bounce;
	int console_offset = 0;
	int console_len = 0;
	int oops_header_offset = 0;
	int oops_header_len = 0;
	int oops_detail_offset = 0;
	int oops_detail_len = 0;
	int rc;
	
	// QHQ patch for without AEE_AED, the 2nd panic data will croupt first one.
    if (have_panic)
    {
        return NOTIFY_DONE;
    }


	if (in_panic)
		return NOTIFY_DONE;

	in_panic = 1;
#ifdef CONFIG_PREEMPT
	/* Ensure that cond_resched() won't try to preempt anybody */
	add_preempt_count(PREEMPT_ACTIVE);
#endif

	if (!ctx->mtd)
		goto out;
#if 0
	if (ctx->curr.magic) {
		printk(KERN_EMERG "Crash partition in use!\n");
		goto out;
	}
#endif

#if defined(CONFIG_ARCH_MT6516)
	/* Disable hardware watchdog to prevent reboot in progress of write */
	mt6516_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
#endif

	/*
	 * Write out the console
	 * Section 0 is reserved for ipanic header, we start at section 1
	 */
	oops_header_offset =  ctx->mtd->writesize;
	oops_header_len = ipanic_write_oops_header(ctx->mtd, oops_header_offset);
	if (oops_header_len < 0) {
		printk(KERN_EMERG "Error writing oops header to panic log! (%d)\n",
		       oops_header_len);
		oops_header_len  = 0;
	}

	oops_detail_offset = ALIGN(oops_header_offset + oops_header_len,
				   ctx->mtd->writesize);
	oops_detail_len = ipanic_write_oops_detail(ctx->mtd, oops_detail_offset);
	if (oops_detail_len < 0) {
		printk(KERN_EMERG "Error writing oops header to panic log! (%d)\n",
		       oops_detail_len);
		oops_detail_len  = 0;
	}

	console_offset = ALIGN(oops_detail_offset + oops_detail_len,
			       ctx->mtd->writesize);
	console_len = ipanic_write_console(ctx->mtd, console_offset);
	if (console_len < 0) {
		printk(KERN_EMERG "Error writing console to panic log! (%d)\n",
		       console_len);
		console_len = 0;
	}

	/*
	 * Finally write the ipanic header
	 */
	memset(ctx->bounce, 0, PAGE_SIZE);
	hdr->magic = AEE_IPANIC_MAGIC;
	hdr->version = AEE_IPANIC_PHDR_VERSION;

	hdr->oops_header_offset = oops_header_offset;
	hdr->oops_header_length = oops_header_len;

	hdr->oops_detail_offset = oops_detail_offset;
	hdr->oops_detail_length = oops_detail_len;

	hdr->console_offset = console_offset;
	hdr->console_length = console_len;

	rc = ipanic_block_write(ctx, 0, sizeof(struct ipanic_header));
	if (rc <= 0) {
		printk(KERN_EMERG "aee-ipanic: Header write failed (%d)\n",
		       rc);
		goto out;
	}
	printk(KERN_EMERG "aee-ipanic: Panic dump sucessfully written to flash (detail len: %d, console len: %d)\n", oops_detail_len, console_len);

 out:
#ifdef CONFIG_PREEMPT
	sub_preempt_count(PREEMPT_ACTIVE);
#endif
	in_panic = 0;
	return NOTIFY_DONE;
}

static struct notifier_block panic_blk = {
	.notifier_call	= ipanic,
};

int __init aee_ipanic_init(void)
{
	register_mtd_user(&mtd_panic_notifier);
	atomic_notifier_chain_register(&panic_notifier_list, &panic_blk);
	memset(&drv_ctx, 0, sizeof(drv_ctx));
	drv_ctx.bounce = (void *) __get_free_page(GFP_KERNEL);
	printk(KERN_INFO "aee-ipanic: startup, partition assgined %s\n",
	       AEE_IPANIC_PLABEL);
	return 0;
}

module_init(aee_ipanic_init);
