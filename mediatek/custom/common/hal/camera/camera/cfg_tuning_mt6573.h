
#ifndef _CFG_TUNING_MT6573_H_
#define _CFG_TUNING_MT6573_H_

MUINT32	DoFrontCamFlip()
{
	MUINT32 flip = 0; // 1: enable, 0: disable
	return flip; 
}

SensorOrientation_T const&
getSensorOrientation()
{
    static SensorOrientation_T const inst = {
        u4Degree_0  : 90,   //  main sensor in degree (0, 90, 180, 270)
        u4Degree_1  : 270,    //  sub  sensor in degree (0, 90, 180, 270)
    };
    return inst;
}


TuningParam_CRZ_T const&
getParam_CRZ_Video()
{
    static TuningParam_CRZ_T const inst = {
        uUpScaleCoeff   : 8,    //  [5 bits; 1~19] Up sample coeff. choose > 12 may get undesirable result, '8' is recommended.
        uDnScaleCoeff   : 15,   //  [5 bits; 1~19] Down sample coeff. '1':blur,'19':sharpest; '15' is recommended.
    };
    return inst;
}

TuningParam_CRZ_T const&
getParam_CRZ_Preview()
{
    static TuningParam_CRZ_T const inst = {
        uUpScaleCoeff   : 8,    //  [5 bits; 1~19] Up sample coeff. choose > 12 may get undesirable result, '8' is recommended.
        uDnScaleCoeff   : 15,   //  [5 bits; 1~19] Down sample coeff. '1':blur,'19':sharpest; '15' is recommended.
    };
    return inst;
}

TuningParam_CRZ_T const&
getParam_CRZ_Capture()
{
    static TuningParam_CRZ_T const inst = {
        uUpScaleCoeff   : 8,    //  [5 bits; 1~19] Up sample coeff. choose > 12 may get undesirable result, '8' is recommended.
        uDnScaleCoeff   : 15,   //  [5 bits; 1~19] Down sample coeff. '1':blur,'19':sharpest; '15' is recommended.
    };
    return inst;
}

TuningParam_PRZ_T const&
getParam_PRZ_QuickView()
{
    static TuningParam_PRZ_T const inst = {
        uUpScaleCoeff   : 8,    //  [5 bits; 1~19] Up sample coeff. choose > 12 may get undesirable result, '8' is recommended.
        uDnScaleCoeff   : 15,   //  [5 bits; 1~19] Down sample coeff. '1':blur,'19':sharpest; '15' is recommended.
        uEEHCoeff       : 0,    //  [4 bits] The strength for horizontal edge.
        uEEVCoeff       : 0,    //  [4 bits] The strength for vertial edge.
    };
    return inst;
}

MINT32 get_atv_input_data()
{
	MINT32 AtvInputdata = 1;
	return AtvInputdata;
}



MINT8 get_fdvt_threshold()  
{
	MINT8 FeatureThreshold = 7;    
	return FeatureThreshold;
}


#endif //  _CFG_TUNING_MT6573_H_

