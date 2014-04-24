





#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "platform.h"







#define WIFI_NVRAM_FILE_NAME   "/data/nvram/APCFG/APRDEB/WIFI"
#define WIFI_NVRAM_CUSTOM_NAME "/data/nvram/APCFG/APRDEB/WIFI_CUSTOM"

WIFI_CFG_DATA gPlatformCfg;
EXPORT_SYMBOL(gPlatformCfg);   
int platform_load_nvram_data( char * filename, char * buf, int len );

#if 0
bool
platformNvramRead16( void * prAdapter,unsigned char ucWordOffset, unsigned short * pu2Data );

bool
platformNvramWrite16( void * prAdapter,unsigned char ucWordOffset,unsigned short u2Data );
#endif


bool
cfgDataRead16( void * prAdapter,unsigned char ucWordOffset, unsigned short * pu2Data );
bool
cfgDataWrite16( void * prAdapter,unsigned char ucWordOffset,unsigned short u2Data );

bool
customDataRead8( void * prAdapter,unsigned char ucByteOffset, unsigned char * pucData );

bool
customDataWrite8( void * prAdapter,unsigned char ucByteOffset,unsigned char ucData );

int platform_init(void);
void platform_deinit(void);
static int nvram_read(char *filename, char *buf, ssize_t len, int offset)
{	
    struct file *fd;
    //ssize_t ret;
    int retLen = -1;
    
    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    fd = filp_open(filename, O_WRONLY|O_CREAT, 0644);
    
    if(IS_ERR(fd)) {
        printk("[MT5921][nvram_read] : failed to open!!\n");
        return -1;
    }
    do{
        if ((fd->f_op == NULL) || (fd->f_op->read == NULL))
    		{
            printk("[MT5921][nvram_read] : file can not be read!!\n");
            break;
    		} 
    		
        if (fd->f_pos != offset) {
            if (fd->f_op->llseek) {
        		    if(fd->f_op->llseek(fd, offset, 0) != offset) {
						printk("[MT5921][nvram_read] : failed to seek!!\n");
					    break;
        		    }
        	  } else {
        		    fd->f_pos = offset;
        	  }
        }    		
        
    		retLen = fd->f_op->read(fd,
    									  buf,
    									  len,
    									  &fd->f_pos);			
    		
    }while(false);
    
    filp_close(fd, NULL);
    
    set_fs(old_fs);
    
    return retLen;
}

static int nvram_write(char *filename, char *buf, ssize_t len, int offset)
{	
    struct file *fd;
    //ssize_t ret;
    int retLen = -1;
        
    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    fd = filp_open(filename, O_WRONLY|O_CREAT, 0644);
    
    if(IS_ERR(fd)) {
        printk("[MT5921][nvram_write] : failed to open!!\n");
        return -1;
    }
    do{
        if ((fd->f_op == NULL) || (fd->f_op->write == NULL))
    		{
            printk("[MT5921][nvram_write] : file can not be write!!\n");
            break;
    		} /* End of if */
    		
        if (fd->f_pos != offset) {
            if (fd->f_op->llseek) {
        	    if(fd->f_op->llseek(fd, offset, 0) != offset) {
				    printk("[MT5921][nvram_write] : failed to seek!!\n");
                    break;
                }
            } else {
                fd->f_pos = offset;
            }
        }       		
        
        retLen = fd->f_op->write(fd,
                                 buf,
                                 len,
                                 &fd->f_pos);			
    		
    }while(false);
    
    filp_close(fd, NULL);
    
    set_fs(old_fs);
    
    return retLen;
}
	

int platform_load_nvram_data( char * filename, char * buf, int len)
{
    //int ret;
    printk("[wifi] platform_load_nvram_data ++\n");

    return nvram_read( filename, buf, len, 0);
}

bool
cfgDataRead16( void * prAdapter,unsigned char ucWordOffset, unsigned short * pu2Data )
{
    if( pu2Data == NULL )
        return false;
        
    if( nvram_read( WIFI_NVRAM_FILE_NAME, (char *)pu2Data, sizeof(unsigned short), ucWordOffset*sizeof(unsigned short)) != sizeof(unsigned short) )
        return false;
    else 
    	  return true;	
}
EXPORT_SYMBOL(cfgDataRead16);

bool
cfgDataWrite16( void * prAdapter,unsigned char ucWordOffset,unsigned short u2Data )    
{
    if( nvram_write( WIFI_NVRAM_FILE_NAME, (char *)&u2Data, sizeof(unsigned short), ucWordOffset*sizeof(unsigned short)) != sizeof(unsigned short) )
        return false;
    else 
    	  return true;
}
EXPORT_SYMBOL(cfgDataWrite16);

bool
customDataRead8( void * prAdapter,unsigned char ucByteOffset, unsigned char * pucData )
{
    if( pucData == NULL )
        return false;
        
    if( nvram_read( WIFI_NVRAM_CUSTOM_NAME, (char *)pucData, sizeof(unsigned char), ucByteOffset*sizeof(unsigned char)) != sizeof(unsigned char) )
        return false;
    else 
    	  return true;	
}
EXPORT_SYMBOL(customDataRead8);

bool
customDataWrite8( void * prAdapter,unsigned char ucByteOffset,unsigned char ucData )    
{
    if( nvram_write( WIFI_NVRAM_CUSTOM_NAME, (char *)&ucData, sizeof(unsigned char), ucByteOffset*sizeof(unsigned char)) != sizeof(unsigned char) )
        return false;
    else 
    	  return true;
}
EXPORT_SYMBOL(customDataWrite8);

#if 0
bool
platformNvramRead16( void * prAdapter,unsigned char ucWordOffset, unsigned short * pu2Data )
{
    if( pu2Data == NULL )
        return false;
        
    if( nvram_read( WIFI_NVRAM_FILE_NAME, (char *)pu2Data, sizeof(unsigned short), ucWordOffset*sizeof(unsigned short)) != sizeof(unsigned short) )
        return false;
    else 
    	  return true;	
}

EXPORT_SYMBOL(platformNvramRead16);      
                      
bool
platformNvramWrite16( void * prAdapter,unsigned char ucWordOffset,unsigned short u2Data )    
{
    if( nvram_write( WIFI_NVRAM_FILE_NAME, (char *)&u2Data, sizeof(unsigned short), ucWordOffset*sizeof(unsigned short)) != sizeof(unsigned short) )
        return false;
    else 
    	  return true;
}
EXPORT_SYMBOL(platformNvramWrite16);      
#endif

int platform_init(void)
{
    memset( &gPlatformCfg, 0, sizeof(gPlatformCfg) );
#if !BUILD_USE_EEPROM	    
    gPlatformCfg.u4Cfglen = platform_load_nvram_data( WIFI_NVRAM_FILE_NAME, (char *)&gPlatformCfg.rWifiNvram, CFG_FILE_WIFI_REC_SIZE);
#endif    

    gPlatformCfg.u4Customlen = platform_load_nvram_data( WIFI_NVRAM_CUSTOM_NAME, (char *)&gPlatformCfg.rWifiCustom, CFG_FILE_WIFI_CUSTOM_REC_SIZE);
    return 0;
}

EXPORT_SYMBOL(platform_init);    

void platform_deinit(void)
{
#if !BUILD_USE_EEPROM	   
    memset( &gPlatformCfg, 0, sizeof(gPlatformCfg) );
#endif    
}

EXPORT_SYMBOL(platform_deinit);    
