
#define AUX_TP_DEBT           (AUXADC_BASE + 0x0050)
#define AUX_TP_CMD            (AUXADC_BASE + 0x0054)
#define AUX_TP_CON            (AUXADC_BASE + 0x0058)
#define AUX_TP_DATA0          (AUXADC_BASE + 0x005C)
#define AUXADC_CON3           (AUXADC_BASE + 0x000C)
#define AUXADC_CON3_STA_MASK  0x0001
#define TP_DEBT_MASK          0x3fff
#define TP_CMD_PD_MASK        0x0003
#define TP_CMD_PD_YDRV_SH     0x0000
#define TP_CMD_PD_IRQ_SH      0x0001
#define TP_CMD_PD_IRQ         0x0003
#define TP_CMD_SE_DF_MASK     0x0004
#define TP_CMD_DIFFERENTIAL   0x0000
#define TP_CMD_SINGLE_END     0x0004
#define TP_CMD_MODE_MASK      0x0008
#define TP_CMD_MODE_10BIT     0x0000
#define TP_CMD_MODE_8BIT      0x0008
#define TP_CMD_ADDR_MASK      0x0070
#define TP_CMD_ADDR_Y         0x0010
#define TP_CMD_ADDR_Z1        0x0030
#define TP_CMD_ADDR_Z2        0x0040
#define TP_CMD_ADDR_X         0x0050
#define TP_CON_SPL_MASK       0x0001
#define TP_CON_SPL_TRIGGER    0x0001
#define TP_CON_STATUS_MASK    0x0002
#define TP_DAT0_DAT_MASK      0x03ff
#define TP_DELAY              (2*HZ/100)
#define TP_DEBOUNCE_TIME      (1*32) /* 20ms */
#define TP_AUXADC_POWER_UP    0x0c000c00

#define caselog(v) case v: TPD_DEBUG("%s\n",#v);
#define TPD_CONTINUITY_TOLERANCE 3
#define RING_BUF_SIZE 8          // please use 2^x 
#define RING_BUF_SIZE_MINUS_1 7  // for convenience
#define TPD_BUF_CP(v) (v)
#define TPD_BUF_NP(v) ((v+1)&(RING_BUF_SIZE_MINUS_1))
#define TPD_BUF_PP(v) ((v+RING_BUF_SIZE_MINUS_1)&(RING_BUF_SIZE_MINUS_1))
#define TPD_BUF_NNP(v,n) ((v+n)&(RING_BUF_SIZE_MINUS_1))
#define TPD_BUF_PNP(v,n) (((v)+RING_BUF_SIZE-((n)&RING_BUF_SIZE_MINUS_1))&(RING_BUF_SIZE_MINUS_1))
#define TPD_BUF_QUEUE(v1,v2,v3,v4) do { \
 buf.x[RING_BUF_SIZE]=(v1),buf.y[RING_BUF_SIZE]=(v2),\
 buf.d[RING_BUF_SIZE]=(v3),buf.p[RING_BUF_SIZE]=(v4); buf.queued=1;}while(0);
#define TPD_BUF_UNQUEUE(v1,v2,v3,v4) do { \
 v1=buf.x[RING_BUF_SIZE],v2=buf.y[RING_BUF_SIZE],\
 v3=buf.d[RING_BUF_SIZE],v4=buf.p[RING_BUF_SIZE]; buf.queued=0;}while(0);

int tpd_sysfs(void);
void tpd_timer_fn(unsigned long);
void tpd_tasklet(unsigned long unused);
irqreturn_t tpd_interrupt_handler(int, void *);
