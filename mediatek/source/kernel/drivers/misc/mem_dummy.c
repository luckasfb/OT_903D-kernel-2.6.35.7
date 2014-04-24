
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/mem_dummy.h>
#include <asm/tlbflush.h>
#include <asm/page.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define MEM_DUMMY_DEVNAME "mem_dummy"

static dev_t mem_dummy_devno;
static struct cdev *mem_dummy_cdev;
static struct class *mem_dummy_class = NULL;

static int mem_dummy_release(struct inode *, struct file *);
static int mem_dummy_open(struct inode *, struct file *);
static int mem_dummy_ioctl(struct inode *, struct file *, unsigned int, unsigned long);

static unsigned long v2p(unsigned long va)
{
    unsigned long pageOffset = (va & (PAGE_SIZE - 1));
    pgd_t *pgd;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long pa;

    pgd = pgd_offset(current->mm, va); /* what is tsk->mm */
    pmd = pmd_offset(pgd, va);
    pte = pte_offset_map(pmd, va);
    
    pa = (pte_val(*pte) & (PAGE_MASK)) | pageOffset;

#if 0
    printk("virtual to physical (ver1): 0x%lx --> 0x%lx\n", va, pa);
    printk("virtual to physical (ver2): 0x%lx --> 0x%lx\n", va, v2p_ver2(va));
#endif

    return pa;
}

static int mem_dummy_release(struct inode *inode, struct file *file)
{
    return 0;
}

static int mem_dummy_open(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t mem_dummy_read(struct file *file, char __user *data, size_t len, loff_t *ppos)
{
    return 0;
}

static ssize_t mem_dummy_write(struct file *file, const char __user *data, size_t len, loff_t *ppos)
{
    return 0;
}

static int mem_dummy_ioctl(
    struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case MEM_DUMMY_VA2PA:
        {
            VAtoPA kVAtoPA;
            void __user *argp = (void __user *)arg;
            if (copy_from_user(&kVAtoPA, argp, sizeof(VAtoPA))){
                printk(KERN_ERR"[mem_dummy] copy_from_user fail\n");
                return -EFAULT;
            }            
            kVAtoPA.pa = v2p(kVAtoPA.va);
            if (copy_to_user(argp, &kVAtoPA, sizeof(VAtoPA))){
                printk(KERN_ERR"[mem_dummy] copy_to_user fail\n");
                return -EFAULT;
            }
        }
        break;
    }
    return 0;
}

struct file_operations mem_dummy_fops = {
    .owner   = THIS_MODULE,
    .ioctl   = mem_dummy_ioctl,
    .open    = mem_dummy_open,    
    .release = mem_dummy_release,
    .read    = mem_dummy_read,
    .write   = mem_dummy_write,
};

static int mem_dummy_probe(struct platform_device *pdev)
{
    struct class_device *class_dev = NULL;
    int ret = alloc_chrdev_region(&mem_dummy_devno, 0, 1, MEM_DUMMY_DEVNAME);
    mem_dummy_cdev = cdev_alloc();
    mem_dummy_cdev->owner = THIS_MODULE;
    mem_dummy_cdev->ops = &mem_dummy_fops;
    ret = cdev_add(mem_dummy_cdev, mem_dummy_devno, 1);
    mem_dummy_class = class_create(THIS_MODULE, MEM_DUMMY_DEVNAME);
    class_dev = (struct class_device *)device_create(
        mem_dummy_class, NULL, mem_dummy_devno,	NULL, MEM_DUMMY_DEVNAME);
    return 0;
}

static int mem_dummy_remove(struct platform_device *pdev)
{
    return 0;
}

static struct platform_driver mem_dummy_driver = {
    .probe  = mem_dummy_probe,
    .remove = mem_dummy_remove,
    .driver = { .name = MEM_DUMMY_DEVNAME }
};

static struct platform_device mem_dummy_device = {
    .name = MEM_DUMMY_DEVNAME,
    .id   = 0,
};

static int __init mem_dummy_init(void)
{
    if (platform_device_register(&mem_dummy_device)) 
    {
        return -ENODEV;
    }
    if (platform_driver_register(&mem_dummy_driver))
    {
        platform_device_unregister(&mem_dummy_device);
        return -ENODEV;
    }
    return 0;
}

static void __exit mem_dummy_exit(void)
{
    device_destroy(mem_dummy_class, mem_dummy_devno);    
    class_destroy(mem_dummy_class);
    cdev_del(mem_dummy_cdev);
    unregister_chrdev_region(mem_dummy_devno, 1);
    
    platform_driver_unregister(&mem_dummy_driver);
    platform_device_unregister(&mem_dummy_device);
}

module_init(mem_dummy_init);
module_exit(mem_dummy_exit);
MODULE_AUTHOR("Nick, Huang <nick.huang@mediatek.com>");
MODULE_DESCRIPTION("Memory Dummy Driver");
MODULE_LICENSE("GPL");
