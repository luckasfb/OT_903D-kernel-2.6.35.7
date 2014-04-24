
#include "tpd_custom_generic.h"
#include "tpd.h"
#include <linux/delay.h>

extern struct tpd_device *tpd;
extern int tpd_register_flag;
extern int tpd_em_debuglog;

int tpd_intr_status = 0;
int tpd_suspend_flag = 0;

int tpd_event_status = 0; // default up

static int raw_x, raw_y, pre_x = 0, pre_y = 0;
static int count_down=0;
static struct timeval d_time;
static struct timeval first_report;

/* internal function definition */
static void tpd_timer_fn(unsigned long);
static void tpd_tasklet(unsigned long unused);
irqreturn_t tpd_handler(int, void *);

static void tpd_down(int cx, int cy, int cd, int cp);
static void tpd_up(int cx, int cy, int cd, int cp);
extern void mt6573_irq_mask(unsigned int line);
extern void mt6573_irq_unmask(unsigned int line);

/* invoke by tpd_local_init, initialize r-type touchpanel */
int tpd_driver_local_init(void) {
    if(hwEnableClock(MT65XX_PDN_PERI_TP,"Touch")==FALSE)
        TPD_DMESG("hwEnableClock TP failed.");
    tpd_adc_init();
    init_timer(&(tpd->timer));
    tpd->timer.function = tpd_timer_fn;
    tasklet_init(&(tpd->tasklet), tpd_tasklet, 0);
    if(request_irq(MT6573_TOUCH_IRQ_LINE, tpd_handler, 0, "mtk_tpd", NULL))
        TPD_DMESG("request_irq failed.\n");
    return 0;
}

/* timer keep polling touch panel status */
void tpd_timer_fn(unsigned long arg) {
	if(tpd_em_debuglog == 1) {
		TPD_DMESG("tpd_timer_fn.\n");
	}
    TPD_EM_PRINT(0, 0, 0, 0, 0, TPD_TYPE_TIMER);
    if(tpd->tasklet.state != TASKLET_STATE_RUN)
        tasklet_hi_schedule(&(tpd->tasklet));
}

/* handle interrupt from touch panel (by dispatching to tasklet */
irqreturn_t tpd_handler(int irq, void *dev_id) {
	if(tpd_em_debuglog==1) {
		TPD_DMESG("tpd_handler.\n");
	}
    if (!tpd_event_status) {
        TPD_EM_PRINT(0, 0, 0, 0, 0, TPD_TYPE_INT_DOWN);
        tpd_event_status = 1;
    }
    else {
        TPD_EM_PRINT(0, 0, 0, 0, 0, TPD_TYPE_INT_UP);
        tpd_event_status = 0;
    }    
    tpd_intr_status = 1;
    if(tpd->tasklet.state != TASKLET_STATE_RUN)
        tasklet_hi_schedule(&(tpd->tasklet));
    return IRQ_HANDLED;
}

/**************************** touch down / up *******************************/
static void tpd_down(int cx, int cy, int cd, int cp) {
    /*
    #ifdef TPD_HAVE_BUTTON
    if (cy >= TPD_BUTTON_HEIGHT) {
        tpd_button(cx, cy, 1);
        return;
    }
    if (cy < TPD_BUTTON_HEIGHT && tpd->btn_state && cp < TPD_PRESSURE_MAX) 
        tpd_button(cx, cy, 0);
    #endif
    */
    int p;
    p = cp;
    cp = (255 * (TPD_PRESSURE_MAX - cp)) / (TPD_PRESSURE_MAX - TPD_PRESSURE_MIN);
//    if(cx<TPD_RES_X/100) cx=TPD_RES_X/100;
//    else if(cx>(TPD_RES_X-TPD_RES_X/100)) cx=TPD_RES_X-TPD_RES_X/100;
//    if(cy<TPD_RES_Y/100) cy=TPD_RES_Y/100;
//    else if(cy>(TPD_RES_Y-TPD_RES_Y/100)) cy=TPD_RES_Y-TPD_RES_Y/100;    
	if(tpd && tpd->dev && tpd_register_flag==1) {
    input_report_abs(tpd->dev, ABS_X, cx);
    input_report_abs(tpd->dev, ABS_Y, cy);
    input_report_abs(tpd->dev, ABS_PRESSURE, cp);
    input_report_key(tpd->dev, BTN_TOUCH, 1);
    input_sync(tpd->dev);
    
    TPD_DEBUG_PRINT_DOWN;
    TPD_EM_PRINT(raw_x, raw_y, cx, cy, p, 1);
  }  
    //printk(KERN_ERR "tpd, raw_x=%d, raw_y=%d, cx=%d, cy=%d, cp=%d, down\n", raw_x, raw_y, cx, cy, cp);
}
  
static void tpd_up(int cx, int cy, int cd, int cp) {
    int pending = 0;

    /*
    #ifdef TPD_HAVE_BUTTON
    if(tpd->btn_state) tpd_button(cx,cy,0);
    #endif
    */
    int p;
    p = cp;  
//    if(cx<TPD_RES_X/100) cx=TPD_RES_X/100;
//    else if(cx>(TPD_RES_X-TPD_RES_X/100)) cx=TPD_RES_X-TPD_RES_X/100;
//    if(cy<TPD_RES_Y/100) cy=TPD_RES_Y/100;
//    else if(cy>(TPD_RES_Y-TPD_RES_Y/100)) cy=TPD_RES_Y-TPD_RES_Y/100;    
	if(tpd && tpd->dev && tpd_register_flag==1) {
    input_report_abs(tpd->dev, ABS_X, cx);
    input_report_abs(tpd->dev, ABS_Y, cy);
    input_report_abs(tpd->dev, ABS_PRESSURE, 0);
    input_report_key(tpd->dev, BTN_TOUCH, 0);
    input_sync(tpd->dev);
    
    TPD_DEBUG_PRINT_UP;
    TPD_EM_PRINT(raw_x, raw_y, cx, cy, p, 0);
  }
    //printk(KERN_ERR "tpd, raw_x=%d, raw_y=%d, cx=%d, cy=%d, cp=%d, up\n", raw_x, raw_y, cx, cy, cp);
}

/*************************** main event handler ******************************/
void tpd_sampling(int *cx, int *cy, int *cp, int *cd) {
    int i, x = 0, y = 0, p = 0, rx = 0, ry = 0, rz1 = 0, rz2 = 0;
    
    /* ignore the 1st raw data */
    rx  = tpd_read(TPD_X);
    ry  = tpd_read(TPD_Y);
    rz1 = tpd_read(TPD_Z1);
    rz2 = tpd_read(TPD_Z2);  
            
    for (i = 0; i < 8; i++) {
        rx  = tpd_read(TPD_X);
        ry  = tpd_read(TPD_Y);
        rz1 = tpd_read(TPD_Z1);
        rz2 = tpd_read(TPD_Z2);
        if(tpd_em_debuglog == 1) {
        printk("raw[%5d %5d %5d %5d] \n", rx, ry, rz1, rz2);    
      }
        if (rx == 0 && ry == 4095) break;
                
        x += rx; y += ry;
        p = p + ((rx + 1) * (rz2 - rz1) / (rz1 + 1));
        
        TPD_EM_PRINT(rx, ry, rz1, rz2, ((rx+1)*(rz2-rz1)/(rz1+1)), TPD_TYPE_RAW_DATA);
        udelay(5);
    } 
    
    if (i == 0)
        x = 0, y = 0, p = 0;
    else     
        x /= i, y /= i, p /= i;
   
    raw_x = x; raw_y = y;
       
    if(!rx && ry == 4095) 
        *cd = 0;
    else 
        *cd = 1;
    *cx = x, *cy = y, *cp = p;
  	if(tpd_em_debuglog == 1){
    printk("cx = %d, cy = %d, cp = %d, cd = %d\n", *cx, *cy, *cp, *cd);
  }
}

/* handle touch panel interrupt for down / up event */
void tpd_tasklet(unsigned long unused) {
    int cx = 0, cy = 0, cp = 0, cd = 0;
    static int down = 0;
    static int latency_us = 0;
	if(tpd_em_debuglog==1) {
		TPD_DMESG("tpd_taslet, down=%d, count_down=%d.\n", down, count_down);
	}
    TPD_DEBUG_SET_TIME;

    if (tpd_em_debounce_time != 0) {
        tpd_set_debounce_time(tpd_em_debounce_time);
        tpd_em_debounce_time = 0;
    }

    if (tpd_em_spl_num != 0) {
        tpd_set_spl_number(tpd_em_spl_num);
        tpd_em_spl_num = 0;
    }
    
    tpd_sampling(&cx, &cy, &cp, &cd);
    
    // ignore unstable point from interrupt trigger
    if (tpd_intr_status == 1) {
	do_gettimeofday(&d_time);
        tpd_intr_status = 0;
        mod_timer(&(tpd->timer), jiffies + TPD_DELAY * 2);
       	TPD_EM_PRINT(0, 0, 0, 0, 0, TPD_TYPE_REJECT1);
        return;
    }

    // ignore unstable point - over the nice pressure value
    if(tpd_em_pressure_threshold >=TPD_PRESSURE_NICE) {
	    if (cp > tpd_em_pressure_threshold && cd != 0) {
	        mod_timer(&(tpd->timer), jiffies + TPD_DELAY * 2);
	        TPD_EM_PRINT(cp, tpd_em_pressure_threshold, 0, 0, 0, TPD_TYPE_REJECT2);
	        return;
	    }
    } else {
    if (cp > TPD_PRESSURE_NICE && cd != 0) {
        mod_timer(&(tpd->timer), jiffies + TPD_DELAY * 2);
        TPD_EM_PRINT(cp, TPD_PRESSURE_NICE, 0, 0, 0, TPD_TYPE_REJECT2);
        return;
	     }
    }
		if(tpd_suspend_flag==1){
			TPD_DMESG("tpd_suspend_flag==1.\n");
			return;    
		}
    
    if (cd == 1) {
        tpd_calibrate(&cx, &cy);
        
        if (cx < 0) cx = 0;
        if (cy < 0) cy = 0;
        if (cx > TPD_RES_X) cx = TPD_RES_X;
        if (cy > TPD_RES_Y) cy = TPD_RES_Y;
        if(tpd_em_debuglog==1) {
        printk("pre_x = %d, pre_y = %d, cx = %d, cy = %d\n", pre_x, pre_y, cx, cy);
      }
        
        if (pre_x != 0 || pre_y != 0) {
	    if(count_down==0) {
		do_gettimeofday(&first_report);
		latency_us =( (first_report.tv_sec & 0xFFF) * 1000000 + first_report.tv_usec)-( (d_time.tv_sec & 0xFFF) * 1000000 + d_time.tv_usec);
		TPD_EM_PRINT(latency_us, 0, 0, 0, 0, TPD_TYPE_FIST_LATENCY);
	    }
	    count_down ++;
            down = 1;
            cx = (pre_x + cx) / 2;
            cy = (pre_y + cy) / 2;
	    if(cy==TPD_RES_Y) cy=TPD_RES_Y-1;
            tpd_down(cx, cy, cd, cp);
        }
        
        pre_x = cx;
        pre_y = cy;        
        mod_timer(&(tpd->timer), jiffies + TPD_DELAY);
    } else {
        if (down) {
	    if(cy==TPD_RES_Y) cy=TPD_RES_Y-1;
            tpd_up(pre_x, pre_y, cd, cp);    
            down = 0;
	    count_down = 0;
        }
        pre_x = 0;
        pre_y = 0;
        if(tpd_event_status == 1) {
        	mod_timer(&(tpd->timer), jiffies + TPD_DELAY);
        }
    }
    
    return;
}

//#ifdef CONFIG_HAS_EARLYSUSPEND
void tpd_driver_suspend(struct early_suspend *h) {
    mt6573_irq_mask(MT6573_TOUCH_IRQ_LINE);
    if(hwDisableClock(MT65XX_PDN_PERI_TP,"Touch")==FALSE)
        TPD_DMESG("entering suspend mode - hwDisableClock failed.");
    tpd_intr_status = 0;
    tpd_suspend_flag = 1;
}
void tpd_driver_resume(struct early_suspend *h) {
    if(hwEnableClock(MT65XX_PDN_PERI_TP,"Touch")==FALSE)
        TPD_DMESG("resume from suspend mode - hwEnableClock TP failed.");
    mt6573_irq_unmask(MT6573_TOUCH_IRQ_LINE);
    tpd_suspend_flag = 0;
}
//#endif



