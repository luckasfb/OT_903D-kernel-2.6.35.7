
#ifndef __G2D_DRV_H__
#define __G2D_DRV_H__

typedef enum 
{
    G2D_ROTATE_NONE = 0,
    G2D_ROTATE_90,
    G2D_ROTATE_180,
    G2D_ROTATE_270
    
} g2d_rotate_enum;

typedef enum 
{
    G2D_COLOR_GRAY = 0,
    G2D_COLOR_GRAY_SWAP = 1,
    G2D_COLOR_RGB565 = 2,
    G2D_COLOR_RGB565_SWAP = 3,
    G2D_COLOR_ARGB8888 = 4,
    G2D_COLOR_ARGB8888_SWAP = 5,
    G2D_COLOR_RGB888 = 6,
    G2D_COLOR_RGB888_SWAP = 7,
    G2D_COLOR_PARGB8888 = 12,
    G2D_COLOR_PARGB8888_SWAP = 13

} g2d_color_enum;

typedef struct
{
    // src info
    void * gSrcAddr;
    unsigned int gSrcWidth;
    unsigned int gSrcHeight;
    unsigned int gSrcPitch;
    g2d_color_enum gSrcColorFormat;

    // dst info
    void * gDstAddr;
    unsigned int gDstWidth;
    unsigned int gDstHeight;
    unsigned int gDstPitch;
    g2d_color_enum gDstColorFormat;
    
    g2d_rotate_enum gRotate;

    /** Enable alpha blending bitblt:
        (1) Per-pixel alpha blending if source with alpha channel
        (2) Constant alpha blending if source w/o alpha channel
    */
    int enAlphaBlending;
    unsigned char gConstAlpha;  //! only used in case (2)
    
} g2d_context_t;


#define G2D_IOCTL_MAGIC     'G'

#define G2D_IOCTL_INIT      _IO (G2D_IOCTL_MAGIC, 1)
#define G2D_IOCTL_BITBLT    _IOW(G2D_IOCTL_MAGIC, 2, g2d_context_t)
//#define G2D_IOCTL_START     _IO (G2D_IOCTL_MAGIC, 3)
#define G2D_IOCTL_WAIT      _IOW(G2D_IOCTL_MAGIC, 4, int) 
#define G2D_IOCTL_DEINIT    _IO (G2D_IOCTL_MAGIC, 5)

#endif

