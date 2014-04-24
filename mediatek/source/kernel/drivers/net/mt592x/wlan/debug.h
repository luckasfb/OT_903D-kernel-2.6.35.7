





#ifndef _DEBUG_H
#define _DEBUG_H


#include "gl_typedef.h"

extern UINT_8   aucDebugModule[];
extern UINT_32  u4DebugModule;

#define DBG_CLASS_ERROR         BIT(0)
#define DBG_CLASS_WARN          BIT(1)
#define DBG_CLASS_STATE         BIT(2)
#define DBG_CLASS_EVENT         BIT(3)
#define DBG_CLASS_TRACE         BIT(4)
#define DBG_CLASS_INFO          BIT(5)
#define DBG_CLASS_LOUD          BIT(6)
#define DBG_CLASS_TEMP          BIT(7)
#define DBG_CLASS_MASK          BITS(0,7)


#if defined(LINUX)
#define DBG_PRINTF_64BIT_DEC    "lld"

#else  //Windows
#define DBG_PRINTF_64BIT_DEC    "I64d"

#endif
/* Define debug module index */
typedef enum _ENUM_DBG_MODULE_T {
    DBG_INIT_IDX = 0,       /* For driver initial */
    DBG_ARB_IDX,            /* For Arbiter State Machine */
    DBG_TEST_IDX,           /* For ARB_STATE_RF_TEST */
    DBG_SCAN_IDX,           /* For ARB_STATE_SCAN */
    DBG_JOIN_IDX,           /* For ARB_STATE_JOIN */
    DBG_ROAMING_IDX,        /* For Roaming Module */
    DBG_NIC_IDX,            /* For NIC Layer */
    DBG_PHY_IDX,            /* For BB or RF */
    DBG_HAL_IDX,            /* For HAL(HW) Layer */
    DBG_INTR_IDX,           /* For Interrupt */
    DBG_TX_IDX,
    DBG_RX_IDX,
    DBG_REQ_IDX,
    DBG_MGT_IDX,
    DBG_RSN_IDX,
    DBG_LP_IDX,
    DBG_RFTEST_IDX,         /* For RF test mode*/
    DBG_LP_IOT_IDX,         /* For LP-IOT mode*/
    DBG_RCPI_MEASURE_IDX,   /* For debugging RCPI measurement */
    DBG_DOMAIN_IDX,         /* For debugging DOMAIN */
//#if SUPPORT_WAPI
    DBG_WAPI_IDX,           /* For debugging WAPI */
//#endif
    DBG_SW1_IDX,            /* Developer specific */
    DBG_SW2_IDX,            /* Developer specific */
    DBG_SW3_IDX,            /* Developer specific */
    DBG_SW4_IDX,            /* Developer specific */
    DBG_SW5_IDX,            /* Developer specific */
    DBG_SW6_IDX,            /* Developer specific */


    DBG_MODULE_NUM
} ENUM_DBG_MODULE_T;

/* Define who owns developer specific index */
#define DBG_WH_IDX              DBG_SW1_IDX
#define DBG_KEVIN_IDX           DBG_SW2_IDX
#define DBG_MIKE_IDX            DBG_SW3_IDX
#define DBG_KENT_IDX            DBG_SW4_IDX
#define DBG_GEORGE_IDX          DBG_SW5_IDX
#define DBG_CMC_IDX             DBG_SW6_IDX



/* Debug print format string for the OS system time */
#define OS_SYSTIME_DBG_FORMAT               "0x%08x"

/* Debug print argument for the OS system time */
#define OS_SYSTIME_DBG_ARGUMENT(systime)    (systime)

/* Debug print format string for the MAC Address */
#define MACSTR          "%02x:%02x:%02x:%02x:%02x:%02x"

/* Debug print argument for the MAC Address */
#define MAC2STR(a)      ((PUINT_8)a)[0], ((PUINT_8)a)[1], ((PUINT_8)a)[2], \
                        ((PUINT_8)a)[3], ((PUINT_8)a)[4], ((PUINT_8)a)[5]

/* The pre-defined format to dump the value of a varaible with its name shown. */
#define DUMPVAR(variable, format)           (#variable " = " format "\n", variable)

/* The pre-defined format to dump the MAC type value with its name shown. */
#define DUMPMACADDR(addr)                   (#addr " = " MACSTR "\n", MAC2STR(addr))


#define LOG_FUNC_TIME           kalPrint
#define LOG_FUNC                kalPrint

#if DBG

    /* If __FUNCTION__ is already defined by compiler, we just use it. */
    #if defined(__FUNCTION__)
        #define DEBUGFUNC(_Func)
    #else
        #define DEBUGFUNC(_Func) static const char __FUNCTION__[] = _Func;
    #endif

    /* The following macros are used for future debug message. */
    /* TODO(Kevin): We should remove INITLOG/ERRORLOG/WARNLOG macro sooner or later */
    #define INITLOG(_Fmt) \
    { \
        if (aucDebugModule[DBG_INIT_IDX] & DBG_CLASS_TRACE) { \
            LOG_FUNC("%s: ", __FUNCTION__); \
            LOG_FUNC _Fmt; \
        } \
    }

    #define ERRORLOG(_Fmt) \
    { \
        if (aucDebugModule[DBG_ARB_IDX] & DBG_CLASS_ERROR) { \
            LOG_FUNC("**Error[%s:%d]-", __FILE__, __LINE__); \
            LOG_FUNC _Fmt; \
        } \
    }

    #define WARNLOG(_Fmt) \
    { \
        if (aucDebugModule[DBG_ARB_IDX] & DBG_CLASS_WARN) { \
            LOG_FUNC("**Warning[%s:%d]-", __FILE__, __LINE__); \
            LOG_FUNC _Fmt; \
        } \
    }

    /*lint -save -e960 Multiple use of '#/##' */
    #define DBGLOG(_Module, _Class, _Fmt) \
        { \
            if (aucDebugModule[DBG_##_Module##_IDX] & DBG_CLASS_##_Class) { \
                if (DBG_CLASS_##_Class == DBG_CLASS_ERROR) { \
                    LOG_FUNC_TIME("**Error[%s:%d]-", __FILE__, __LINE__); \
                    LOG_FUNC("%s: (" #_Module " " #_Class ") ", __FUNCTION__); \
                } \
                else if (DBG_CLASS_##_Class == DBG_CLASS_WARN) { \
                    LOG_FUNC_TIME("**Warning[%s:%d]-", __FILE__, __LINE__); \
                    LOG_FUNC("%s: (" #_Module " " #_Class ") ", __FUNCTION__); \
                } \
                else if (DBG_CLASS_##_Class == DBG_CLASS_EVENT) { \
                } \
                else { \
                    LOG_FUNC_TIME("%s: (" #_Module " " #_Class ") ", __FUNCTION__); \
                } \
                LOG_FUNC _Fmt; \
            } \
        }

    #define DBGLOG_MEM8(_Module, _Class, _StartAddr, _Length) \
        { \
            if (aucDebugModule[DBG_##_Module##_IDX] & DBG_CLASS_##_Class) { \
                LOG_FUNC("%s: (" #_Module " " #_Class ")\n", __FUNCTION__); \
                dumpMemory8((PUINT_8) (_StartAddr), (UINT_32) (_Length)); \
            } \
        }

    #define DBGLOG_MEM32(_Module, _Class, _StartAddr, _Length) \
        { \
            if (aucDebugModule[DBG_##_Module##_IDX] & DBG_CLASS_##_Class) { \
                LOG_FUNC("%s: (" #_Module " " #_Class ")\n", __FUNCTION__); \
                dumpMemory32((PUINT_32) (_StartAddr), (UINT_32) (_Length)); \
            } \
        }
    /*lint -restore */

    /*lint -save -e961 use of '#undef' is discouraged */
    #undef ASSERT
    /*lint -restore */

    #ifdef _lint
    #define ASSERT(_exp) \
        { \
            if (!(_exp)) {do {} while (1);} \
        }
    #else
    #define ASSERT(_exp) \
        { \
            if (!(_exp) && !fgIsBusAccessFailed) { \
                LOG_FUNC("Assertion failed: %s:%d %s\n", __FILE__, __LINE__, #_exp); \
                kalBreakPoint(); \
            } \
        }
    #endif /* _lint */

    #define ASSERT_REPORT(_exp, _fmt) \
        { \
            if (!(_exp) && !fgIsBusAccessFailed) { \
                LOG_FUNC("Assertion failed: %s:%d %s\n", __FILE__, __LINE__, #_exp); \
                LOG_FUNC _fmt; \
                kalBreakPoint(); \
            } \
        }

    #define DISP_STRING(_str)       _str

#else /* !DBG */

    #define DEBUGFUNC(_Func)
    #define INITLOG(_Fmt)
    #define ERRORLOG(_Fmt)
    #define WARNLOG(_Fmt)
    #define DBGLOG(_Module, _Class, _Fmt)
    #define DBGLOG_MEM8(_Module, _Class, _StartAddr, _Length)
    #define DBGLOG_MEM32(_Module, _Class, _StartAddr, _Length)

    #undef ASSERT

#if BUILD_QA_DBG
    #if defined(LINUX) /* For debugging in Linux w/o GDB */
        #define ASSERT(_exp) \
            { \
                if (!(_exp) && !fgIsBusAccessFailed) { \
                    LOG_FUNC("Assertion failed: %s:%d (%s)\n", __FILE__, __LINE__, #_exp); \
                    kalBreakPoint(); \
                } \
            }

        #define ASSERT_REPORT(_exp, _fmt) \
            { \
                if (!(_exp) && !fgIsBusAccessFailed) { \
                    LOG_FUNC("Assertion failed: %s:%d (%s)\n", __FILE__, __LINE__, #_exp); \
                    LOG_FUNC _fmt; \
                    kalBreakPoint(); \
                } \
            }
    #else
        #ifdef WINDOWS_CE
            #define UNICODE_TEXT(_msg)  TEXT(_msg)
            #define ASSERT(_exp) \
                { \
                    if (!(_exp) && !fgIsBusAccessFailed) { \
                        TCHAR rUbuf[256]; \
                        kalBreakPoint(); \
                        _stprintf(rUbuf, TEXT("Assertion failed: %s:%d %s\n"), \
                            UNICODE_TEXT(__FILE__), \
                            __LINE__, \
                            UNICODE_TEXT(#_exp)); \
                        MessageBox(NULL, rUbuf, TEXT("ASSERT!"), MB_OK); \
                    } \
                }

            #define ASSERT_REPORT(_exp, _fmt) \
                { \
                    if (!(_exp) && !fgIsBusAccessFailed) { \
                        TCHAR rUbuf[256]; \
                        kalBreakPoint(); \
                        _stprintf(rUbuf, TEXT("Assertion failed: %s:%d %s\n"), \
                            UNICODE_TEXT(__FILE__), \
                            __LINE__, \
                            UNICODE_TEXT(#_exp)); \
                        MessageBox(NULL, rUbuf, TEXT("ASSERT!"), MB_OK); \
                    } \
                }
        #else
            #define ASSERT(_exp) \
                { \
                    if (!(_exp) && !fgIsBusAccessFailed) { \
                        kalBreakPoint(); \
                    } \
                }

            #define ASSERT_REPORT(_exp, _fmt) \
                { \
                    if (!(_exp) && !fgIsBusAccessFailed) { \
                        kalBreakPoint(); \
                    } \
                }
        #endif /* WINDOWS_CE */
    #endif /* LINUX */
#else
    #define ASSERT(_exp)
    #define ASSERT_REPORT(_exp, _fmt)
#endif /* BUILD_QA_DBG */

    #define DISP_STRING(_str)       ""

#endif /* DBG */


/* The following macro is used for debugging packed structures. */
#define DATA_STRUC_INSPECTING_ASSERT(expr)      switch (0) {case 0: case (expr): default:;}


#if DBG
VOID
dumpMemory8 (
    IN PUINT_8 pucStartAddr,
    IN UINT_32 u4Length
    );

VOID
dumpMemory32 (
    IN PUINT_32 pu4StartAddr,
    IN UINT_32  u4Length
    );
#endif /* DBG */

#endif /* _DEBUG_H */

