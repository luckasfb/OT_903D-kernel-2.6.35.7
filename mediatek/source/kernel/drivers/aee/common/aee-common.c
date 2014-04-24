
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/aee.h>

static struct aee_kernel_api *g_aee_api = NULL;

struct aee_oops *aee_oops_create(AE_DEFECT_ATTR attr, const char *module)
{
	struct aee_oops *oops = kzalloc(sizeof(struct aee_oops), GFP_KERNEL);
	oops->attr = attr;
	if (module != NULL) {
		strlcpy(oops->module, module, sizeof(oops->module));
	}
	else {
		strcpy(oops->module, "N/A");
	}
	strcpy(oops->backtrace, "N/A");
	strcpy(oops->process_path, "N/A");
	
	return oops;
}
EXPORT_SYMBOL(aee_oops_create);

void aee_oops_set_process_path(struct aee_oops *oops, const char *process_path) 
{
	if (process_path != NULL) {
		strlcpy(oops->process_path, process_path, sizeof(oops->process_path));
	}
}

void aee_oops_set_backtrace(struct aee_oops *oops, const char *backtrace) 
{
	if (backtrace != NULL) {
		strlcpy(oops->backtrace, backtrace, sizeof(oops->backtrace));
	}
}

void aee_oops_free(struct aee_oops *oops) 
{
	if (oops->detail) {
		kfree(oops->detail);
	}
	if (oops->console) {
		kfree(oops->console);
	}
	kfree(oops);
}
EXPORT_SYMBOL(aee_oops_free);

int aee_register_api(struct aee_kernel_api *aee_api)
{
	if (!aee_api) {
		BUG();
	}

    g_aee_api = aee_api;
	return 0;
}
EXPORT_SYMBOL(aee_register_api);

void aee_kernel_exception(const char *module, const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    if(g_aee_api && g_aee_api->kernel_exception) {
        char *msgbuf = kmalloc(128, GFP_KERNEL);
        vsnprintf(msgbuf, 128, msg, args);
        g_aee_api->kernel_exception(module,msgbuf);
    } else {
        printk(KERN_ERR "%s: ", module);
        vprintk(msg, args);
    }
	va_end(args);
}
EXPORT_SYMBOL(aee_kernel_exception);

void aee_kernel_warning(const char *module, const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    if(g_aee_api && g_aee_api->kernel_warning) {
        char *msgbuf = kmalloc(128, GFP_KERNEL);
        vsnprintf(msgbuf, 128, msg, args);
        g_aee_api->kernel_warning(module,msgbuf);
    } else {
        printk(KERN_ERR "%s: ", module);
        vprintk(msg, args);
    }
	va_end(args);
}
EXPORT_SYMBOL(aee_kernel_warning);

void aee_kernel_reminding(const char *module, const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    if(g_aee_api && g_aee_api->kernel_reminding) {
        char *msgbuf = kmalloc(128, GFP_KERNEL);
        vsnprintf(msgbuf, 128, msg, args);
        g_aee_api->kernel_reminding(module,msgbuf);
    } else {
        printk(KERN_ERR "%s: ", module);
        vprintk(msg, args);
    }
	va_end(args);
}
EXPORT_SYMBOL(aee_kernel_reminding);

void aed_md_exception1(const int *log, int log_size, const int *phy, int phy_size, const char *assert_type, const char *exp_filename, unsigned int exp_linenum, unsigned int fatal1, unsigned int fatal2)
{
    if(g_aee_api && g_aee_api->md_exception1)
            g_aee_api->md_exception1(log, log_size, phy,phy_size,assert_type,
                                    exp_filename,exp_linenum,fatal1,fatal2);  
}
EXPORT_SYMBOL(aed_md_exception1);

