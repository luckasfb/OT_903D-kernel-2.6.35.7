
#include <common.h>
#include "mt65xx_partition.h"

#define BOOTIMG_ADDR 0x10000000
#define BOOTIMG_SIZE 0x300000

extern part_dev_t *mt6573_part_get_device(void);

int download_img(void)
{
    part_dev_t *dev;
    part_t *part;
    long len;

    dev = mt6573_part_get_device();
    if (dev == NULL) {
        printf("fail to get the device\n");
        return -1;
    }
    part = mt6573_part_get_partition(PART_BOOTIMG);
    if (part == NULL) {
        printf("fail to get the BOOTIMG partition\n");
        return -1;
    }

    len = dev->write(dev, (uchar *)BOOTIMG_ADDR, (part->startblk) * BLK_SIZE, BOOTIMG_SIZE);
    if (len != BOOTIMG_SIZE) {
        printf("fail to write the image\n");
        return -1;
    }

    return 0;
}

