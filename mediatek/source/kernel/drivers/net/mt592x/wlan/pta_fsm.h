





#ifndef _PTA_FSM_H
#define _PTA_FSM_H




/* BTCER1[30:29]=> 2'b00: 1-wire, 2'b01: 2-wire, 2'b10: 3-wire, 2'b11: 4-wire */
#define PTA_SW_WIRE_MODE        PTA_BTCER1_WIRE_MODE
#define PTA_SW_1_WIRE_MODE      0x00000000
#define PTA_SW_2_WIRE_MODE      BIT(29)
#define PTA_SW_3_WIRE_MODE      BIT(30)
#define PTA_SW_4_WIRE_MODE      (BIT(30) | BIT(29))


typedef enum _ENUM_PTA_STATE_T {
    PTA_STATE_IDLE = 0,
    PTA_STATE_ON,
    PTA_STATE_NUM
} ENUM_PTA_STATE_T;

typedef struct _PTA_INFO_T {
    ENUM_PTA_STATE_T    eCurrentState;
    UINT_32             u4PTAWireMode;      /* PTA_SW_1_WIRE_MODE and so on */
    BOOLEAN             fgSingleAntenna;
    BOOLEAN             fgEnabled;
    PTA_PROFILE_T       rBtProfile;
    PTA_PARAM_T	        rPtaParam;
    /* new board setting takes two sets of PTA settings */
#if PTA_NEW_BOARD_DESIGN
    PTA_PARAM_T	        rDualAclPtaParam;
    PTA_PARAM_T	        rDualMixPtaParam;
    PTA_PARAM_T	        rSingleAclPtaParam;
    PTA_PARAM_T	        rSingleMixPtaParam;
#else
    PTA_PARAM_T	        rScoPtaParam;
    PTA_PARAM_T	        rAclPtaParam;
    PTA_PARAM_T	        rMixPtaParam;
#endif
} PTA_INFO_T, *P_PTA_INFO_T;




#define PTA_STATE_TRANSITION(prAdapter, rFromState, rToState) \
            { /* ptaFsmTransAction_ ## rFromState ## _to_ ## rToState((P_ADAPTER_T)prAdapter);*/ \
              eNextState = PTA_STATE_ ## rToState; \
              DBGLOG(INIT, TRACE, (("PTA STATE TRANSITION: [%s] --> [%s]\n"), \
                                   #rFromState, #rToState)); \
            }



VOID
ptaFsmInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
ptaFsmRunEventSetConfig (
    IN P_ADAPTER_T          prAdapter,
    IN P_PTA_PARAM_T        prPtaParam
    );

#endif /* _PTA_FSM_H */


