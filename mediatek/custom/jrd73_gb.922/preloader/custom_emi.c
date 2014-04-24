

#include "mt6573.h"
#include "mt6573_typedefs.h"
#include "mt6573_emi_reg.h"
#include "custom_emi.h"

#define NUM_EMI_RECORD 1

struct EMI_SETTINGS emi_settings[15]=
{
    
	//H9DA4GH2GJAMCR_4EM
	{
		0xADBC,		/* NAND ID */
		0x00324040,		/* EMI_CONI_VAL */
		0x88888888,		/* EMI_DRVA_VAL */
		0x00880000,		/* EMI_DRVB_VAL */
		0x0014134A,		/* EMI_CONJ_VAL */
		0x000C1000,		/* EMI_CONK_VAL */
		0x00414005,		/* EMI_CONL_VAL */
		0x03000000,		/* EMI_IOCL_VAL */
		0x0000030A,		/* EMI_GENA_VAL */
		0x1000e,		/* EMI_GEND_VAL */
		0x00000003,		/* EMI_DRCT_VAL */
		0xFFFF0000,		/* EMI_PPCT_VAL */
		0x0000037F,		/* EMI_SLCT_VAL */
		0x00070020,		/* EMI_ABCT_VAL */
		0x05050555,		/* EMI_DUTB_VAL */
		0x008B0000		/* EMI_CONN_VAL */
	}
};

extern int _EmiDataTrain( EMI_DATA_TRAIN_REG_t* pResult, int SDRAM_CS);


void Custom_EMI_InitDDR(unsigned short nand_id)
{
  UINT32 i, index, num_record, chose;
  EMI_DATA_TRAIN_REG_t DataTrainResultCS[4];

  num_record = sizeof(emi_settings) /sizeof(struct EMI_SETTINGS);		

  dbg_print("[EMI] num_record = %d\n", num_record);

  dbg_print("[EMI] NAND ID = %x\n", nand_id);
  
  for(i=0; i<num_record; i++)
  {
    dbg_print("[EMI] emi_settings[%d].NAND_ID = %x\n", i, emi_settings[i].NAND_ID);
    if(emi_settings[i].NAND_ID == nand_id)
    {
     index = i;
     break;
    }
  }
  
  if(i == num_record)
  {   
    dbg_print("[ERROR] Can not find specified EMI setting !!! (NAND_ID=0x%x)\n",nand_id);     
    while(1);          
  }

  
  DRV_WriteReg32(0x70026320, 0x00000000);
  DRV_WriteReg32(EMI_CONI, emi_settings[index].EMI_CONI_VAL);
  
  DRV_WriteReg32(EMI_DRVA, emi_settings[index].EMI_DRVA_VAL); //Need ETT result
  DRV_WriteReg32(EMI_DRVB, emi_settings[index].EMI_DRVB_VAL); //Need ETT result
  
  DRV_WriteReg32(EMI_CONJ, emi_settings[index].EMI_CONJ_VAL);
  DRV_WriteReg32(EMI_CONK, emi_settings[index].EMI_CONK_VAL);
  DRV_WriteReg32(EMI_CONL, emi_settings[index].EMI_CONL_VAL);
  DRV_WriteReg32(EMI_IOCL, emi_settings[index].EMI_IOCL_VAL); //Setup swap function for LPDDR EVB
  DRV_WriteReg32(EMI_GENA, emi_settings[index].EMI_GENA_VAL); //Enable clocks, pause-start signal, external boot
  DRV_WriteReg32(EMI_GEND, emi_settings[index].EMI_GEND_VAL); //Enable DDR CS0 and CS1
  
  DRV_WriteReg32(EMI_CONN, emi_settings[index].EMI_CONN_VAL);
  /* Initialization of DDR devices (steps followed per DDR device datasheet) */
  DRV_WriteReg32(EMI_CONN, (emi_settings[index].EMI_CONN_VAL | 0x10000001)); //Single Pre-charge All
  gpt4_busy_wait_ms (1);
  DRV_WriteReg32(EMI_CONN, (emi_settings[index].EMI_CONN_VAL | 0x08000001)); //Single Auto-refresh 1 Enable
  gpt4_busy_wait_ms (1);
  DRV_WriteReg32(EMI_CONN, (emi_settings[index].EMI_CONN_VAL | 0x04000001)); //Single Auto-refresh 2 Enable
  gpt4_busy_wait_ms (1);
  DRV_WriteReg32(EMI_CONN, (emi_settings[index].EMI_CONN_VAL | 0x02000001)); //Single Load Mode Register
  gpt4_busy_wait_ms (1);
  DRV_WriteReg32(EMI_CONN, (emi_settings[index].EMI_CONN_VAL | 0x01000001)); //Single Extended Mode Register Enable
  gpt4_busy_wait_ms (1);
  DRV_WriteReg32(EMI_CONN, (emi_settings[index].EMI_CONN_VAL | 0x00001117)); //Update EMI_CONN to enable PDN_EN, CAL_EN (data auto-tracking disabled), AP, and Concurrent AP
  gpt4_busy_wait_ms (1);

  DRV_WriteReg32(EMI_DUTB, emi_settings[index].EMI_DUTB_VAL); //This is to resolve EMI IO TX problem in LT_1.90_1.08 (Temp_Vmem_Vcore), and only in LPDDR1 case.

  dbg_print("[EMI] EMI_GEND = %x\n", DRV_Reg32(EMI_GEND));
  
  // process all DRAM bank 
  chose = DRV_Reg32(EMI_GEND) >> 16;
  
  *EMI_DQSE = 0x0; //Disable auto tracking function
  
  for(i=0; i<4; i++)
  {
    dbg_print("[EMI]choice = %x\n", chose);
    if((chose & 0x00000001) == 0)
    {
      chose = chose >> 1;
      continue;
    }
    
    if( _EmiDataTrain(&DataTrainResultCS[i], i) == 0)
    {
      dbg_print("[EMI]data training fail = %x\n", chose);
    }
    
    *EMI_CONN |= DataTrainResultCS[i].EMI_CONN_regval; //Apply DataTrain result for CSi
    *EMI_CONN |= CAL_EN;
  
    *EMI_DQSE |= DataTrainResultCS[i].EMI_DQSE_regval;
    
    *EMI_IDLA = DataTrainResultCS[i].EMI_IDLA_regval;
    *EMI_IDLB = DataTrainResultCS[i].EMI_IDLB_regval;
    *EMI_IDLC = DataTrainResultCS[i].EMI_IDLC_regval;
    *EMI_IDLD = DataTrainResultCS[i].EMI_IDLD_regval;
    *EMI_IDLE = DataTrainResultCS[i].EMI_IDLE_regval;
    *EMI_IDLF = DataTrainResultCS[i].EMI_IDLF_regval;
    *EMI_IDLG = DataTrainResultCS[i].EMI_IDLG_regval;
    *EMI_IDLH = DataTrainResultCS[i].EMI_IDLH_regval;
    
    *EMI_CALA = DataTrainResultCS[i].EMI_CALA_regval;
    *EMI_CALB = DataTrainResultCS[i].EMI_CALB_regval; 
    *EMI_CALE = DataTrainResultCS[i].EMI_CALE_regval;
    *EMI_CALF = DataTrainResultCS[i].EMI_CALF_regval; 
    *EMI_CALI = DataTrainResultCS[i].EMI_CALI_regval;
    *EMI_CALJ = DataTrainResultCS[i].EMI_CALJ_regval; 
    *EMI_CALP = DataTrainResultCS[i].EMI_CALP_regval;

    switch(i)
    {
      case 0:
        dbg_print("[EMI] Set up DQSA\n");
        *EMI_DQSA = DataTrainResultCS[i].EMI_DQSA_regval; //Apply DataTrain result for CS0
        *EMI_DQSE |= 0x000F;
        break;
      case 1:
        dbg_print("[EMI] Set up DQSB\n");
        *EMI_DQSB = DataTrainResultCS[i].EMI_DQSB_regval; //Apply DataTrain result for CS1
        *EMI_DQSE |= 0x00F0;
        break;
      case 2:  
        dbg_print("[EMI] Set up DQSC\n");
        *EMI_DQSC = DataTrainResultCS[i].EMI_DQSC_regval; //Apply DataTrain result for CS2
        *EMI_DQSE |= 0x0F00;
        break;
      case 3:  
        dbg_print("[EMI] Set up DQSD\n");
        *EMI_DQSD = DataTrainResultCS[i].EMI_DQSD_regval; //Apply DataTrain result for CS3
        *EMI_DQSE |= 0xF000;
        break;
      default:
        ;
    }
    
    *EMI_IDLI = 0x0; 
    
    chose = chose >> 1;
  }

  dbg_print("EMI_DLLV = %x\n", DRV_Reg32(EMI_DLLV));
  dbg_print("[EMI] EMI_CONN = %x\n", DRV_Reg32(EMI_CONN));

  DRV_WriteReg32(EMI_PPCT, emi_settings[index].EMI_PPCT_VAL); // Enable EMI_PPCT performance and power control
  DRV_WriteReg32(EMI_SLCT, emi_settings[index].EMI_SLCT_VAL); // EMI_SLCT - Enable R/W command favor for all masters
  DRV_WriteReg32(EMI_ABCT, emi_settings[index].EMI_ABCT_VAL);// Enable 1/32 freq for HWDCM mode and enable arbitration controls (lower_rw, higher_ph, lower_rc)
  gpt4_busy_wait_ms (100);
  
  DRV_WriteReg32(0x70026320, 0x00000000);
  DRV_WriteReg32(0x700FAFB0, 0xC5ACCE55);
  DRV_WriteReg32(0x700FA034, 0x00000001);
  

  if((DRV_Reg32(EMI_GEND)) == 0x2000D)  
  {
    dbg_print("EMI CS remapping... change cs0, c1\n");
    DRV_WriteReg32(EMI_GENA, 0x30B); //Enable clocks, pause-start signal, external boot 
    DRV_WriteReg32(EMI_DRCT, 0x1); //Enable clocks, pause-start signal, external boot 
  }
  else
  {
    DRV_WriteReg32(EMI_DRCT, emi_settings[index].EMI_DRCT_VAL); // Enable Dummy Read (required for HW DQS auto-tracking)
  }

}

void mt6573_set_emi ()
{
  unsigned short nand_id;
  
  if(NUM_EMI_RECORD <= 0)
  {
      dbg_print("[EMI] There is no EMI settings to initial EMI\n");	
      while(1);	
  }
  
  if((NUM_EMI_RECORD == 1) && (emi_settings[0].NAND_ID == 0x0))//Have no NAND
  {
    dbg_print("[EMI] Device without NAND\n");
    nand_id = 0x0;
  }
  else
  {
    getflashid(&nand_id);
  }
  
  dbg_print("[EMI] MT6573 EMI initialize\n");
  Custom_EMI_InitDDR (nand_id);
}




