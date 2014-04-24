

#ifndef __DISP_ASSERT_LAYER_H__
#define __DISP_ASSERT_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{	
   DAL_STATUS_OK                = 0,

   DAL_STATUS_NOT_READY         = -1,
   DAL_STATUS_INVALID_ARGUMENT  = -2,
   DAL_STATUS_LOCK_FAIL         = -3,
   DAL_STATUS_FATAL_ERROR       = -4,
} DAL_STATUS;


/* Display Assertion Layer API */

unsigned int DAL_GetLayerSize(void);

DAL_STATUS DAL_Init(unsigned int layerVA, unsigned int layerPA);
DAL_STATUS DAL_Clean(void);
DAL_STATUS DAL_Printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // __DISP_ASSERT_LAYER_H__
