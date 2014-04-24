
#ifndef __G2D_6573_FUNC_H__
#define __G2D_6573_FUNC_H__

#include "g2d_drv.h"

void g2d_drv_start(g2d_context_t *g2d_ctx);

void g2d_drv_reset(void);

unsigned int g2d_drv_get_dst_addr(void);

int g2d_drv_get_status(void);

int g2d_drv_get_rmmu_flag(void);

#endif
