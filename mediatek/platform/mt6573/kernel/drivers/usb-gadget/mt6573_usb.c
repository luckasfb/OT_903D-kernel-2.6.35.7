#include <linux/device.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/cdc.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/wait.h>
#include <linux/platform_device.h>

#include <linux/wakelock.h>
/* architecture dependent header files */
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_reg_base.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <mach/mt6573_udc.h>


extern void mt6573_usb_phy_recover_usb (void);
extern void mt6573_usb_phy_poweron_usb(void);

#define DWORD unsigned int 
#define WORD unsigned int 

#define UCHAR unsigned char
#define BYTE unsigned char
#define UINT32 unsigned int 
#define UINT8 unsigned char
#define UINT16 unsigned short
#define void void
#define bool bool
#define PBYTE unsigned char *


#define CONTROL_ENDPT   0
#define BULKOUT_ENDPT   1   //bulk out
#define BULKIN_ENDPT    2   //bulk in
#define MAX_ENDPT       BULKIN_ENDPT

#define MAX_BULKRX_PKT_SIZE   (512*20)
static BYTE m_BulkRxPktBuf[MAX_BULKRX_PKT_SIZE];
static bool m_fConnected = FALSE;

DWORD gs_EpMaxSize[]={
	0x40, //EP0 64
	0x200, //Rx
  0x200, //Tx
};

typedef struct {
    unsigned char* pbBuffer;               //buffer
    unsigned int cbTransferSize;         //size
    unsigned int cbTransferred;          //have transferred
    unsigned int cbToTransfer;           //this time  
} EPTransfer;

static EPTransfer m_epTransfers[MAX_ENDPT+1];

static unsigned char m_DeviceDescriptors[] = 
{
    /* 0  */    18,                         //  bLength = 18 bytes.
    /* 1  */    USB_DT_DEVICE, //  bDescriptorType = DEVICE
    /* 2  */    0x00, 0x02,                 //  bcdUSB          = 2.0
    /* 4  */    0xff,                       //  bDeviceClass    = Communication Device Class
    /* 5  */    0xff,                       //  bDeviceSubClass = Unused at this time.
    /* 6  */    0xff,                       //  bDeviceProtocol = Unused at this time.
    /* 7  */    64,                         //  EP0 MaxPacketSize
    /* 8  */    0x5E, 0x04,                 //  idVendor = Microsoft Vendor ID.
    /* 10 */    0xCE, 0x00,                 //  idProduct
    /* 12 */    0, 0,                       //  bcdDevice
    /* 14 */    0x00,                       //  iManufacturer   
    /* 15 */    0x00,                       //  iProduct        
    /* 16 */    0x00,                       //  iSerialNumber   
    /* 17 */    0x01,                       //  bNumConfigs
};

static unsigned char m_ConfigDescriptors[] =
{
    /* 0 */    9,                                 // bLength
    /* 1 */    USB_DT_CONFIG, 
    /* 2 */    32, 0,                             // wTotalLength
    /* 4 */    1,                                 // bNumInterfaces
    /* 5 */    1,                                 // bConfigurationValue
    /* 6 */    0,                                 // iConfiguration
    /* 7 */    0x40, //0xC0,                      // bmAttributes = Self-Powered & Bus-Powered
    /* 8 */    0x1,  //32                         // MaxPower = 100mA
//-------------------------------------------------------
//         Interface Descriptor (One Interface) 
//-------------------------------------------------------
    /* 9  */    9,                                // bLength
    /* 10 */    USB_DT_INTERFACE, 
    /* 11 */    0,                                // bInterfaceNumber
    /* 12 */    0,                                // bAlternateSetting
    /* 13 */    2,                                // bNumEndpoints (number endpoints used, excluding EP0)
    /* 14 */    0xff,                             // bInterfaceClass
    /* 15 */    0xff,                             // bInterfaceSubClass
    /* 16 */    0xff,                             // bInterfaceProtocol
    /* 17 */    0,                                // ilInterface  (Index of this interface string desc.)
//-------------------------------------------------------
//          Endpoint Descriptor (EP1 - BULK OUT)          
//-------------------------------------------------------
    /* 18 */    7,                                // bLength
    /* 19 */    USB_DT_ENDPOINT,
    /* 20 */    0x01,                             // bEndpointAddress (EP 1, OUT)
    /* 21 */    2,                                // bmAttributes  (0010 = Bulk)
    /* 22 */    0, 0x2,                           // wMaxPacketSize. retrieved from PDD. 0 by default.
    /* 24 */    0,                                // bInterval (ignored for Bulk)
//-------------------------------------------------------
//          Endpoint Descriptor (EP2 - BULK IN)          
//-------------------------------------------------------
    /* 25 */    7,                                   // bLength
    /* 26 */    USB_DT_ENDPOINT,   
    /* 27 */    0x82,                                // bEndpointAddress (EP 2, IN)
    /* 28 */    2,                                   // bmAttributes  (0010 = Bulk)
    /* 29 */    0, 0x2,                              // wMaxPacketSize. retrieved from PDD. 0 by default.
    /* 31 */    0                                    // bInterval (ignored for Bulk)
};


/*-----------------------
   Config HW and every EP
-------------------------*/
void Ser_Config(void)
{ /*there is no interrupt in eboot*/

    // EP Interrupt
    OUTREG16(INTRTXE, 0);
    OUTREG16(INTRRXE, 0);
    OUTREG8(INTRUSBE, 0);


	OUTREG16(INTRRX, 0);
	INREG8(INTRUSB);
	INREG16(INTRTX);




    //Enable High-Speep
    SETREG8(POWER, PWR_HS_ENAB);

	//Enable reset interrupt
//	OUTREG8(INTRUSBE, 0x4);
	
    //EP0 Flush   
    OUTREG8(INDEX, 0);
    SETREG16(IECSR + CSR0, EP0_FLUSH_FIFO);
   // SETREG16(INTRTXE, 0x5); 
    
    //---------------------------
    //      EP Config
    //    1:FIFOSz & MaxP
    //    2:start Address
    //---------------------------
    //Rx1 bulk 0x200  start:0x8
    OUTREG8(INDEX, 1); 
    OUTREG16(IECSR + RXMAP, gs_EpMaxSize[1]);
    OUTREG8(RXFIFOSZ, 6); //0x200 <--> 6
    OUTREG16(RXFIFOADD, 0x8);  //Actual Size is 8 times as what be written 
    SETREG16(IECSR + RXCSR,(EPX_RX_CLRDATATOG | EPX_RX_FLUSHFIFO));
    CLRREG16(IECSR + RXCSR, EPX_RX_ISO);                      
    //Tx2 bulk 0x200  start:72
    OUTREG8(INDEX, 2);
    OUTREG16(IECSR + TXMAP, gs_EpMaxSize[2]);
    OUTREG8(TXFIFOSZ, 6); 
    OUTREG16(TXFIFOADD, 72);   //Fix
    SETREG16(IECSR + TXCSR, (EPX_TX_CLRDATATOG | EPX_TX_FLUSHFIFO));
    CLRREG16(IECSR + TXCSR, EPX_TX_ISO);       
}

/* ---------------------------------
  Power On,Config EP,Attach to bus
  TRUE if success, FALSE if error 
-----------------------------------*/

bool                        
UsbSerial_Init(void )
{ 
	 //set usb phy and enable clock
	  mt6573_usb_phy_recover_usb();
//	mt6573_usb_phy_poweron();
	  //Get usb hw register
	  Ser_Config();
	  CLRREG8(POWER, PWR_SOFT_CONN);	  
	  // Config HW EP
	  
	  printk("serial:: Attach to Bus\r\n");
	  SETREG8(POWER, PWR_SOFT_CONN);	  
	  return TRUE;
}

static void
Ser_ResetEP(DWORD dwEndPoint)
{
    OUTREG8(INDEX, dwEndPoint);
 //   printk("reset ep %d\n",dwEndPoint);
    if(dwEndPoint == 0 )
    {
        // Clear all EP0 Status bits
        OUTREG16(IECSR + CSR0, 0x00);
        SETREG16(IECSR + CSR0, EP0_FLUSH_FIFO);
        SETREG16(IECSR + CSR0,
                (EP0_SERVICED_RXPKTRDY | EP0_SERVICE_SETUP_END));
    }
    else if(dwEndPoint == 1)
    {
        OUTREG16(IECSR + RXCSR,
                    (EPX_RX_CLRDATATOG | EPX_RX_FLUSHFIFO));
        //disable Rx interrupt            
     //   CLRREG16(INTRRXE, (0x01 << dwEndPoint));
        //clear Rx Intr,wirte 0 clear
        OUTREG16(INTRRX, 0);
    }
    else//Tx
    {
       OUTREG16(IECSR + TXCSR,
                    (EPX_TX_CLRDATATOG | EPX_TX_FLUSHFIFO));
	   INREG16(INTRTX);
        //disable Tx interrupt
    //    CLRREG16(INTRTXE, (0x01 << dwEndPoint));
        //Tx Intr Read Clear 
    }
}


/*---------------
    For debug
-----------------*/
static void Ser_DumpReg(void)
{
#if 0
	
  OUTREG8(&gs_pUSBRegs->INDEX, 1); 
  DBGMSG(1,("Dump EP1...\r\n"));
  DBGMSG(1,("RxMapP=%x\r\n",INREG16(&gs_pUSBRegs->Indexed_CSR.EPX.RxMapP)));
  DBGMSG(1,("RxFIFOSz=%x\r\n",INREG8(&gs_pUSBRegs->RxFIFOSz)));
  DBGMSG(1,("RxFIFOAddr=%x\r\n",INREG16(&gs_pUSBRegs->RxFIFOAddr)));

  OUTREG8(&gs_pUSBRegs->INDEX, 2); 
  DBGMSG(1,("Dump EP2\r\n"));
  DBGMSG(1,("TxMapP=%x\r\n",INREG16(&gs_pUSBRegs->Indexed_CSR.EPX.TxMapP)));
  DBGMSG(1,("TxFIFOSz=%x\r\n",INREG8(&gs_pUSBRegs->TxFIFOSz)));
  DBGMSG(1,("TxFIFOAddr=%x\r\n",INREG16(&gs_pUSBRegs->TxFIFOAddr)));          
                	
#endif
}

static void Ser_ResetDevice(void)
{
    //DBGMSG(1,("Enter Function Ser_ResetDevice\r\n")); 
    OUTREG16(SWRST,SWRST_DISUSBRESET | SWRST_SWRST);
    OUTREG16(INTRRX, 0);
    INREG16(INTRTX);
 	INREG8(INTRUSB);
    
    // Bus Interrupt
    OUTREG16(INTRTXE, 0);
    OUTREG16(INTRRXE, 0);
    OUTREG8(INTRUSBE, 0);//Change Marco

    //Enable High-Speep
    SETREG8(POWER, PWR_HS_ENAB);
    Ser_ResetEP(0);
    Ser_ResetEP(1);
    Ser_ResetEP(2);


	//SETREG16(INTRTXE, 0x1);
	printk("INxTRTXE == %x INTRRXE= %x\n",INREG16(INTRTXE),INREG16(INTRRXE));
		printk("INxTRTX == %x INTRRX= %x\n",INREG16(INTRTX),INREG16(INTRRX));
		

    Ser_DumpReg();
    
    m_fConnected = FALSE;
    OUTREG8(INDEX, 0); 
} 

static
DWORD
S_SendData(
    UINT32 epNum,
    UINT8 *pBuffer, 
    UINT16 cbDataLen
    )
{
    DWORD *pdwBuffer;  
    u16  wCSRToWrite; 	
    
    
    //DBGMSG(1, (("serial::S_SendData.\r\n")));
    
    OUTREG8(INDEX, epNum); 
    while(INREG16(IECSR + TXCSR) & EPX_TX_FIFONOTEMPTY){
       printk("serial::error!!!! USB_TXCSR_FIFONotEmpty\r\n");
    }  
    if(cbDataLen > 0x200){
        printk("serial::error!,why eboot send a data length\r\n");	
    }
  
    //write data to FIFO   
    if(pBuffer!=NULL){          
	    if(((DWORD)pBuffer)%4 == 0){
	        pdwBuffer = (DWORD *)pBuffer;                               
	        while(cbDataLen > 3)                                           
	        {                                                           
	            OUTREG32(FIFO(epNum), *pdwBuffer++);     
	            cbDataLen -= 4;                                            
	        }                                                           
	        pBuffer = (UCHAR *)pdwBuffer;  
	    }    	                                                                                                                                                                                                             
	    while(cbDataLen > 0)                                           
	    {                                                           
	        OUTREG8(FIFO(epNum),*pBuffer++);   
	        cbDataLen -= 1;                                          
	    } 
	  }   
        
    //for serial download,always complete
    if(epNum == 0){
        wCSRToWrite = INREG16(IECSR + CSR0);
        wCSRToWrite |= (EP0_DATAEND|EP0_TXPKTRDY);
        OUTREG16(IECSR + CSR0, wCSRToWrite);
        printk("serial::USB_CSR0_TXPKTRDY and USB_CSR0_DATAEND set\r\n");
        //HW automatic handle,we can go to next        
    }else{
        wCSRToWrite = INREG16(IECSR + TXCSR);
        wCSRToWrite |= EPX_TX_TXPKTRDY;
    	  OUTREG16(IECSR + TXCSR, wCSRToWrite);
    } 
    return cbDataLen;     
}

DWORD S_RecvData(
    DWORD epNum,
    unsigned char* pBuffer,       //start buffer 
    DWORD pcbBufLen      //left
    )
{
    DWORD cbFIFO;	      //data number in FIFO
    DWORD cbLeftToRead;
    DWORD *pdwBuffer; 
    u16  wCSRToWrite;   

//    printk("Enter Function S_RecvData\r\n"); 
    OUTREG8(INDEX, epNum);

    //How many Data in FIFO	
    cbLeftToRead = cbFIFO = INREG16(IECSR + RXCOUNT);
    wCSRToWrite  = INREG16(IECSR + RXCSR);
    wCSRToWrite &= (~EPX_RX_CLRDATATOG);
     
    //Read Data    
    if(pBuffer!=NULL){    
	    if(((DWORD)pBuffer)%4 == 0)                  
	    {                                                                                     
	        pdwBuffer = (DWORD *)pBuffer;                               
	        while(cbLeftToRead > 3)                                           
	        {                                                           
	            *pdwBuffer++ = INREG32(FIFO(epNum));    
	            cbLeftToRead -= 4;                                            
	        }                                                           
	        pBuffer = (UCHAR *)pdwBuffer;    
	    }                                                                                                                           
	    while(cbLeftToRead > 0)                                           
	    {                                                           
	        *pBuffer++ = INREG8(FIFO(epNum));       
	        cbLeftToRead -= 1;                                          
	    } 
	  }
    
    //Clear RxPktRy               
    wCSRToWrite &= (~EPX_RX_RXPKTRDY);
    OUTREG16(IECSR + RXCSR, wCSRToWrite);	      
    return cbFIFO;	    	
}  


static bool S_SendDescriptor(struct usb_ctrlrequest* pUdr)
{
    UCHAR *pucData = NULL;
    WORD wLength = 0;
    WORD wType = pUdr->wValue;
    bool fRet = TRUE;
    unsigned short 	uCSR0;

    switch (wType >> 8) {
        case USB_DT_DEVICE:
            printk("serial::USB_DEVICE_DESCRIPTOR_TYPE.\r\n");  
            pucData = m_DeviceDescriptors;
            wLength = 0x12;
            break;

        case USB_DT_CONFIG:
        	  printk("serial::USB_CONFIGURATION_DESCRIPTOR_TYPE.\r\n");  
            pucData = m_ConfigDescriptors;
            //Length 9 or 32
            if (pUdr->wLength >= 32)
                wLength = 32;
            else
                wLength = 9;

            break;

        case USB_DT_STRING:
            printk("serial::USB_STRING_DESCRIPTOR_TYPE.\r\n");  
            fRet = FALSE; 
            break;
        default:
            printk("serial::default.\r\n");  
            fRet = FALSE;
            break;
    }

    OUTREG8(INDEX, 0); 
    //Clear RxPktRdy            
    uCSR0 = INREG16(IECSR+CSR0);
    uCSR0 &= (~EP0_SENTSTALL);
    if(uCSR0 & EP0_RXPKTRDY)
    {
    	  printk("serial::USB_CSR0_SERVICEDRXPKTRDY.\r\n");  
        uCSR0 |= EP0_SERVICED_RXPKTRDY;
    }
    OUTREG16(IECSR+CSR0, uCSR0);

    if (fRet){
        if(wLength>64){
            printk("serial::Warnnig,EP0 Send descriptor > 64 bytes\r\n");	 
        }
        if(pucData!=NULL){
        	printk("serial::S_SendData.\r\n");  
        	S_SendData(CONTROL_ENDPT,pucData,wLength);
        }
        else{
        	printk("serial::pucData NULL.\r\n");  
        }
    }    
    return TRUE;
}

void Ser_Handshake(void)
{	
    WORD  wToWrite = 0;
    OUTREG8(INDEX, 0);                      
    wToWrite = INREG16(IECSR + CSR0);
    wToWrite &= (~EP0_SENTSTALL);
    wToWrite |= (EP0_DATAEND | EP0_SERVICED_RXPKTRDY);		
    OUTREG16(IECSR + CSR0, wToWrite); 
	printk("(CSR) == %d \n",INREG16(IECSR + CSR0));
}

static void S_ProcessSetupPacket(struct usb_ctrlrequest* pUdr)
{
    
    switch(pUdr->bRequest) {
        case USB_REQ_GET_STATUS:
            printk("USB_REQUEST_GET_STATUS\r\n") ;
            if (pUdr->bRequestType == 0x80){
            		UCHAR DeviceStatus[2]={0,0};//bus-power and remote wakeup is disabled
                printk ("serial::USB_REQUEST_GET_STATUS_DEVICE\r\n");                
                S_SendData(CONTROL_ENDPT,DeviceStatus,2);
              }
              else if (pUdr->bRequestType == 0xC0){
                                printk("serial::USB_REQUEST_GET_STATUS_DEVICE 0xc0\r\n");                
              }
              else if (pUdr->bRequestType == 0x00){
                                printk("serial::USB_REQUEST_GET_STATUS_DEVICE 0x00\r\n");                
              }
                    
            break;

        case USB_REQ_CLEAR_FEATURE:
            printk("USB_REQUEST_CLEAR_FEATURE\r\n") ;
            if (pUdr->bRequestType == 0x02)
                printk("serial::USB_REQUEST_CLEAR_FEATURE\r\n");
            break;

        case USB_REQ_SET_FEATURE:
            printk("USB_REQUEST_SET_FEATURE\r\n") ;
            if (pUdr->bRequestType == 0x02)
                printk("serial::USB_REQUEST_SET_FEATURE\r\n");
            break;

        case USB_REQ_SET_ADDRESS:                        	        	
            printk("serial::USB_REQUEST_SET_ADDRESS.\r\n");  
            //Set Address issue
            //INREG16(&gs_pUSBRegs->IntrTx);
	          Ser_Handshake();	          
            OUTREG8(INDEX, 0);
			printk("host is ready set address %d \n",(BYTE)pUdr->wValue);
            while((INREG16(INTRTX)&0x1)==0)
                ;             
            OUTREG8(FADDR, (BYTE)pUdr->wValue);
			printk("address is %x\n",INREG8(FADDR));
            break;

        case USB_REQ_GET_DESCRIPTOR:
        	  printk("serial::USB_REQUEST_GET_DESCRIPTOR.\r\n");  
            S_SendDescriptor(pUdr);
            break;

        case USB_REQ_SET_DESCRIPTOR:
            printk("serial::USB_REQUEST_SET_DESCRIPTOR\r\n");
            break;

        case USB_REQ_GET_CONFIGURATION:
            printk("serial::USB_REQUEST_GET_CONFIGURATION\r\n");	
            break;

        case USB_REQ_SET_CONFIGURATION:
            Ser_Handshake();
            m_fConnected = TRUE;
            printk("serial:: Set Config received.\r\n");	
            break;

        case USB_REQ_GET_INTERFACE:
            printk ("serial::USB_REQUEST_GET_INTERFACE\r\n");	
            break;

        case USB_REQ_SET_INTERFACE:
           printk("serial::USB_REQUEST_SET_INTERFACE\r\n");	
            break;

        default:
            printk("serial::un-kown Request from PC\r\n");
    }    
}


/* -----------------------------------------------------
   Please Eboot Follow bestSize to Receive and Send Data
--------------------------------------------------------*/
bool 
Serial_Init(void )
{        
    UINT16   CommUsbIntr, uCSR0 ,uFIFO ;
    DWORD    udr[2];
    
    printk("serial:: Eboot call Serial_Init\r\n");	      
    // tell eboot transfer size,
    // init lower layer - HW config and attach to bus
    UsbSerial_Init();
	
    // wait PC set configuration request
    while(1){       
        //Polling Bus Event and EP0
        // 1:Bus Event
        CommUsbIntr = INREG8(INTRUSB);
	//	printk("INTRUSB = %x \n",CommUsbIntr);
	//	printk("INTRUSB is %x \n",INTRUSB);
/*        if(CommUsbIntr & INTRUSB_SUSPEND){
            printk("serial::Ser_ResetEP 0,1,2\r\n");
            Ser_ResetEP(0);
            Ser_ResetEP(1);
            Ser_ResetEP(2);
        }*/
        if(CommUsbIntr & INTRUSB_RESET){
       //    printk("serialaa::Ser_ResetDevice\r\n");
            Ser_ResetDevice();
			break;
        }
    }  
	//printk("power register is %x\n",INREG8(POWER));
		printk("INTRTXE == %x INTRRXE= %x\n",INREG16(INTRTXE),INREG16(INTRRXE));
	printk("INTRTX == %x INTRRX= %x\n",INREG16(INTRTX),INREG16(INTRRX));
        // 2:Wait EP0 Setup interrupt 
        while(1)
        	{
			if( (INREG16(INTRTX) & 0x1) != 0){	 
				printk("serial::(TxIntr & USB_INTRTX_EP0) != 0\r\n");	  
				OUTREG8(INDEX, 0);
				uCSR0 = INREG16(IECSR + CSR0);
				if(uCSR0 & EP0_RXPKTRDY){
					printk("serial::(uCSR0 & USB_CSR0_RXPKTRDY)\r\n");	   
					//we receive setup Packet,read it out
					uFIFO = INREG8(IECSR + COUNT0);
					if(uFIFO != 8){
						printk("serial::Error,setup Packet size!=8\r\n");
					}
					udr[0]=INREG32(FIFO(0));
					udr[1]=INREG32(FIFO(0));
					S_ProcessSetupPacket((struct usb_ctrlrequest*)udr);
				}
			}	   
			if(m_fConnected == TRUE){
				break;
			}

		}


    
    printk("serial:: Init success\r\n");    	
    return TRUE;	
}

//------------------------------------------------------------------------------
// Parameters:
//     (IN) pData: data to send
//     (IN) size:  number of bytes to send from pData
//
// Returns: Number of bytes sent
// 
UINT16
Serial_Send(
    UINT8 *pData,
    UINT16 size
    )
{
    //DBGMSG(1, ("serial::serial_send size=%d\r\n", size));        
    S_SendData(BULKIN_ENDPT,pData,size);
    //DBGMSG(1, ("serial::serial_send size=%d,return\r\n", size));
    return size;
}

/*-------------------------------------------
  Eboot can get the data when function return
---------------------------------------------*/
UINT16
Serial_Recv(
    UINT8 *pData,
    UINT16 size,
    UINT32 timeout
    )
{
    DWORD   cbRecvdData = 0;  
    WORD    wRxCSR = 0;
    EPTransfer* pTransfer = &m_epTransfers[BULKOUT_ENDPT];

//DWORD newtime;
		if(!pData){
			printk("serial::Error,pData == NULL\r\n");
			return 0;
		}
    //DBGMSG(1,("serial::Serial_Recv is called,size = %d\r\n",size));    
    //1: issue a transfer
    pTransfer->pbBuffer = &m_BulkRxPktBuf[0];
    pTransfer->cbTransferSize = size;
    if(size > MAX_BULKRX_PKT_SIZE){
        printk("serial::Recv size > MAX_BULKRX_PKT_SIZE\r\n");
    } 
    pTransfer->cbTransferred = 0;

    OUTREG8(INDEX, BULKOUT_ENDPT);  

	
    timeout *= 1000 ;
    while(  timeout -- ){
        //1:polling wait for RxPktRy
        wRxCSR = INREG16(IECSR + RXCSR);		
        if( (wRxCSR & EPX_RX_RXPKTRDY)==0 ){	
            continue;   	
		}	
		
        //2:read       	
        pTransfer->cbToTransfer = S_RecvData( //this function return data number we read
                    BULKOUT_ENDPT,
                    pTransfer->pbBuffer +  pTransfer->cbTransferred,        //buffer start
                    pTransfer->cbTransferSize -  pTransfer->cbTransferred); //Left size
        
        //3:update transfer
        pTransfer->cbTransferred += pTransfer->cbToTransfer;
        
        //4:if complete
        if ((pTransfer->cbTransferred >= pTransfer->cbTransferSize) ||
                   (pTransfer->cbToTransfer < gs_EpMaxSize[BULKOUT_ENDPT])){
            break;                 
        }
    }        
    
    cbRecvdData = pTransfer->cbTransferred;
    if(cbRecvdData!=size){

    }
    memcpy(pData, pTransfer->pbBuffer, cbRecvdData);   
    return (WORD)cbRecvdData;   
}

void Serial_DeInit(void)
{
  
}

int usb_init(void)
{
	Serial_Init();
	printk("connect success\n");
	return 0;
}

EXPORT_SYMBOL(Serial_Send);
EXPORT_SYMBOL(Serial_Recv);
EXPORT_SYMBOL(usb_init);
