
#include "g2d_6573_reg.h"
#include "g2d_6573_func.h"

void g2d_drv_set_src_info(g2d_context_t *g2d_ctx, unsigned int *fire_mode_ctrl_reg)
{
    unsigned int bpp = 1;

    REG_G2D_SRC_BASE = (unsigned int)g2d_ctx->gSrcAddr;
    REG_G2D_SRC_SIZE = ((g2d_ctx->gSrcWidth- 1) << 16) | (g2d_ctx->gSrcHeight- 1);

    REG_G2D_SMODE_CON &= ~(G2D_SMODE_CON_BIDX_EN | G2D_SMODE_CON_BIDX_MASK);
    *fire_mode_ctrl_reg &= ~(G2D_FMODE_CON_SRC_CLR_MODE_MASK | G2D_FMODE_CON_SRC_SWAP);

    switch(g2d_ctx->gSrcColorFormat)
    {
        case G2D_COLOR_GRAY :
            *fire_mode_ctrl_reg |= G2D_FMODE_CON_SRC_CLR_LUT_DIS;
            bpp = 1;
            break;
        case G2D_COLOR_GRAY_SWAP:
            *fire_mode_ctrl_reg |= (G2D_FMODE_CON_SRC_CLR_LUT_DIS | G2D_FMODE_CON_SRC_SWAP);
            bpp = 1;
            break;
        case G2D_COLOR_RGB565 :
            *fire_mode_ctrl_reg |= G2D_FMODE_CON_SRC_CLR_RGB565;
            bpp = 2;
            break;
        case G2D_COLOR_RGB565_SWAP:
            *fire_mode_ctrl_reg |= (G2D_FMODE_CON_SRC_CLR_RGB565 | G2D_FMODE_CON_SRC_SWAP);
            bpp = 2;
            break;
        case G2D_COLOR_ARGB8888 :
            *fire_mode_ctrl_reg |= G2D_FMODE_CON_SRC_CLR_ARGB8888;
            bpp = 4;
            break;
        case G2D_COLOR_ARGB8888_SWAP:
            *fire_mode_ctrl_reg |= (G2D_FMODE_CON_SRC_CLR_ARGB8888 | G2D_FMODE_CON_SRC_SWAP);
            bpp = 4;
            break;
        case G2D_COLOR_RGB888 :
            *fire_mode_ctrl_reg |= G2D_FMODE_CON_SRC_CLR_RGB888;
            bpp = 4;
            break;
        case G2D_COLOR_RGB888_SWAP:
            *fire_mode_ctrl_reg |= (G2D_FMODE_CON_SRC_CLR_RGB888 | G2D_FMODE_CON_SRC_SWAP);
            bpp = 3;
            break;
        case G2D_COLOR_PARGB8888 :
            *fire_mode_ctrl_reg |= G2D_FMODE_CON_SRC_CLR_PARGB8888;
            bpp = 4;
            break;
        case G2D_COLOR_PARGB8888_SWAP :
            *fire_mode_ctrl_reg |= (G2D_FMODE_CON_SRC_CLR_PARGB8888 | G2D_FMODE_CON_SRC_SWAP);
            bpp = 4;
            break;
    }

    REG_G2D_SRC_PITCH = g2d_ctx->gSrcPitch * bpp - 1;
    
}

void g2d_drv_set_dst_info(g2d_context_t *g2d_ctx, unsigned int *fire_mode_ctrl_reg)
{
    unsigned int bpp = 1;
    
    REG_G2D_DST_BASE = (unsigned int)g2d_ctx->gDstAddr;
    REG_G2D_DST_SIZE = ((g2d_ctx->gDstWidth - 1) << 16) | (g2d_ctx->gDstHeight - 1);

    REG_G2D_SMODE_CON &= ~(G2D_SMODE_CON_BIDX_EN | G2D_SMODE_CON_BIDX_MASK);
    *fire_mode_ctrl_reg &= ~(G2D_FMODE_CON_DST_CLR_MODE_MASK | G2D_FMODE_CON_DST_SWAP);

    switch(g2d_ctx->gDstColorFormat)
    {
        case G2D_COLOR_GRAY :
            *fire_mode_ctrl_reg |= G2D_FMODE_CON_DST_CLR_LUT_DIS;
            bpp = 1;
            break;
        case G2D_COLOR_GRAY_SWAP:
            *fire_mode_ctrl_reg |= (G2D_FMODE_CON_DST_CLR_LUT_DIS | G2D_FMODE_CON_DST_SWAP);
            bpp = 1;
            break;
        case G2D_COLOR_RGB565 :
            *fire_mode_ctrl_reg |= G2D_FMODE_CON_DST_CLR_RGB565;
            bpp = 2;
            break;
        case G2D_COLOR_RGB565_SWAP:
            *fire_mode_ctrl_reg |= (G2D_FMODE_CON_DST_CLR_RGB565 | G2D_FMODE_CON_DST_SWAP);
            bpp = 2;
            break;
        case G2D_COLOR_ARGB8888 :
            *fire_mode_ctrl_reg |= G2D_FMODE_CON_DST_CLR_ARGB8888;
            bpp = 4;
            break;
        case G2D_COLOR_ARGB8888_SWAP:
            *fire_mode_ctrl_reg |= (G2D_FMODE_CON_DST_CLR_ARGB8888 | G2D_FMODE_CON_DST_SWAP);
            bpp = 4;
            break;
        case G2D_COLOR_RGB888 :
            *fire_mode_ctrl_reg |= G2D_FMODE_CON_DST_CLR_RGB888;
            bpp = 4;
            break;
        case G2D_COLOR_RGB888_SWAP:
            *fire_mode_ctrl_reg |= (G2D_FMODE_CON_DST_CLR_RGB888 | G2D_FMODE_CON_DST_SWAP);
            bpp = 3;
            break;
        case G2D_COLOR_PARGB8888 :
            *fire_mode_ctrl_reg |= G2D_FMODE_CON_DST_CLR_PARGB8888;
            bpp = 4;
            break;
        case G2D_COLOR_PARGB8888_SWAP :
            *fire_mode_ctrl_reg |= (G2D_FMODE_CON_DST_CLR_PARGB8888 | G2D_FMODE_CON_DST_SWAP);
            bpp = 4;
            break;
    }

    REG_G2D_DST_PITCH = g2d_ctx->gDstPitch * bpp - 1;
    
}

void g2d_drv_set_rotate_mode(g2d_context_t *g2d_ctx)
{
    REG_G2D_SMODE_CON &= ~(G2D_SMODE_CON_SRC_SCAN_MASK | G2D_SMODE_CON_DST_SCAN_MASK);

    switch (g2d_ctx->gRotate)
    {
        case G2D_ROTATE_NONE:
            REG_G2D_SMODE_CON |= 0x44;
            REG_G2D_SRC_XY = 0;
            REG_G2D_DST_XY0 = 0;
            break;

        case G2D_ROTATE_90:
            REG_G2D_SMODE_CON |= 0xD8;
            REG_G2D_SRC_XY = (g2d_ctx->gSrcHeight - 1) & 0xFFFF;
            REG_G2D_DST_XY0 = 0;
            break;
            
        case G2D_ROTATE_180:
            REG_G2D_SMODE_CON |= 0x65;
            REG_G2D_SRC_XY = ((g2d_ctx->gSrcWidth - 1) & 0xFFFF) << 16;
            REG_G2D_DST_XY0 = (g2d_ctx->gDstHeight - 1) & 0xFFFF;
            break;
            
        case G2D_ROTATE_270:
            REG_G2D_SMODE_CON |= 0xC9;
            REG_G2D_SRC_XY = 0;
            REG_G2D_DST_XY0 = (g2d_ctx->gDstHeight - 1) & 0xFFFF;
            break;
    }
}

void g2d_drv_set_alpha_blending(g2d_context_t *g2d_ctx)
{
    REG_G2D_COM_CON |= G2D_COM_CON_ALP_TRP;

    if(g2d_ctx->enAlphaBlending == 1)
    {
       REG_G2D_SMODE_CON |= G2D_SMODE_CON_BB_ALPHA_BLEND; //enable alpha
       REG_G2D_SMODE_CON |=((g2d_ctx->gConstAlpha) & 0xFF) << 16;
    }
}

void g2d_drv_start(g2d_context_t *g2d_ctx)
{
    unsigned int fire_mode_ctrl_reg = 0;

    RESET_G2D_ENGINE;
    
    REG_G2D_SMODE_CON = 0;
    REG_G2D_COM_CON = 0;

    fire_mode_ctrl_reg |= G2D_FMODE_CON_ENG_BITBLT;

    g2d_drv_set_src_info(g2d_ctx, &fire_mode_ctrl_reg);

    g2d_drv_set_dst_info(g2d_ctx, &fire_mode_ctrl_reg);

    g2d_drv_set_rotate_mode(g2d_ctx);

    g2d_drv_set_alpha_blending(g2d_ctx);

    REG_G2D_IRQ_CON |= G2D_IRQ_CON_EN;
    
    REG_G2D_FMODE_CON = fire_mode_ctrl_reg;
}

void g2d_drv_reset(void)
{
    RESET_G2D_ENGINE;

    REG_G2D_IRQ_CON &= ~G2D_IRQ_CON_EN; 
    
    REG_G2D_SMODE_CON = 0;
    REG_G2D_COM_CON = 0;
}

unsigned int g2d_drv_get_dst_addr(void)
{
    return REG_G2D_DST_BASE;
}

int g2d_drv_get_status(void)
{
    return (REG_G2D_COM_STA & G2D_COM_STA_BUSY);
}

int g2d_drv_get_rmmu_flag(void)
{
    return (REG_RMMU_IRQ_FLAG & G2D_RMMU_IRQ_FLAG_MASK);
}
