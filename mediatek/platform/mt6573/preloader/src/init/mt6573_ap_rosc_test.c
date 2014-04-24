//#include <linux/module.h>
//#include <linux/kernel.h>
//#include <linux/init.h>

int rosc_cor_test(int sel)
{
    int i;
    int frm_rdata = 0;
    int frm_avg = 0;

    // **Configure Frequency Meters**
    switch(sel)
    {
      case 0: *((volatile unsigned int *)0x7002FF84) = 0x1A; break; //AP ROSC
      case 1: *((volatile unsigned int *)0x7002FF84) = 0x1B; break; //MD ROSC
      case 2: *((volatile unsigned int *)0x7002FF84) = 0x15; break; //EMI ROSC
      default: break;
    }

    frm_avg = 0;
    for (i = 0; i < 10; i++) {
      //Reset frequency meter
      *((volatile unsigned int *)0x7002FF80) = 0x4A28;
      while (0 != (*((volatile unsigned int *)0x7002FF88)));

      //Enable frequency meter
      *((volatile unsigned int *)0x7002FF80) = 0x8A28;

      //Wait busy bit asserted
      while (0 != ((*((volatile unsigned int *)0x7002FF84)) >> 15));

      //Wait busy bit de-asserted
      while (((*((volatile unsigned int *)0x7002FF84)) >> 15));

      //Read frequency meter value
      frm_rdata = *((volatile unsigned int *)0x7002FF88);
      //dbg_print("Frequency meter: 0x%x  \r\n", frm_rdata);

      //Accumulate results
      frm_avg = frm_avg + frm_rdata;
    }

    //Average value
    frm_avg = (frm_avg / 10) / 100;
    dbg_print("=> ROSC Frqeuncy: %d  \r\n", frm_avg);

    return 0;
}


int mt6573_rosc_characterization(void)
{
    // *****Configure AP_ROSC*****
    // Set Left_top_ROSC
    int i=0;
    dbg_print(" ************************* AP LEFT-TOP ROSC ****************************\r\n");
    dbg_print(" **** INV ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x00000;
    *((volatile unsigned int *)0x7002604C) = 0x00002;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(0);
    //} 
    dbg_print(" **** INV ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NAND 
    dbg_print(" **** NAND ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x10000;
    *((volatile unsigned int *)0x7002604C) = 0x10002;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(0);
    //} 
    dbg_print(" **** NAND ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NOR 
    dbg_print(" **** NOR ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x20000;
    *((volatile unsigned int *)0x7002604C) = 0x20002;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(0);
    //} 
    dbg_print(" **** NOR ROSC TEST FINISH****  \r\n\n");


    // Set Left_bottom_ROSC 
    dbg_print(" *********************** AP LEFT-BOTTOM ROSC ***************************\r\n");
    dbg_print(" **** INV ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x30000;
    *((volatile unsigned int *)0x7002604C) = 0x30002;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(0);
    //} 
    dbg_print(" **** INV ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NAND 
    dbg_print(" **** NAND ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x40000;
    *((volatile unsigned int *)0x7002604C) = 0x40002;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(0);
    //} 
    dbg_print(" **** NAND ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NOR 
    dbg_print(" **** NOR ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x50000;
    *((volatile unsigned int *)0x7002604C) = 0x50002;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(0);
    //} 
    dbg_print(" **** NOR ROSC TEST FINISH****  \r\n\n");


    // Set Right_bottom_ROSC 
    dbg_print(" *********************** AP RIGHT-BOTTOM ROSC **************************\r\n");
    dbg_print(" **** INV ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x60000;
    *((volatile unsigned int *)0x7002604C) = 0x60002;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(0);
    //} 
    dbg_print(" **** INV ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NAND 
    dbg_print(" **** NAND ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x70000;
    *((volatile unsigned int *)0x7002604C) = 0x70002;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(0);
    //} 
    dbg_print(" **** NAND ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NOR 
    dbg_print(" **** NOR ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x80000;
    *((volatile unsigned int *)0x7002604C) = 0x80002;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(0);
    //} 
    dbg_print(" **** NOR ROSC TEST FINISH****  \r\n\n");


    // Set center_ROSC 
    dbg_print(" *************************** AP CENTER ROSC ****************************\r\n");
    dbg_print(" **** INV ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x90000;
    *((volatile unsigned int *)0x7002604C) = 0x90002;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(0);
    //} 
    dbg_print(" **** INV ROSC TEST FINISH****  \r\n\n");



    // *****Configure MD_ROSC*****
    // Set Left_top_ROSC
    dbg_print(" ************************* MD LEFT-TOP ROSC ****************************\r\n");
    dbg_print(" **** INV ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x0000000;
    *((volatile unsigned int *)0x7002604C) = 0x0000004;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(1);
    //} 
    dbg_print(" **** INV ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NAND 
    dbg_print(" **** NAND ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x1000000;
    *((volatile unsigned int *)0x7002604C) = 0x1000004;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(1);
    //} 
    dbg_print(" **** NAND ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NOR 
    dbg_print(" **** NOR ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x2000000;
    *((volatile unsigned int *)0x7002604C) = 0x2000004;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(1);
    //} 
    dbg_print(" **** NOR ROSC TEST FINISH****  \r\n\n");


    // Set Left_bottom_ROSC 
    dbg_print(" *********************** MD LEFT-BOTTOM ROSC ***************************\r\n");
    dbg_print(" **** INV ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x3000000;
    *((volatile unsigned int *)0x7002604C) = 0x3000004;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(1);
    //} 
    dbg_print(" **** INV ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NAND 
    dbg_print(" **** NAND ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x4000000;
    *((volatile unsigned int *)0x7002604C) = 0x4000004;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(1);
    //} 
    dbg_print(" **** NAND ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NOR 
    dbg_print(" **** NOR ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x5000000;
    *((volatile unsigned int *)0x7002604C) = 0x5000004;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(1);
    //} 
    dbg_print(" **** NOR ROSC TEST FINISH****  \r\n\n");


    // Set Right_bottom_ROSC 
    dbg_print(" *********************** MD RIGHT-BOTTOM ROSC **************************\r\n");
    dbg_print(" **** INV ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x6000000;
    *((volatile unsigned int *)0x7002604C) = 0x6000004;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(1);
    //} 
    dbg_print(" **** INV ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NAND 
    dbg_print(" **** NAND ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x7000000;
    *((volatile unsigned int *)0x7002604C) = 0x7000004;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(1);
    //} 
    dbg_print(" **** NAND ROSC TEST FINISH****  \r\n\n");

    // Set Left_top_NOR 
    dbg_print(" **** NOR ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x8000000;
    *((volatile unsigned int *)0x7002604C) = 0x8000004;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(1);
    //} 
    dbg_print(" **** NOR ROSC TEST FINISH****  \r\n\n");


    // Set center_ROSC 
    dbg_print(" *************************** MD CENTER ROSC ****************************\r\n");
    dbg_print(" **** INV ROSC TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x9000000;
    *((volatile unsigned int *)0x7002604C) = 0x9000004;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(1);
    //} 
    dbg_print(" **** INV ROSC TEST FINISH****  \r\n\n");



    // *****Configure EMI_ROSC*****
    // Set O-RING
    dbg_print(" ************************* EMI ROSC ****************************\r\n");
    dbg_print(" **** O-RING128 TEST START****  \r\n");
    *((volatile unsigned int *)0x7002604C) = 0x000;
    *((volatile unsigned int *)0x7002604C) = 0x001;
    // **Read ROSC Value Under 1.2V~1.375V**
    //for (i=8; i<16; i++ )
    //{
      rosc_cor_test(2);
    //} 
    dbg_print(" **** O-RING128 TEST FINISH****  \r\n\n");


    //Disable ROSC
    *((volatile unsigned int *)0x7002604C) = 0x0;

    return 0;
}

/*
int __init mt6573_ap_rosc_test_mod_init(void)
{
    ap_rosc_characterization();

    return 0;
}

void __exit mt6573_ap_rosc_test_mod_exit(void)
{
}

module_init(mt6573_ap_rosc_test_mod_init);
module_exit(mt6573_ap_rosc_test_mod_exit);
*/
