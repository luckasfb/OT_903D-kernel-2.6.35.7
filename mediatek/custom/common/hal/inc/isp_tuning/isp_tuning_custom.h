
#ifndef _ISP_TUNING_CUSTOM_H_
#define _ISP_TUNING_CUSTOM_H_


#include "camera_custom_nvram.h"


namespace NSIspTuning
{


class IspTuningCustom
{
protected:  ////    Ctor/Dtor.
    IspTuningCustom() {}
    virtual ~IspTuningCustom() {}

public:
    static IspTuningCustom* createInstance(ESensorRole_T const eSensorRole);
    virtual void destroyInstance() = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes
    virtual ESensorRole_T   getSensorRole() const = 0;
    virtual INDEX_T const*  getDefaultIndex(
                                ECamMode_T const eCamMode, 
                                EIndex_Scene_T const eIdx_Scene, 
                                EIndex_ISO_T const eIdx_ISO
                            ) const = 0;

public:     ////    Operations.

    virtual
    MBOOL
    is_to_invoke_offline_capture(
        RAWIspCamInfo const& rCamInfo
    ) const;

public:     ////    NVRAM

    virtual
    MVOID
    evaluate_nvram_index(
        RAWIspCamInfo const& rCamInfo, IndexMgr& rIdxMgr
    );

    virtual
    MVOID
    refine_NR1(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_NR1_T& rNR1
    );

    virtual
    MVOID
    refine_NR2(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_NR2_T& rNR2
    );

    virtual
    MVOID
    refine_DM(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_DEMOSAIC_T& rDM
    );

    virtual
    MVOID
    refine_EE(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_EE_T& rEE
    );

    virtual
    MVOID
    refine_CCM(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CCM_T& rCCM
    );

    virtual
    MVOID
    prepare_edge_gamma(ISP_NVRAM_EDGE_GAMMA_T& rEGamma);

public:     ////    Color Temperature Index: CCM, Shading

    virtual
    EIndex_CCM_CCT_T
    evaluate_CCM_CCT_index  (
        EIndex_CCM_CCT_T const eIdx_CCM_CCT_old, 
        MINT32 const i4CCT, 
        MINT32 const i4FluorescentIndex
    ) const;

    virtual
    EIndex_Shading_CCT_T
    evaluate_Shading_CCT_index  (
        EIndex_Shading_CCT_T const eIdx_Shading_CCT_old, 
        MINT32 const i4CCT
    ) const;

public:     ////    ISO

    virtual
    EIndex_ISO_T
    map_ISO_value_to_index(
        MUINT32 const u4Iso
    ) const;

public:     ////    Effect

    template <EIndex_Effect_T eEffect>
    MVOID
    prepare_effect(NS_MT6573ISP_EFFECT::ISP_EFFECT_T& rEffect);

public:     ////    End-User Setting Level.

    template <class ISP_NVRAM_xxx_T>
    MUINT32
    map_user_setting_to_nvram_index(
        MUINT8 const u8Idx_nvram_current, 
        IspUsrSelectLevel_T const& rUsr
    );

};


};  //  NSIspTuning
#endif //  _ISP_TUNING_CUSTOM_H_

