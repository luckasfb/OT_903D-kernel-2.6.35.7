#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

/******************************************************************************
 *  MODULE NAME
 ******************************************************************************/
#define MOD                         "ASP"

/**************************************************************************
*   INTERNAL DEFINITION
**************************************************************************/
#define SEC_OK                      0x0000
#define SEC_SBOOT_NOT_ENABLED       0x9007
#define SEC_SUSBDL_NOT_ENABLED      0x9009

/**************************************************************************
 *  EXTERNAL FUNCTION
 **************************************************************************/
extern int asp_main(int argc, char *argv[], bool bPunishCtrl);

/******************************************************************************
 *  MAIN FLOW
 ******************************************************************************/

int main(int argc, char *argv[])
{
    int ret = SEC_OK;
   
    /* =================================== */
    /* do check                            */
    /* =================================== */
    /* @ asp_main parameter */
    /*   1st : application input argument */
    /*   2nd : application input argument */
    /*   3rd : do sbchk punishment 'kernel assert' or not when image is verified fail */    
    ret = asp_main(argc,argv,true);

    if((ret == SEC_SBOOT_NOT_ENABLED) || (ret == SEC_SUSBDL_NOT_ENABLED))
    {
        printf("[%s] no check. ret 0x%x\n",MOD,ret);
        goto _end;
    }
    
    if(SEC_OK != ret)
    {   
        printf("[%s] check fail. ret '0x%x'\n",MOD,ret);
        assert(0);
    }

_end:
    return ret;    
    
}
