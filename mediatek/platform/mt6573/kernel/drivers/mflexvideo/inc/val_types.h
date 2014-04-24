

#ifndef _VAL_TYPES_H_
#define _VAL_TYPES_H_
 
#ifdef __cplusplus
 extern "C" {
#endif


typedef void            VAL_VOID_T;      ///< void type definition
typedef char            VAL_BOOL_T;      ///< char type definition
typedef char            VAL_CHAR_T;      ///< char type definition
typedef signed char     VAL_INT8_T;      ///< signed char type definition
typedef signed short    VAL_INT16_T;     ///< signed short type definition
typedef signed int      VAL_INT32_T;     ///< signed int type definition
typedef unsigned char   VAL_UCHAR_T;     ///< unsigned char type definition
typedef unsigned char   VAL_UINT8_T;     ///< unsigned char type definition
typedef unsigned short  VAL_UINT16_T;    ///< unsigned short definition
typedef unsigned int    VAL_UINT32_T;    ///< unsigned int type definition
typedef unsigned long long  VAL_UINT64_T; ///< unsigned long long type definition
typedef long long       VAL_INT64_T;      ///< long long type definition
typedef unsigned int    VAL_HANDLE_T;    ///< handle type definition

#define VAL_NULL        (0) ///< VAL_NULL = 0
#define VAL_TRUE        (1) ///< VAL_TRUE = 1
#define VAL_FALSE       (0) ///< VAL_FALSE = 0

typedef enum _VAL_HW_COMPLETE_T
{
    VAL_POLLING_MODE = 0,                       ///< polling
    VAL_INTERRUPT_MODE,                         ///< interrupt
    VAL_MODE_MAX = 0xFFFFFFFF                   ///< Max result
} VAL_HW_COMPLETE_T;

typedef enum _VAL_RESULT_T
{
    VAL_RESULT_NO_ERROR = 0,                    ///< The function work successfully
    VAL_RESULT_INVALID_DRIVER,                  ///< Error due to invalid driver
    VAL_RESULT_INVALID_PARAMETER,               ///< Error due to invalid parameter
    VAL_RESULT_INVALID_MEMORY,                  ///< Error due to invalid memory
    VAL_RESULT_INVALID_ISR,                     ///< Error due to invalid isr request
    VAL_RESULT_UNKNOWN_ERROR,                   ///< Unknown error    
    
    VAL_RESULT_MAX = 0xFFFFFFFF          ///< Max result
} VAL_RESULT_T;

typedef enum _VAL_DRIVER_TYPE_T
{
    VAL_DRIVER_TYPE_NONE = 0,                        ///< None   
    VAL_DRIVER_TYPE_MP4_ENC,                         ///< MP4 encoder
    VAL_DRIVER_TYPE_MP4_DEC,                         ///< MP4 decoder
    VAL_DRIVER_TYPE_H263_ENC,                        ///< H.263 encoder
    VAL_DRIVER_TYPE_H263_DEC,                        ///< H.263 decoder
    VAL_DRIVER_TYPE_H264_ENC,                        ///< H.264 encoder
    VAL_DRIVER_TYPE_H264_DEC,                        ///< H.264 decoder
    VAL_DRIVER_TYPE_SORENSON_SPARK_DEC,              ///< Sorenson Spark decoder 
    VAL_DRIVER_TYPE_VC1_SP_DEC,                      ///< VC-1 simple profile decoder
    VAL_DRIVER_TYPE_RV9_DEC,                         ///< RV9 decoder
    VAL_DRIVER_TYPE_MP1_MP2_DEC,                     ///< MPEG1/2 decoder
    VAL_DRIVER_TYPE_XVID_DEC,                        ///< Xvid decoder
    VAL_DRIVER_TYPE_DIVX4_DIVX5_DEC,                 ///< Divx4/5 decoder
    VAL_DRIVER_TYPE_VC1_MP_WMV9_DEC,                 ///< VC-1 main profile (WMV9) decoder
    VAL_DRIVER_TYPE_RV8_DEC,                         ///< RV8 decoder      
    VAL_DRIVER_TYPE_WMV7_DEC,                        ///< WMV7 decoder
    VAL_DRIVER_TYPE_WMV8_DEC,                        ///< WMV8 decoder
    VAL_DRIVER_TYPE_AVS_DEC,                         ///< AVS decoder
    VAL_DRIVER_TYPE_DIVX_3_11_DEC,                   ///< Divx3.11 decoder
    VAL_DRIVER_TYPE_H264_DEC_MAIN,                   ///< H.264 main profile decoder (due to different packet) == 20
    VAL_DRIVER_TYPE_H264_DEC_MAIN_CABAC,             ///< H.264 main profile decoder for CABAC type but packet is the same, just for reload.
    VAL_DRIVER_TYPE_MAX = 0xFFFFFFFF     ///< Max driver type 
} VAL_DRIVER_TYPE_T;

typedef enum _VAL_MEM_ALIGN_T
{
    VAL_MEM_ALIGN_1 = 1,                ///< 1 byte alignment
    VAL_MEM_ALIGN_2 = (1<<1),           ///< 2 byte alignment
    VAL_MEM_ALIGN_4 = (1<<2),           ///< 4 byte alignment
    VAL_MEM_ALIGN_8 = (1<<3),           ///< 8 byte alignment
    VAL_MEM_ALIGN_16 = (1<<4),          ///< 16 byte alignment
    VAL_MEM_ALIGN_32 = (1<<5),          ///< 32 byte alignment
    VAL_MEM_ALIGN_64 = (1<<6),          ///< 64 byte alignment
    VAL_MEM_ALIGN_128 = (1<<7),         ///< 128 byte alignment
    VAL_MEM_ALIGN_256 = (1<<8),         ///< 256 byte alignment
    VAL_MEM_ALIGN_512 = (1<<9),         ///< 512 byte alignment
    VAL_MEM_ALIGN_1K = (1<<10),         ///< 1K byte alignment
    VAL_MEM_ALIGN_2K = (1<<11),         ///< 2K byte alignment
    VAL_MEM_ALIGN_4K = (1<<12),         ///< 4K byte alignment
    VAL_MEM_ALIGN_8K = (1<<13),         ///< 8K byte alignment
    VAL_MEM_ALIGN_MAX = 0xFFFFFFFF      ///< Max memory byte alignment
} VAL_MEM_ALIGN_T;

typedef enum _VAL_MEM_TYPE_T
{
    VAL_MEM_TYPE_FOR_SW = 0,        ///< External memory foe SW
    VAL_MEM_TYPE_FOR_HW,            ///< External memory for hw 
    VAL_MEM_TYPE_MAX = 0xFFFFFFFF       ///< Max memory type
} VAL_MEM_TYPE_T;

 
typedef struct _VAL_OBJECT_T
{
    VAL_VOID_T *pvHandle;                ///< [IN/OUT] The video codec driver handle
    VAL_UINT32_T u4HandleSize;           ///< [IN]     The size of video codec driver handle
    VAL_DRIVER_TYPE_T eDriverType;       ///< [IN]     The driver type
    VAL_VOID_T *pvReserved;              ///< [IN/OUT] The reserved parameter
    VAL_UINT32_T u4ReservedSize;         ///< [IN]     The size of reserved parameter structure
} VAL_OBJECT_T;



typedef struct _VAL_PARAM_T
{
    VAL_VOID_T *pvHandle;                ///< [IN]     The video codec driver handle
    VAL_UINT32_T u4HandleSize;           ///< [IN]     The size of video codec driver handle
    VAL_UINT32_T u4CtrlCode;             ///< [IN]     The IO Control Code
    VAL_VOID_T *pvInOutBuffer;           ///< [IN/OUT] The input/output parameter
    VAL_UINT32_T u4InOutBufferSize;      ///< [IN]     The size of input/output parameter structure
    VAL_VOID_T *pvReserved;              ///< [IN/OUT] The reserved parameter
    VAL_UINT32_T u4ReservedSize;         ///< [IN]     The size of reserved parameter structure
} VAL_PARAM_T;


typedef struct _VAL_MEMORY_T
{
    VAL_MEM_TYPE_T eMemType;            ///< [IN]     The allocation memory type
    VAL_UINT32_T u4MemSize;             ///< [IN]     The size of memory allocation
    VAL_VOID_T *pvMemVa;                ///< [IN/OUT] The memory virtual address
    VAL_VOID_T *pvMemPa;                ///< [IN/OUT] The memory physical address
    VAL_MEM_ALIGN_T eAlignment;         ///< [IN]     The memory byte alignment setting
    VAL_VOID_T *pvAlignMemVa;           ///< [IN/OUT] The align memory virtual address
    VAL_VOID_T *pvAlignMemPa;           ///< [IN/OUT] The align memory physical address
    VAL_VOID_T *pvReserved;             ///< [IN/OUT] The reserved parameter
    VAL_UINT32_T u4ReservedSize;        ///< [IN]     The size of reserved parameter structure
} VAL_MEMORY_T;

typedef struct _VAL_INTMEM_T
{
    VAL_VOID_T *pvHandle;                ///< [IN]     The video codec driver handle
    VAL_UINT32_T u4HandleSize;           ///< [IN]     The size of video codec driver handle
    VAL_UINT32_T u4MemSize;              ///< [OUT]    The size of internal memory
    VAL_VOID_T *pvMemVa;                 ///< [OUT]    The internal memory start virtual address
    VAL_VOID_T *pvMemPa;                 ///< [OUT]    The internal memory start physical address
    VAL_VOID_T *pvReserved;              ///< [IN/OUT] The reserved parameter
    VAL_UINT32_T u4ReservedSize;         ///< [IN]     The size of reserved parameter structure
} VAL_INTMEM_T;

typedef struct _VAL_CLOCK_T
{
    VAL_VOID_T *pvHandle;                ///< [IN]     The video codec driver handle
    VAL_UINT32_T u4HandleSize;           ///< [IN]     The size of video codec driver handle
    VAL_DRIVER_TYPE_T eDriverType;       ///< [IN]     The driver type
    VAL_VOID_T *pvReserved;              ///< [IN/OUT] The reserved parameter
    VAL_UINT32_T u4ReservedSize;         ///< [IN]     The size of reserved parameter structure
} VAL_CLOCK_T;

typedef struct _VAL_ISR_T
{
    VAL_VOID_T *pvHandle;                ///< [IN]     The video codec driver handle
    VAL_UINT32_T u4HandleSize;           ///< [IN]     The size of video codec driver handle
    VAL_DRIVER_TYPE_T eDriverType;       ///< [IN]     The driver type
    VAL_VOID_T *pvIsrFunction;           ///< [IN]     The isr function
    VAL_VOID_T *pvReserved;              ///< [IN/OUT] The reserved parameter
    VAL_UINT32_T u4ReservedSize;         ///< [IN]     The size of reserved parameter structure
} VAL_ISR_T;

typedef struct _VAL_EVENT_T
{
    VAL_VOID_T *pvHandle;                ///< [IN]     The video codec driver handle
    VAL_UINT32_T u4HandleSize;           ///< [IN]     The size of video codec driver handle
    VAL_VOID_T *pvWaitQueue;             ///< [IN]     The waitqueue discription
    VAL_VOID_T *pvEvent;                 ///< [IN]     The event discription
    VAL_UINT32_T u4TimeoutMs;            ///< [IN]     The timeout ms
    VAL_VOID_T *pvReserved;              ///< [IN/OUT] The reserved parameter
    VAL_UINT32_T u4ReservedSize;         ///< [IN]     The size of reserved parameter structure
} VAL_EVENT_T;

typedef struct _VAL_MUTEX_T
{
    VAL_VOID_T *pvHandle;                ///< [IN]     The video codec driver handle
    VAL_UINT32_T u4HandleSize;           ///< [IN]     The size of video codec driver handle    
    VAL_VOID_T *pvMutex;                 ///< [IN]     The Mutex discriptor    
    VAL_UINT32_T u4TimeoutMs;            ///< [IN]     The timeout ms
    VAL_VOID_T *pvReserved;              ///< [IN/OUT] The reserved parameter
    VAL_UINT32_T u4ReservedSize;         ///< [IN]     The size of reserved parameter structure
} VAL_MUTEX_T;

typedef struct _VAL_CACHE_T
{
    VAL_VOID_T *pvHandle;                ///< [IN]     The video codec driver handle
    VAL_UINT32_T u4HandleSize;           ///< [IN]     The size of video codec driver handle
    VAL_VOID_T *pvMemVa;                 ///< [IN]     The virtual memory address
    VAL_UINT32_T u4MemSize;              ///< [IN]     The memory size
    VAL_VOID_T *pvReserved;              ///< [IN/OUT] The reserved parameter
    VAL_UINT32_T u4ReservedSize;         ///< [IN]     The size of reserved parameter structure
} VAL_CACHE_T;

typedef struct _VAL_MEM_ADDR_T
{
    VAL_UINT32_T    u4VA;      ///< [IN/OUT] virtual address   
    VAL_UINT32_T    u4PA;       ///< [IN/OUT] physical address   
    VAL_UINT32_T    u4Size;     ///< [IN/OUT] size   
} VAL_MEM_ADDR_T;

typedef struct _VAL_TIME_T
{
    VAL_UINT32_T    u4Sec;        ///< [IN/OUT] second   
    VAL_UINT32_T    u4uSec;       ///< [IN/OUT] micro second   
} VAL_TIME_T;

 /**
 * @par Structure
 *  VAL_STRSTR_T
 * @par Description
 *  This is a parameter for eVideoStrStr()
 */
typedef struct _VAL_STRSTR_T
{
    VAL_VOID_T *pvStr;				///< [IN]     Null-terminated string to search.
	VAL_VOID_T *pvStrSearch;		///< [IN]     Null-terminated string to search for
	VAL_VOID_T *pvStrResult;		///< [Out]    Returns a pointer to the first occurrence of strSearch in str, or NULL if strSearch does not appear in str.
	VAL_VOID_T *pvReserved;         ///< [IN/OUT] The reserved parameter
	VAL_UINT32_T u4ReservedSize;    ///< [IN]     The size of reserved parameter structure
} VAL_STRSTR_T;

typedef struct _VAL_ATOI_T
{
    VAL_VOID_T *pvStr;				///< [IN]     Null-terminated String to be converted
	VAL_INT32_T i4Result;			///< [Out]    returns the int value produced by interpreting the input characters as a number. 
	VAL_VOID_T *pvReserved;         ///< [IN/OUT] The reserved parameter
	VAL_UINT32_T u4ReservedSize;    ///< [IN]     The size of reserved parameter structure
} VAL_ATOI_T;

#ifdef __cplusplus
}
#endif

#endif // #ifndef _VAL_TYPES_H_
