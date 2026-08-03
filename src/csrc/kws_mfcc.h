#ifndef _MFCC_DATA_H_
#define _MFCC_DATA_H_ 

#include "arm_math_types.h"


#ifdef   __cplusplus
extern "C"
{
#endif


/*****

 DCT COEFFICIENTS FOR THE MFCC

*****/


#define NB_MFCC_DCT_COEFS_KWS_F32 400
extern const float32_t mfcc_dct_coefs_kws_f32[NB_MFCC_DCT_COEFS_KWS_F32];



/*****

 WINDOW COEFFICIENTS

*****/


#define NB_MFCC_WIN_COEFS_KWS_F32 640
extern const float32_t mfcc_window_coefs_kws_f32[NB_MFCC_WIN_COEFS_KWS_F32];



/*****

 MEL FILTER COEFFICIENTS FOR THE MFCC

*****/

#define NB_MFCC_NB_FILTER_KWS_F32 40
extern const uint32_t mfcc_filter_pos_kws_f32[NB_MFCC_NB_FILTER_KWS_F32];
extern const uint32_t mfcc_filter_len_kws_f32[NB_MFCC_NB_FILTER_KWS_F32];





#define NB_MFCC_FILTER_COEFS_KWS_F32 493
extern const float32_t mfcc_filter_coefs_kws_f32[NB_MFCC_FILTER_COEFS_KWS_F32];



#ifdef   __cplusplus
}
#endif

#endif

