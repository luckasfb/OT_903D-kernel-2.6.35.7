
#ifndef __MTK_G2D_H__
#define __MTK_G2D_H__

/* ----------------------------- Enumerations ----------------------------- */

typedef enum
{
    M2D_COLOR_INDEX    = 0,
    M2D_COLOR_8BPP     = 0,
    M2D_COLOR_RGB565   = 1,
    M2D_COLOR_ARGB8888 = 2,
    M2D_COLOR_RGB888   = 3,
    M2D_COLOR_ARGB4444 = 5,
    M2D_COLOR_UNKOWN   = 6,

    M2D_COLOR_FORCE_DWORD = 0xFFFFFFFF,
} M2D_COLOR_FORMAT;


typedef enum
{
    M2D_ROTATE_HFLIP_90  = 0,
    M2D_ROTATE_90        = 1,
    M2D_ROTATE_270       = 2,
    M2D_ROTATE_HFLIP_270 = 3,
    M2D_ROTATE_180       = 4,
    M2D_ROTATE_HFLIP_0   = 5,
    M2D_ROTATE_HFLIP_180 = 6,
    M2D_ROTATE_0         = 7,

    M2D_ROTATE_FORCE_DWORD = 0xFFFFFFFF,
} M2D_ROTATE;


/* ---------------------------- Data structures --------------------------- */

typedef struct
{
    int x;
    int y;
    unsigned int width;
    unsigned int height;
} m2d_rect;


typedef struct
{
    unsigned int     virtAddr;      //! surface virtual base address
    unsigned int     pitchInPixels; //! surface pitch in PIXELS
    M2D_COLOR_FORMAT format;        //! surface format
    m2d_rect         rect;          //! cropped rectangle
} m2d_surface;


typedef struct
{
    m2d_surface dstSurf;
    m2d_surface srcSurf;

    M2D_ROTATE rotate;

    /** Enable alpha blending bitblt:
        (1) Per-pixel alpha blending if source with alpha channel
        (2) Constant alpha blending if source w/o alpha channel
    */
    int enAlphaBlending;
    int enPremultipliedAlpha;       //! only used in case (1)
    unsigned char constAlphaValue;  //! only used in case (2)
    
} m2d_bitblt;

/* ----------------------------- IOCTL commands --------------------------- */

#define M2D_IO(num)             _IO(0x2D, num)
#define M2D_IOR(num, dtype)     _IOR(0x2D, num, dtype)
#define M2D_IOW(num, dtype)     _IOW(0x2D, num, dtype)
#define M2D_IOWR(num, dtype)    _IOWR(0x2D, num, dtype)

#define M2D_IOC_BITBLT          M2D_IOW(1, m2d_bitblt)


/* ------------------------------ Driver Name ----------------------------- */

#define M2D_DEV_NAME "MTK_G2D"


#endif // __MTK_G2D_H__
