
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <linux/proc_fs.h>  //proc file use
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/sched.h>

//Arch dependent files
#include <asm/tcm.h>
#include <mach/mt6573_sysram.h>
    typedef MT6573_SYSRAM_USR       ESysramUser_T;
#include <mach/mt6573_sysram_layout.h>

//The follow code are only for MT6573.
//This is compatible with other other chip.
#define ESysramUser_CNT     MT6573SYSRAMUSR_CNT
#define ESysramUser_None    MT6573SYSRAMUSR_NONE
//--------------------------------------------Define-----------------------------------------------//
#if     1
 #define MY_ERR printk
#else
 #define MY_ERR(x,...)
#endif

#if     1
 #define MY_DBG printk
#else
 #define MY_DBG(x,...)
#endif

#if     1
 #define MY_LOG printk
#else
 #define MY_LOG(x,...)
#endif

/* Debug message*/
#define DBG_SYSRAM_NONE                 0x00000000
#define DBG_SYSRAM_OVER_PREDEFINE_SIZE  0x00000001
#define DBG_SYSRAM_ALL                  0xFFFFFFFF
unsigned int g_u4SysramDbgFlag = DBG_SYSRAM_ALL;
unsigned int g_u4EnableUserGroupSize = 1;
//MT6573: 0, we just show warning messages instead of returning failure.
//MT6575: 1, we do not allow anyone use un-predicted size.
unsigned int g_u4EnableOverSizeFail = 0;

#define DBG_SYSRAM_MASK     (g_u4SysramDbgFlag)



//-------------------------------------Global variables-------------------------
typedef struct SysramLog {
    unsigned long u4Tbl;
    unsigned long u4Occupied;
} SysramLog_T;

static SysramLog_T g_pstSysramLog[ESysramUser_CNT];

//--------------------------------------Functions-------------------------------
static void MTxxx_SYSRAM_DUMPLAYOUT(void);

static void Dump_ResMgr(void);
static inline int IsBadOwner(ESysramUser_T const eOwner);
static void SetOwnerTaskInfo(ESysramUser_T const eOwner);
static void ResetOwnerTaskInfo(ESysramUser_T const eOwner);
static inline void Lock_ResMgr(void);
static inline void Unlock_ResMgr(void);
static inline int IsLockedOwner_ResMgr(ESysramUser_T const eOwner);
static inline int IsUnlockedOwner_ResMgr(ESysramUser_T const eOwner);
static void LockOwner_ResMgr(ESysramUser_T const eOwner, unsigned long const u4Size);
static void UnlockOwner_ResMgr(ESysramUser_T const eOwner);

typedef struct TaskInfo
{
    pid_t   pid;                    // thread id
    pid_t   tgid;                   // process id
    char    comm[TASK_COMM_LEN];    // executable name
    //  Time stamp when alloc.
    unsigned long       t_jiffies;  // jiffies
    unsigned long       t_sec;      // nanosec_rem = do_div((t_jiffies-INITIAL_JIFFIES) * (USEC_PER_SEC / HZ), USEC_PER_SEC);
    unsigned long       t_usec;     // nanosec_rem / 1000)
} TaskInfo_T;

typedef struct
{
    spinlock_t          lock;
    unsigned long       u4TotalUserCount;
    unsigned long       u4AllocatedTbl;
    unsigned long       au4AllocatedSize[ESysramUser_CNT];
    TaskInfo_T          aOwnerTaskInfo[ESysramUser_CNT];
    P_MaxUsrSizeTbl_T   pMaxUsrSizeTbl;
    wait_queue_head_t   wq_head;
} ResMgr_T;

static ResMgr_T g_ResMgr =
{
    .u4TotalUserCount   =   0, 
    .u4AllocatedTbl     =   0, 
    .au4AllocatedSize   =   {0}, 
    .pMaxUsrSizeTbl     =   &g_au4MaxUserSize, 
};

//------------------------------------------------------------------------------
static void Dump_ResMgr(void)
{
    unsigned int u4Idx = 0;
    MY_LOG("[Dump_ResMgr]\n");
    MY_LOG(
        "(u4TotalUserCount, u4AllocatedTbl)=(%ld, 0x%lX)\n"
        , g_ResMgr.u4TotalUserCount, g_ResMgr.u4AllocatedTbl
    );
    for (u4Idx = 0; u4Idx < ESysramUser_CNT; u4Idx++)
    {
        if  ( 0 < g_ResMgr.au4AllocatedSize[u4Idx] )
        {
            TaskInfo_T*const pTaskInfo = &g_ResMgr.aOwnerTaskInfo[u4Idx];
            MY_LOG(
                "[id:%u][%s][size:0x%lX][pid:%d][tgid:%d][%s][%5lu.%06lu][%08lX]\n"
                , u4Idx, g_apszOwnerName[u4Idx], g_ResMgr.au4AllocatedSize[u4Idx]
                , pTaskInfo->pid, pTaskInfo->tgid, pTaskInfo->comm
                , pTaskInfo->t_sec, pTaskInfo->t_usec, pTaskInfo->t_jiffies
            );
        }
    }
    MY_LOG("\n");
}

static inline int IsBadOwner(ESysramUser_T const eOwner)
{
    return  ( ESysramUser_CNT <= eOwner || eOwner < 0 );
}

static void SetOwnerTaskInfo(ESysramUser_T const eOwner)
{
    if  ( ! IsBadOwner(eOwner) )
    {
        TaskInfo_T*const pInfo = &g_ResMgr.aOwnerTaskInfo[eOwner];
        pInfo->pid = current->pid;
        pInfo->tgid = current->tgid;
        memcpy(pInfo->comm, current->comm, sizeof(pInfo->comm));

        pInfo->t_jiffies= jiffies;
        pInfo->t_sec    = (pInfo->t_jiffies - INITIAL_JIFFIES) * (USEC_PER_SEC / HZ);
        pInfo->t_usec   = do_div(pInfo->t_sec , USEC_PER_SEC) / 1000;   //  pInfo->t_sec is changed after do_div().
    }
}

static void ResetOwnerTaskInfo(ESysramUser_T const eOwner)
{
    if  ( ! IsBadOwner(eOwner) )
    {
        TaskInfo_T*const pInfo = &g_ResMgr.aOwnerTaskInfo[eOwner];
        memset(pInfo, 0, sizeof(*pInfo));
    }
}

static inline void Lock_ResMgr(void)
{
    spin_lock(&g_ResMgr.lock);
}

static inline void Unlock_ResMgr(void)
{
    spin_unlock(&g_ResMgr.lock);
}

static inline int IsLockedOwner_ResMgr(ESysramUser_T const eOwner)
{
    return  ((1 << eOwner) & g_ResMgr.u4AllocatedTbl);
}

static inline int IsUnlockedOwner_ResMgr(ESysramUser_T const eOwner)
{
    return  0 == IsLockedOwner_ResMgr(eOwner);
}

static void LockOwner_ResMgr(ESysramUser_T const eOwner, unsigned long const u4Size)
{
    if  ( IsLockedOwner_ResMgr(eOwner) )
        return;

    g_ResMgr.u4TotalUserCount++;
    g_ResMgr.u4AllocatedTbl |= (1 << eOwner);
    g_ResMgr.au4AllocatedSize[eOwner] = u4Size;
    SetOwnerTaskInfo(eOwner);

    //  Debug Log.
    if  ( (1<<eOwner) & ELogOwnersMask )
    {
        TaskInfo_T*const pTaskInfo = &g_ResMgr.aOwnerTaskInfo[eOwner];
        MY_DBG(
            "[SYSRAM][LockOwner][%s][%lu bytes] succeed [Lock Time:%5lu.%06lu][%08lX]\n"
            , g_apszOwnerName[eOwner], g_ResMgr.au4AllocatedSize[eOwner]
            , pTaskInfo->t_sec, pTaskInfo->t_usec, pTaskInfo->t_jiffies
        );
    }
}

static void UnlockOwner_ResMgr(ESysramUser_T const eOwner)
{
    if  ( IsUnlockedOwner_ResMgr(eOwner) )
        return;

    //  Debug Log.
    if  ( (1<<eOwner) & ELogOwnersMask )
    {
        TaskInfo_T*const pTaskInfo = &g_ResMgr.aOwnerTaskInfo[eOwner];
        MY_DBG(
            "[SYSRAM][UnlockOwner][%s][%lu bytes]...[Lock Time:%5lu.%06lu][Lock Period:%lu usec]\n"
            , g_apszOwnerName[eOwner], g_ResMgr.au4AllocatedSize[eOwner]
            , pTaskInfo->t_sec, pTaskInfo->t_usec, ((jiffies - pTaskInfo->t_jiffies)*(USEC_PER_SEC/HZ))
        );
    }

    if  ( g_ResMgr.u4TotalUserCount > 0 )
        g_ResMgr.u4TotalUserCount--;
    g_ResMgr.u4AllocatedTbl &= (~(1 << eOwner));
    g_ResMgr.au4AllocatedSize[eOwner] = 0;
    ResetOwnerTaskInfo(eOwner);
}


static MemNode_T* AllocNode(MemPoolInfo_T*const pMemPoolInfo)
{
    MemNode_T* pNode = NULL;
    unsigned long u4Index = 0;
    for(u4Index = 0; u4Index < pMemPoolInfo->u4OwnerCnt; u4Index+=1)
    {
        if  ((pMemPoolInfo->u4IndexTbl) & (1 << u4Index))
        {
            pMemPoolInfo->u4IndexTbl &= (~(1 << u4Index));
            //  A free node is found.
            pNode = &pMemPoolInfo->paNodes[u4Index];
            pNode->eOwner   = ESysramUser_None;
            pNode->u4Offset = 0;
            pNode->u4Length = 0;
            pNode->pPrev    = NULL;
            pNode->pNext    = NULL;
            pNode->u4Index  = u4Index;
            break;
        }
    }
    //  Shouldn't happen.
    if  ( ! pNode )
    {
        MY_ERR("[SYSRAM][AllocNode] returns NULL - pMemPoolInfo->u4IndexTbl(%lX)\n", pMemPoolInfo->u4IndexTbl);
    }
    return  pNode;
}

static void FreeNode(MemPoolInfo_T*const pMemPoolInfo, MemNode_T*const pNode)
{
    pMemPoolInfo->u4IndexTbl |= (1<<pNode->u4Index);
    pNode->eOwner   = ESysramUser_None;
    pNode->u4Offset = 0;
    pNode->u4Length = 0;
    pNode->pPrev    = NULL;
    pNode->pNext    = NULL;
    pNode->u4Index  = 0;
}

static int IsLegalSizeToAlloc(MEM_POOL_BANK_NO_T const eMemPoolBankNo, ESysramUser_T const eOwner, unsigned long const u4Size)
{
    unsigned long u4MaxSize = 0;

    //  (1) Check the memory pool.
    switch  (eMemPoolBankNo)
    {
    case MEM_POOL_BAD:
    case MEM_POOL_CNT:
        //  Illegal Memory Pool: return "illegal"
        //  Shouldn't happen.
        goto lbExit;
    case MEM_POOL_STATIC:   //  Static
        //  Static Memory Pool: compare with the max. user size.
        u4MaxSize = g_au4MaxUserSize[eOwner];
        goto lbExit;
    default:                //  Dynamic
        break;
    }

    //  (2)
    //  Here we use the dynamic memory pools.
    u4MaxSize = (*g_ResMgr.pMaxUsrSizeTbl)[eOwner];

    //  (3) User Group.
    if  ( 0 != g_u4EnableUserGroupSize )
    {
        int bIsInGroup = 1;
        unsigned long u4GroupSize = 0;
        unsigned long u4MaxGroupSize = 0;
        switch (eOwner)
        {
        //  (3.1) User Group: ROTDMA0~3
        case MT6573SYSRAMUSR_ROTDMA0:
        case MT6573SYSRAMUSR_ROTDMA1:
        case MT6573SYSRAMUSR_ROTDMA2:
        case MT6573SYSRAMUSR_ROTDMA3:
            u4GroupSize = g_ResMgr.au4AllocatedSize[MT6573SYSRAMUSR_ROTDMA0]+g_ResMgr.au4AllocatedSize[MT6573SYSRAMUSR_ROTDMA1]+g_ResMgr.au4AllocatedSize[MT6573SYSRAMUSR_ROTDMA2]+g_ResMgr.au4AllocatedSize[MT6573SYSRAMUSR_ROTDMA3];
            u4MaxGroupSize = g_au4MaxUserSize[MT6573SYSRAMUSR_ROTDMA0]+g_au4MaxUserSize[MT6573SYSRAMUSR_ROTDMA1]+g_au4MaxUserSize[MT6573SYSRAMUSR_ROTDMA2]+g_au4MaxUserSize[MT6573SYSRAMUSR_ROTDMA3];
            break;
        //  (3.2) User Group: RDMA0~1
        case MT6573SYSRAMUSR_RDMA0:
        case MT6573SYSRAMUSR_RDMA1:
            u4GroupSize = g_ResMgr.au4AllocatedSize[MT6573SYSRAMUSR_RDMA0]+g_ResMgr.au4AllocatedSize[MT6573SYSRAMUSR_RDMA1];
            u4MaxGroupSize = g_au4MaxUserSize[MT6573SYSRAMUSR_RDMA0]+g_au4MaxUserSize[MT6573SYSRAMUSR_RDMA1];
            break;
        default:
            bIsInGroup = 0; //  Not in group.
            break;
        }

        if  ( bIsInGroup != 0 )  //  In group ?
        {   
            if  ( u4Size + u4GroupSize > u4MaxGroupSize )
            {
                MY_LOG(
                    "[SYSRAM][Owner:%d in group] size to alloc(%ld) + group size(%ld) > max group size(%ld)\n\r"
                    , eOwner, u4Size, u4GroupSize, u4MaxGroupSize
                );
                return  0;
            }
            u4MaxSize = (u4MaxGroupSize - u4GroupSize);
        }
    }

lbExit:
    if  (u4MaxSize < u4Size)
    {
        if  ( DBG_SYSRAM_OVER_PREDEFINE_SIZE & DBG_SYSRAM_MASK )
        {
            MY_LOG(
                "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
                "[SYSRAM][ALLOC][owner: %s] requested size(0x%lX) > max size(0x%lX)\n"
                " Before Allocating...\n", 
                g_apszOwnerName[eOwner], u4Size, u4MaxSize
            );
            MTxxx_SYSRAM_DUMPLAYOUT();
        }
        return   0; 
    }
    return  1;
}


static unsigned long Allocate(
    ESysramUser_T const eOwner, 
    unsigned long const u4Size, 
    unsigned long const u4Alignment, 
    MEM_POOL_BANK_NO_T const eMemPoolBankNo
)
{
    MemNode_T* pSplitNode = NULL;
    MemNode_T* pCurrNode = NULL;
    unsigned long u4AlingnedAddr = 0;
    unsigned long u4ActualSize = 0;

    MemPoolInfo_T*const pMemPoolInfo = GetMemPoolInfo(eMemPoolBankNo);
    if  ( ! pMemPoolInfo )
    {
        return  0;
    }

    pCurrNode = &pMemPoolInfo->paNodes[0];
    for (; pCurrNode && pCurrNode->u4Offset < pMemPoolInfo->u4Size; pCurrNode = pCurrNode->pNext)
    {
        if  (ESysramUser_None == pCurrNode->eOwner)
        {
            //Free space
            u4AlingnedAddr = (pCurrNode->u4Offset + u4Alignment - 1)&(~(u4Alignment - 1));
            u4ActualSize = u4Size + u4AlingnedAddr - pCurrNode->u4Offset;
            if  (u4ActualSize <= pCurrNode->u4Length)
            {
                // Hit!! Split into 2
                // pSplitNode pointers to the next available (free) node.
                pSplitNode = AllocNode(pMemPoolInfo);
                pSplitNode->u4Offset = pCurrNode->u4Offset + u4ActualSize;
                pSplitNode->u4Length = pCurrNode->u4Length - u4ActualSize;
                pSplitNode->pPrev    = pCurrNode;
                pSplitNode->pNext    = pCurrNode->pNext;
                //
                pCurrNode->eOwner = eOwner;
                pCurrNode->u4Length = u4ActualSize;
                pCurrNode->pNext = pSplitNode;
                //
                if  (NULL != pSplitNode->pNext)
                {
                    pSplitNode->pNext->pPrev = pSplitNode;
                }
                //
                pMemPoolInfo->u4UserCount++;
                break;
            }
            //Not hit
            u4ActualSize = 0;
        }
    };

    return u4ActualSize ? (u4AlingnedAddr + pMemPoolInfo->u4StartAddr) : 0;
}

static int Free(ESysramUser_T const eOwner, MEM_POOL_BANK_NO_T const eMemPoolBankNo)
{
    int bRet = 0;
    MemNode_T* pPrevOrNextNode = NULL;
    MemNode_T* pCurrNode = NULL;
    MemNode_T* pTempNode = NULL;

    MemPoolInfo_T*const pMemPoolInfo = GetMemPoolInfo(eMemPoolBankNo);
    if  ( ! pMemPoolInfo )
    {
        MY_ERR("[ERR][SYSRAM][Free] pMemPoolInfo==NULL (eOwner, eMemPoolBankNo)=(%d, %d)\n", eOwner, eMemPoolBankNo);
        return  0;
    }

    pCurrNode = &pMemPoolInfo->paNodes[0];
    for (; pCurrNode; pCurrNode = pCurrNode->pNext)
    {
        if  (eOwner == pCurrNode->eOwner)
        {
            bRet = 1;   //  owner is found.
            if  ( pMemPoolInfo->u4UserCount > 0 )
                pMemPoolInfo->u4UserCount--;

            pCurrNode->eOwner = ESysramUser_None;
            if  (NULL != pCurrNode->pPrev)
            {
                pPrevOrNextNode = pCurrNode->pPrev;
#if 0
                if  ( pPrevOrNextNode->pNext != pCurrNode )
                {
                    MY_ERR("[ERR][Free] pPrevOrNextNode->pNext != pCurrNode (pNext, pNext->u4Index, pCurrNode, pCurrNode->u4Index)=(%p, %ld, %p, %ld)\n", pPrevOrNextNode->pNext, pPrevOrNextNode->pNext->u4Index, pCurrNode, pCurrNode->u4Index);
                }
#endif
                if  (ESysramUser_None == pPrevOrNextNode->eOwner)
                {
                    //Merge previous: prev(o) + curr(x)
                    pTempNode = pCurrNode;
                    pCurrNode = pPrevOrNextNode;
                    pCurrNode->u4Length += pTempNode->u4Length;
                    pCurrNode->pNext = pTempNode->pNext;
                    if  (NULL != pTempNode->pNext)
                    {
                        pTempNode->pNext->pPrev = pCurrNode;
                    }
                    FreeNode(pMemPoolInfo, pTempNode);
                }
            }

            if  (NULL != pCurrNode->pNext)
            {
                pPrevOrNextNode = pCurrNode->pNext;
#if 0
                if  ( pPrevOrNextNode->pPrev != pCurrNode )
                {
                    MY_ERR("[ERR][Free] pPrevOrNextNode->pPrev != pCurrNode (pPrev, pPrev->u4Index, pCurrNode, pCurrNode->u4Index)=(%p, %ld, %p, %ld)\n", pPrevOrNextNode->pPrev, pPrevOrNextNode->pPrev->u4Index, pCurrNode, pCurrNode->u4Index);
                }
#endif
                if  (ESysramUser_None == pPrevOrNextNode->eOwner)
                {
                    //Merge next: curr(o) + next(x)
                    pTempNode = pPrevOrNextNode;
                    pCurrNode->u4Length += pTempNode->u4Length;
                    pCurrNode->pNext = pTempNode->pNext;
                    if  (NULL != pCurrNode->pNext)
                    {
                        pCurrNode->pNext->pPrev = pCurrNode;
                    }
                    FreeNode(pMemPoolInfo, pTempNode);
                }
            }
            break;
        }
    }
    return  bRet;
}

static unsigned long AllocOwner(ESysramUser_T const eOwner, unsigned long u4Size, unsigned long const u4Alignment)
{
    unsigned long u4Addr = 0;
    MEM_POOL_BANK_NO_T const eMemPoolBankNo = GetMemPoolNo(eOwner);

    if  ( IsBadOwner(eOwner) )
    {
        printk("[SYSRAM][AllocOwner]: eOwner(%d) out of range [0, %d) \n", eOwner, ESysramUser_CNT);
        return  0;
    }

    if  ( ! IsLegalSizeToAlloc(eMemPoolBankNo, eOwner, u4Size) )
    {
        if(g_u4EnableOverSizeFail)
        {
            return 0;
        }
    }

    switch  (eMemPoolBankNo)
    {
    case MEM_POOL_BAD:
    case MEM_POOL_CNT:
        //  Do nothing.
        break;
    case MEM_POOL_STATIC:   //  Static
        u4Addr = g_au4StaticUserAddr[eOwner];
        if  ( 0 == u4Addr )
        {   //  Shoundn't happen.
            printk("[SYSRAM] g_au4StaticUserAddr[%d] == 0\n", eOwner);
        }
        if  ( g_au4MaxUserSize[eOwner] < u4Size )
        {
            printk("[SYSRAM][Static User: %s] forbid allocating!!\n", g_apszOwnerName[eOwner]);
            u4Addr = 0;
        }
        break;
    default:                //  Dynamic
        u4Addr = Allocate(eOwner , u4Size, u4Alignment , eMemPoolBankNo);
        break;
    }

    if  ( 0 < u4Addr )
    {
        LockOwner_ResMgr(eOwner, u4Size);
    }
    return u4Addr;
}

static void FreeOwner(ESysramUser_T const eOwner)
{
    MEM_POOL_BANK_NO_T const eMemPoolBankNo = GetMemPoolNo(eOwner);

    switch  (eMemPoolBankNo)
    {
    case MEM_POOL_BAD:
    case MEM_POOL_CNT:
        //  Do nothing.
        break;
    case MEM_POOL_STATIC:   //  Static
        UnlockOwner_ResMgr(eOwner);
        break;
    default:                //  Dynamic
        if  ( Free(eOwner, eMemPoolBankNo) )
        {
            UnlockOwner_ResMgr(eOwner);
        }
        else
        {
            MY_LOG("[SYSRAM][FreeOwner] Cannot free eOwner(%d)\n", eOwner);
            MTxxx_SYSRAM_DUMPLAYOUT();
        }
        break;
    }
}

static unsigned long MsToJiffies(unsigned long u4ms)
{
    return ((u4ms*HZ + 512) >> 10);
}

static unsigned long TryAllocOwner(ESysramUser_T const eOwner, unsigned long const u4Size, unsigned long const u4Alignment)
{
    unsigned long u4Addr = 0;

    Lock_ResMgr();

    if  ( IsLockedOwner_ResMgr(eOwner) )
    {
        Unlock_ResMgr();
        printk("[SYSRAM][User: %s] has been already allocated!!\n" , g_apszOwnerName[eOwner]);
        return 0;
    }

    u4Addr = AllocOwner(eOwner, u4Size, u4Alignment);
    if  (0 == u4Addr)
    {
#if 0
        printk("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printk("[SYSRAM][User: %s] fails to allocate %lu BYTES with %lu alignment\n", g_apszOwnerName[eOwner], u4Size, u4Alignment);
        MTxxx_SYSRAM_DUMPLAYOUT();
#endif
    }

    Unlock_ResMgr();

    return  u4Addr;
}

static unsigned long MTxxx_SYSRAM_ALLOC_TIMEOUT(ESysramUser_T const eOwner, unsigned long const u4Size, unsigned long u4Alignment, unsigned long const u4TimeoutInMS)
{
    unsigned long u4Addr = 0;
    long iTimeOut = 0;

    if  ( IsBadOwner(eOwner) )
    {
        printk("[SYSRAM][ALLOC]: eOwner(%d) out of range [0, %d) \n", eOwner, ESysramUser_CNT);
        return  0;
    }

    if  (0 == u4Size)
    {
        printk("[SYSRAM][User: %s] allocates 0 size!!\n", g_apszOwnerName[eOwner]);
        return  0;
    }

    u4Addr = TryAllocOwner(eOwner, u4Size, u4Alignment);
    if  (
            0 != u4Addr         //success
        ||  0 == u4TimeoutInMS  //failure without a timeout specified
        )
    {
        goto lbExit;
    }

    iTimeOut = wait_event_interruptible_timeout(
        g_ResMgr.wq_head, 
        0 != ( u4Addr = TryAllocOwner(eOwner, u4Size, u4Alignment) ), 
        MsToJiffies(u4TimeoutInMS)
    );
    if  (0 == iTimeOut && 0 == u4Addr )
    {
        printk("[SYSRAM][User: %s] allocate timeout\n", g_apszOwnerName[eOwner]);
    }

lbExit:
    if  (0 == u4Addr)
    {   //  Failure
        printk("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printk(
            "[SYSRAM][User: %s] fails to allocate (Size in Bytes, Alignment, u4TimeoutInMS)=(%lu, %lu, %lu)\n"
            , g_apszOwnerName[eOwner], u4Size, u4Alignment, u4TimeoutInMS
        );
        MTxxx_SYSRAM_DUMPLAYOUT();
    }
    else
    {   //  Success
#if 1
        if  ( (1<<eOwner) & ELogOwnersMask )
        {
            MY_DBG("-[SYSRAM][Alloc][User: %s] alloc %lu bytes successfully\n", g_apszOwnerName[eOwner], u4Size);
        }
#endif
    }

    return u4Addr;
}

static unsigned long MTxxx_SYSRAM_ALLOC(ESysramUser_T eOwner, unsigned long u4Size, unsigned long u4Alignment)
{
    return  MTxxx_SYSRAM_ALLOC_TIMEOUT(eOwner, u4Size, u4Alignment, 0);
}

static void MTxxx_SYSRAM_FREE(ESysramUser_T eOwner)
{
    if  ( IsBadOwner(eOwner) )
    {
        printk("[SYSRAM][FREE]: eOwner(%d) out of range [0, %d) \n", eOwner, ESysramUser_CNT);
        return;
    }

    Lock_ResMgr();
    FreeOwner(eOwner);

    wake_up_interruptible(&g_ResMgr.wq_head);

    Unlock_ResMgr();

    if  ( (1<<eOwner) & ELogOwnersMask )
    {
        MY_DBG("-[SYSRAM][Free][User: %s] try to free\n", g_apszOwnerName[eOwner]);
    }
}

static int SYSRAM_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    unsigned int u4Index = 0;

    Lock_ResMgr();

    a_pstFile->private_data = NULL;

    for(u4Index = 0 ; u4Index < ESysramUser_CNT; u4Index++)
    {
        if(0 == g_pstSysramLog[u4Index].u4Occupied)
        {
            g_pstSysramLog[u4Index].u4Occupied = 1;
            g_pstSysramLog[u4Index].u4Tbl = 0;
            a_pstFile->private_data = (void *)&g_pstSysramLog[u4Index];
            break;
        }
    }

    Unlock_ResMgr();

    if(NULL == a_pstFile->private_data)
    {
        printk("open the driver too many times\n");
        return -ENOMEM;
    }

    return 0;
}

static int SYSRAM_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    return 0;
}

static int SYSRAM_Flush(struct file * a_pstFile , fl_owner_t a_id)
{
    unsigned int u4Index = 0;
    SysramLog_T * pstLog = NULL;

    Lock_ResMgr();  //+

    pstLog = (SysramLog_T *)a_pstFile->private_data;

    if  ( ! pstLog )
    {
        printk("[SYSRAM_Flush] private_data == NULL. (process, pid, tgid)=(%s, %d, %d)\n", current->comm , current->pid, current->tgid);
        goto lbExit;
    }

    if(pstLog->u4Tbl)
    {
        printk("[SYSRAM_Flush] Some sysram buffers are not freed before. (process, pid, tgid)=(%s, %d, %d)\n", current->comm , current->pid, current->tgid);
        MTxxx_SYSRAM_DUMPLAYOUT();
    }

    for(u4Index = 0 ; u4Index < ESysramUser_CNT; u4Index++)
    {
        if(pstLog->u4Tbl & (1 << u4Index))
        {
            Unlock_ResMgr();//-
            MTxxx_SYSRAM_FREE((ESysramUser_T)u4Index);
            Lock_ResMgr();  //+
        }
    }

    pstLog->u4Occupied = 0;
    pstLog->u4Tbl = 0;

    a_pstFile->private_data = NULL;

lbExit:
    Unlock_ResMgr();//-

    return 0;
}

static int SYSRAM_mmap(struct file *file, struct vm_area_struct *vma)
{
    MY_LOG("[SYSRAM_mmap] \n");

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    if  (   remap_pfn_range(
            vma, vma->vm_start, vma->vm_pgoff, 
            vma->vm_end - vma->vm_start, vma->vm_page_prot
            )
        )
    {
        MY_LOG("[SYSRAM_mmap] fail\n");
        return -EAGAIN;
    }

    return 0;
}

static int SYSRAM_Ioctl(struct inode* a_pstInode, struct file* a_pstFile, unsigned int a_u4Command, unsigned long a_u4Param)
{
    int i4RetVal = 0;

    SysramLog_T *pstLog = NULL;

    stSysramParam stParam;

    if  ( _IOC_NONE != _IOC_DIR(a_u4Command) )
    {
        if  ( copy_from_user(&stParam, (void *)a_u4Param, sizeof(stSysramParam)) )
        {
            printk("[SYSRAM][Ioctl] copy from user failed\n");
            i4RetVal = -EFAULT;
            return  i4RetVal;
        }
    }

    Lock_ResMgr();  //+
    pstLog = (SysramLog_T *)a_pstFile->private_data;
    Unlock_ResMgr();//-

    if(NULL == pstLog)
    {
        printk("[SYSRAM_Ioctl] private_data==NULL!!\n");
        i4RetVal = -1;
        goto lbExit;
    }

    switch(a_u4Command)
    {
    case SYSRAM_X_USRALLOC_TIMEOUT:
        stParam.u4Addr = MTxxx_SYSRAM_ALLOC_TIMEOUT(stParam.u4Owner , stParam.u4Size, stParam.u4Alignment, stParam.u4TimeoutInMS);
        if  (0 != stParam.u4Addr)
        {
            Lock_ResMgr();  //+
            pstLog->u4Tbl |= (1 << stParam.u4Owner);
            Unlock_ResMgr();//-
        }
        else
        {   //  Fail to alloc since address == 0.
            i4RetVal = -EFAULT;
        }
        break;
    case SYSRAM_X_USRALLOC:
        stParam.u4Addr = MTxxx_SYSRAM_ALLOC(stParam.u4Owner , stParam.u4Size, stParam.u4Alignment);
        if  (0 != stParam.u4Addr)
        {
            Lock_ResMgr();  //+
            pstLog->u4Tbl |= (1 << stParam.u4Owner);
            Unlock_ResMgr();//-
        }
        else
        {   //  Fail to alloc since address == 0.
            i4RetVal = -EFAULT;
        }
        break;
    case SYSRAM_S_USRFREE:
        Lock_ResMgr();  //+
        if  ((~pstLog->u4Tbl) & (1 << stParam.u4Owner))
        {
            printk("This driver is freeing unallocated buffer!! , tbl : 0x%lx , owner : %d\n" , pstLog->u4Tbl , stParam.u4Owner);
            i4RetVal = -1;
            Unlock_ResMgr();//-
            goto lbExit;
        }
        pstLog->u4Tbl &= (~(1 << stParam.u4Owner));
        Unlock_ResMgr();//-
        MTxxx_SYSRAM_FREE(stParam.u4Owner);
        break;

    case SYSRAM_T_DUMPLAYOUT:
        MTxxx_SYSRAM_DUMPLAYOUT();
        break;

    default:
        MY_DBG("sysram ioctl : No such command!!\n");
        i4RetVal = -EINVAL;
        break;
    }

lbExit:

    if  ( _IOC_READ & _IOC_DIR(a_u4Command) )
    {
        if  ( copy_to_user((void __user *)a_u4Param, &stParam, sizeof(stSysramParam)) )
        {
            printk("[SYSRAM][Ioctl] copy to user failed\n");
            i4RetVal = -EFAULT;
        }
    }

    return i4RetVal;
}

static const struct file_operations g_stSYSRAM_fops = 
{
	.owner = THIS_MODULE,
	.open  = SYSRAM_Open,
	.release = SYSRAM_Release,
	.flush = SYSRAM_Flush,
	.ioctl = SYSRAM_Ioctl,
	.mmap  = SYSRAM_mmap,
};

static struct cdev * g_pSYSRAM_CharDrv = NULL;
static dev_t g_SYSRAMdevno = MKDEV(MT6573_SYSRAM_DEV_MAJOR_NUMBER,0);
static inline int RegisterSYSRAMCharDrv(void)
{
    if( alloc_chrdev_region(&g_SYSRAMdevno, 0, 1,SYSRAM_DEV_NAME) )
    {
        MY_DBG("[SYSRAM][RegisterSYSRAMCharDrv] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pSYSRAM_CharDrv = cdev_alloc();

    if(NULL == g_pSYSRAM_CharDrv)
    {
        unregister_chrdev_region(g_SYSRAMdevno, 1);

        MY_DBG("[SYSRAM][RegisterSYSRAMCharDrv] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pSYSRAM_CharDrv, &g_stSYSRAM_fops);

    g_pSYSRAM_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pSYSRAM_CharDrv, g_SYSRAMdevno, 1))
    {
        printk("[SYSRAM][RegisterSYSRAMCharDrv] Attatch file operation failed\n");

        unregister_chrdev_region(g_SYSRAMdevno, 1);

        return -EAGAIN;
    }

    return 0;
}

static inline void UnregisterSYSRAMCharDrv(void)
{
    //Release char driver
    cdev_del(g_pSYSRAM_CharDrv);

    unregister_chrdev_region(g_SYSRAMdevno, 1);
}

static struct class *pSYSRAM_CLASS = NULL;
// Called to probe if the device really exists. and create the semaphores
static int SYSRAM_probe(struct platform_device *pdev)
{
    unsigned long u4PoolIdx = 0;
    struct device* sysram_device = NULL;

    MY_LOG("[SYSRAM_probe] Start\n");

    //register char driver
    //Allocate major no
    if(RegisterSYSRAMCharDrv())
    {
        printk("register char failed\n");
        return -EAGAIN;
    }

    pSYSRAM_CLASS = class_create(THIS_MODULE, "sysramdrv");
    if (IS_ERR(pSYSRAM_CLASS)) {
        int ret = PTR_ERR(pSYSRAM_CLASS);
        printk("Unable to create class, err = %d\n", ret);
        return ret;            
    }
    sysram_device = device_create(pSYSRAM_CLASS, NULL, g_SYSRAMdevno, NULL, SYSRAM_DEV_NAME);

    //Initialize variables
    spin_lock_init(&g_ResMgr.lock);
    g_ResMgr.u4TotalUserCount   =   0;
    g_ResMgr.u4AllocatedTbl     =   0;
    g_ResMgr.pMaxUsrSizeTbl     =   &g_au4MaxUserSize;
    memset(g_ResMgr.au4AllocatedSize, 0, sizeof(g_ResMgr.au4AllocatedSize));
    memset(g_ResMgr.aOwnerTaskInfo, 0, sizeof(g_ResMgr.aOwnerTaskInfo));
    init_waitqueue_head(&g_ResMgr.wq_head);
    //
    for (u4PoolIdx = 0; u4PoolIdx < MEM_POOL_CNT; u4PoolIdx++)
    {
        g_aMemPoolInfo[u4PoolIdx].paNodes[0].eOwner   = ESysramUser_None;
        g_aMemPoolInfo[u4PoolIdx].paNodes[0].u4Offset = 0;
        g_aMemPoolInfo[u4PoolIdx].paNodes[0].u4Length = g_aMemPoolInfo[u4PoolIdx].u4Size;
        g_aMemPoolInfo[u4PoolIdx].paNodes[0].u4Index = 0;
        g_aMemPoolInfo[u4PoolIdx].paNodes[0].pNext = NULL;
        g_aMemPoolInfo[u4PoolIdx].paNodes[0].pPrev = NULL;
        g_aMemPoolInfo[u4PoolIdx].u4IndexTbl = (~0x1);
        g_aMemPoolInfo[u4PoolIdx].u4UserCount= 0;
    }

    //For error handling
    memset(g_pstSysramLog, 0, (sizeof(SysramLog_T)*ESysramUser_CNT));

    MY_LOG("[SYSRAM_probe] probe SYSRAM success\n");

    return 0;
}

// Called when the device is being detached from the driver
static int SYSRAM_remove(struct platform_device *pdev)
{
    //unregister char driver.
    UnregisterSYSRAMCharDrv();

    //
    device_destroy(pSYSRAM_CLASS, g_SYSRAMdevno);

    class_destroy(pSYSRAM_CLASS);

    MY_DBG("[SYSRAM_remove]\n");

    return 0;
}

static int SYSRAM_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int SYSRAM_resume(struct platform_device *pdev)
{
    return 0;
}

static struct platform_driver g_stSYSRAM_Platform_Driver = {
    .probe		= SYSRAM_probe,
    .remove	    = SYSRAM_remove,
    .suspend	= SYSRAM_suspend,
    .resume	    = SYSRAM_resume,
    .driver		= {
        .name	= SYSRAM_DEV_NAME,
        .owner	= THIS_MODULE,
    }
};

static int SYSRAM_dump_layout_to_proc(char *page, char **start, off_t off, int count, int *eof, void *data);
static int SYSRAM_read_flag(char *page, char **start, off_t off, int count, int *eof, void *data);
static int SYSRAM_write_flag(struct file *file, const char *buffer, unsigned long count, void *data);
static int __init SYSRAM_Init(void)
{
    struct proc_dir_entry *prEntry;

    if(platform_driver_register(&g_stSYSRAM_Platform_Driver)){
        printk("[SYSRAM]failed to register sysram driver\n");
        return -ENODEV;
    }
    //
    prEntry = create_proc_entry("sysram", 0, NULL);
    if (prEntry) {
        prEntry->read_proc = SYSRAM_dump_layout_to_proc;
        prEntry->write_proc = NULL;
    }
    else {
        MY_LOG("add /proc/sysram entry fail \n");  
    }
    //
    prEntry = create_proc_entry("sysram_flag", 0, NULL);
    if (prEntry) {
        prEntry->read_proc  = SYSRAM_read_flag;
        prEntry->write_proc = SYSRAM_write_flag;
    }
    else {
        MY_LOG("add /proc/sysram_flag entry fail \n");  
    }
    //
    return 0;
}

static void __exit SYSRAM_Exit(void)
{
    platform_driver_unregister(&g_stSYSRAM_Platform_Driver);
}

module_init(SYSRAM_Init);
module_exit(SYSRAM_Exit);
MODULE_DESCRIPTION("MT6573 sysram driver");
MODULE_AUTHOR("Gipi <Gipi.Lin@Mediatek.com>");
MODULE_AUTHOR("Jonas <Jonas.Lai@Mediatek.com>");
MODULE_LICENSE("GPL");


static void MTxxx_SYSRAM_DUMPLAYOUT(void)
{
    unsigned int u4PoolIdx = 0;
    MemNode_T* pCurrNode = NULL;

    printk("\n[SYSRAM Layout]\n");
    printk("=========================================\n");
    for (u4PoolIdx = 0; u4PoolIdx < MEM_POOL_CNT; u4PoolIdx++)
    {
        printk("\n [Mem Pool %d] (u4IndexTbl, u4UserCount)=(%lX, %ld)\n", u4PoolIdx, g_aMemPoolInfo[u4PoolIdx].u4IndexTbl, g_aMemPoolInfo[u4PoolIdx].u4UserCount);
        printk("[Locked Time/jiffies] [Owner   Offset   Size  Index pCurrent pPrevious pNext]  [pid tgid] [Proc Name / Owner Name]\n");
        pCurrNode = &g_aMemPoolInfo[u4PoolIdx].paNodes[0];
        while ( NULL != pCurrNode )
        {
            ESysramUser_T const eOwner = pCurrNode->eOwner;
            if  ( IsBadOwner(eOwner) )
            {
                printk(
                    "------------ --------"
                    " %2d\t0x%05lX 0x%05lX  %ld    %p %p\t%p\n", 
                    pCurrNode->eOwner, pCurrNode->u4Offset, pCurrNode->u4Length, 
                    pCurrNode->u4Index, pCurrNode, pCurrNode->pPrev, pCurrNode->pNext
                );
            }
            else
            {
                TaskInfo_T*const pInfo = &g_ResMgr.aOwnerTaskInfo[eOwner];
                printk(
                    "%5lu.%06lu %08lX"
                    " %2d\t0x%05lX 0x%05lX  %ld    %p %p\t%p"
                    "  %-4d %-4d \"%s\" / \"%s\"\n", 
                    pInfo->t_sec, pInfo->t_usec, pInfo->t_jiffies, 
                    eOwner, pCurrNode->u4Offset, pCurrNode->u4Length, 
                    pCurrNode->u4Index, pCurrNode, pCurrNode->pPrev, pCurrNode->pNext, 
                    pInfo->pid, pInfo->tgid, pInfo->comm, g_apszOwnerName[eOwner]
                );
            }
            pCurrNode = pCurrNode->pNext;
        };
    }

    printk("\n");

    Dump_ResMgr();
}


static int SYSRAM_dump_layout_to_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;

    unsigned int u4PoolIdx = 0;
    MemNode_T * pCurrNode = NULL;

    //
    p += sprintf(p, "\n[SYSRAM Layout]\n");
    p += sprintf(p, "=========================================\n" );
    for (u4PoolIdx = 0; u4PoolIdx < MEM_POOL_CNT; u4PoolIdx++)
    {
        p += sprintf(p, "\n [Mem Pool %d] (u4IndexTbl, u4UserCount)=(%lX, %ld)\n", u4PoolIdx, g_aMemPoolInfo[u4PoolIdx].u4IndexTbl, g_aMemPoolInfo[u4PoolIdx].u4UserCount);
        p += sprintf(p, "[Locked Time/jiffies] [Owner   Offset   Size  Index pCurrent pPrevious pNext]  [pid tgid] [Proc Name / Owner Name]\n");
        pCurrNode = &g_aMemPoolInfo[u4PoolIdx].paNodes[0];
        while ( NULL != pCurrNode )
        {
            ESysramUser_T const eOwner = pCurrNode->eOwner;
            if  ( IsBadOwner(eOwner) )
            {
                p += sprintf(p, 
                    "------------ --------"
                    " %2d\t0x%05lX 0x%05lX  %ld    %p %p\t%p\n", 
                    pCurrNode->eOwner, pCurrNode->u4Offset, pCurrNode->u4Length, 
                    pCurrNode->u4Index, pCurrNode, pCurrNode->pPrev, pCurrNode->pNext
                );
            }
            else
            {
                TaskInfo_T*const pInfo = &g_ResMgr.aOwnerTaskInfo[eOwner];
                p += sprintf(p, 
                    "%5lu.%06lu %08lX"
                    " %2d\t0x%05lX 0x%05lX  %ld    %p %p\t%p"
                    "  %-4d %-4d \"%s\" / \"%s\"\n", 
                    pInfo->t_sec, pInfo->t_usec, pInfo->t_jiffies, 
                    eOwner, pCurrNode->u4Offset, pCurrNode->u4Length, 
                    pCurrNode->u4Index, pCurrNode, pCurrNode->pPrev, pCurrNode->pNext, 
                    pInfo->pid, pInfo->tgid, pInfo->comm, g_apszOwnerName[eOwner]
                );
            }
            pCurrNode = pCurrNode->pNext;
        };
    }

    *start = page + off;

    len = p - page;
    if (len > off) {
        len -= off;
    }
    else {
        len = 0;
    }

    return len < count ? len : count;
}

static int SYSRAM_read_flag(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;
    //
    p += sprintf(p, "\r\n[SYSRAM debug flag]\r\n");
    p += sprintf(p, "=========================================\r\n" );
    p += sprintf(p, "g_u4SysramDbgFlag = 0x%08X\r\n", g_u4SysramDbgFlag);
    p += sprintf(p, "g_u4EnableUserGroupSize = %u\r\n", g_u4EnableUserGroupSize);
    p += sprintf(p, "g_u4EnableOverSizeFail = %u\r\n", g_u4EnableOverSizeFail);

    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}


static int SYSRAM_write_flag(struct file *file, const char *buffer, unsigned long count, void *data)
{
    char acBuf[32]; 
    unsigned long u4CopySize = 0;
    unsigned int  u4SysramDbgFlag = 0;
    unsigned int  u4EnableUserGroupSize = 0;
    unsigned int  u4EnableOverSizeFail = 0;

    u4CopySize = (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
    if(copy_from_user(acBuf, buffer, u4CopySize))
		return 0;
    acBuf[u4CopySize] = '\0';

    if ( 3 == sscanf(acBuf, "%x%u%u", &u4SysramDbgFlag, &u4EnableUserGroupSize, &u4EnableOverSizeFail) )
    {
        g_u4SysramDbgFlag = u4SysramDbgFlag;
        g_u4EnableUserGroupSize = u4EnableUserGroupSize;
        g_u4EnableOverSizeFail = u4EnableOverSizeFail;
    }
    return count;
}

//-----------------------------------------------------------------------------
//This is compatible with other chip.
unsigned long MT6573_SYSRAM_ALLOC_TIMEOUT(ESysramUser_T const eOwner, unsigned long const u4Size, unsigned long u4Alignment, unsigned long const u4TimeoutInMS)
{
    return MTxxx_SYSRAM_ALLOC_TIMEOUT(eOwner, u4Size, u4Alignment, u4TimeoutInMS);
}

unsigned long MT6573_SYSRAM_ALLOC(ESysramUser_T eOwner, unsigned long u4Size, unsigned long u4Alignment)
{
    return MTxxx_SYSRAM_ALLOC(eOwner, u4Size, u4Alignment);
}

void MT6573_SYSRAM_FREE(ESysramUser_T eOwner)
{
    MTxxx_SYSRAM_FREE(eOwner);
}
//-----------------------------------------------------------------------------


EXPORT_SYMBOL(MT6573_SYSRAM_ALLOC_TIMEOUT);
EXPORT_SYMBOL(MT6573_SYSRAM_ALLOC);
EXPORT_SYMBOL(MT6573_SYSRAM_FREE);

