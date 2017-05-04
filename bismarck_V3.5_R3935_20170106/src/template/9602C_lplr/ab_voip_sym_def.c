#include <soc.h>
#include <util.h>
#define SGID_VOIP_DSP_FUNC 'V'
#define SCID_VOIP_ROM_GEN 0

#define VOIP_ADQ_FILTER_AND_ROTATE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 0, 0)
#define VOIP_BLOCK_PROCESS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 1, 0)
#define VOIP_BTR_ADJUST_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 2, 0)
#define VOIP_BIQUAD_FILTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 3, 0)
#define VOIP_BLOCK_DEMOD_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 4, 0)
#define VOIP_CN_PRM2BITS_LD8K_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 5, 0)
#define VOIP_CALCROTVALUES_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 6, 0)
#define VOIP_CHECK_ENERGY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 7, 0)
#define VOIP_COPYC_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 8, 0)
#define VOIP_COPYS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 9, 0)
#define VOIP_DEMOD_SAMPLE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 10, 0)
#define VOIP_DESCRIMINATE_AB_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 11, 0)
#define VOIP_DIV_32S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 12, 0)
#define VOIP_DO_BTR_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 13, 0)
#define VOIP_DO_IF_BANK_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 14, 0)
#define VOIP_FFRACSQRT_C_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 15, 0)
#define VOIP_FIR_FILTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 16, 0)
#define VOIP_FIR_FILTER_16_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 17, 0)
#define VOIP_FIR_FILTER_16_2_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 18, 0)
#define VOIP_FIR_FILTER_16_2S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 19, 0)
#define VOIP_FIR_FILTER_16S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 20, 0)
#define VOIP_FIR_FILTER_2_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 21, 0)
#define VOIP_FIR_FILTER_SHIFT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 22, 0)
#define VOIP_FIR_HILBERT_16_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 23, 0)
#define VOIP_FINDHILBERTCENTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 24, 0)
#define VOIP_GET_A_BIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 25, 0)
#define VOIP_GET_BITS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 26, 0)
#define VOIP_GET_COSINE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 27, 0)
#define VOIP_GET_DATA_BITS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 28, 0)
#define VOIP_GET_RANDOM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 29, 0)
#define VOIP_GET_SINE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 30, 0)
#define VOIP_HSTX_DO_SEGMENT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 31, 0)
#define VOIP_HSTX_ENCODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 32, 0)
#define VOIP_HSTX_GET_PHASE_TO_SEND_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 33, 0)
#define VOIP_HSTX_GET_RANDOM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 34, 0)
#define VOIP_HSTX_MODULATE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 35, 0)
#define VOIP_HSTX_SYMBOL_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 36, 0)
#define VOIP_INV_SQRT_C_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 37, 0)
#define VOIP_INV_SQRTS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 38, 0)
#define VOIP_L_COMPS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 39, 0)
#define VOIP_L_EXTRACTS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 40, 0)
#define VOIP_L_ABSS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 41, 0)
#define VOIP_L_ADDS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 42, 0)
#define VOIP_L_DEPOSIT_H_L_SHRS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 43, 0)
#define VOIP_L_MAC0S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 44, 0)
#define VOIP_L_MAC22S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 45, 0)
#define VOIP_L_MAC_LOOP1S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 46, 0)
#define VOIP_L_MAC_LOOP2S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 47, 0)
#define VOIP_L_MACS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 48, 0)
#define VOIP_L_MLSS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 49, 0)
#define VOIP_L_MSU0S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 50, 0)
#define VOIP_L_MSU_LOOPS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 51, 0)
#define VOIP_L_MSUS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 52, 0)
#define VOIP_L_MULT0S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 53, 0)
#define VOIP_L_MULTS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 54, 0)
#define VOIP_L_NEGATES_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 55, 0)
#define VOIP_L_SHL2_L_MAC_ROUNDS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 56, 0)
#define VOIP_L_SHL2_EXTRACT_HS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 57, 0)
#define VOIP_L_SHL2_ROUNDS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 58, 0)
#define VOIP_L_SHLS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 59, 0)
#define VOIP_L_SHR_RS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 60, 0)
#define VOIP_L_SHRS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 61, 0)
#define VOIP_L_SUBS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 62, 0)
#define VOIP_LOCATE_SAMPLING_TIME_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 63, 0)
#define VOIP_LOG2_C_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 64, 0)
#define VOIP_LOG2S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 65, 0)
#define VOIP_LOOK_FOR_RANDOM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 66, 0)
#define VOIP_MPY_32_16S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 67, 0)
#define VOIP_MPY_32S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 68, 0)
#define VOIP_PSF_CLEAR_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 69, 0)
#define VOIP_PSF_CLOSE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 70, 0)
#define VOIP_PSF_FILTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 71, 0)
#define VOIP_PSF_LOAD_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 72, 0)
#define VOIP_PSF_OPEN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 73, 0)
#define VOIP_PSF_SHIFT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 74, 0)
#define VOIP_PSF_V17_FILTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 75, 0)
#define VOIP_PSF_V17_OPEN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 76, 0)
#define VOIP_POW2S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 77, 0)
#define VOIP_PRE_PROCESS_RANDOM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 78, 0)
#define VOIP_PRE_PROCESS_RATE_SIGNAL_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 79, 0)
#define VOIP_PRE_PROCESS_SB1_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 80, 0)
#define VOIP_PRE_RECEIVE_IMAGE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 81, 0)
#define VOIP_PRE_RECEIVE_TRAIN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 82, 0)
#define VOIP_PRE_VITERBI_DELAY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 83, 0)
#define VOIP_PROCESS_BAUD_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 84, 0)
#define VOIP_PROCESS_BLOCK_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 85, 0)
#define VOIP_PROCESS_RANDOM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 86, 0)
#define VOIP_PROCESS_RATE_SIGNAL_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 87, 0)
#define VOIP_PROCESS_SB1_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 88, 0)
#define VOIP_PROCESS_SAMPLE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 89, 0)
#define VOIP_PUT_BITS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 90, 0)
#define VOIP_RECEIVE_IMAGE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 91, 0)
#define VOIP_RECEIVE_TRAIN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 92, 0)
#define VOIP_ROTATE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 93, 0)
#define VOIP_ROTATE_CARRIER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 94, 0)
#define VOIP_ROTATE_CARRIER_96_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 95, 0)
#define VOIP_ROTATES_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 96, 0)
#define VOIP_RX17_INIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 97, 0)
#define VOIP_RX_SEEK_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 98, 0)
#define VOIP_STM_GET_SAMPLE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 99, 0)
#define VOIP_STM_IS_EMPTY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 100, 0)
#define VOIP_STM_IS_FULL_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 101, 0)
#define VOIP_STM_PUT_DATA_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 102, 0)
#define VOIP_STM_PUT_SAMPLE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 103, 0)
#define VOIP_SET_ZERO_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 104, 0)
#define VOIP_SHIFT_ONLY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 105, 0)
#define VOIP_SHIFT_ONLY_2_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 106, 0)
#define VOIP_SHIFT_ONLY_2S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 107, 0)
#define VOIP_SHIFT_ONLYS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 108, 0)
#define VOIP_TX_ALTERNATE_SEG_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 109, 0)
#define VOIP_UPDATEDELAY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 110, 0)
#define VOIP_UPDATEDELAY_2_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 111, 0)
#define VOIP_UPDATE_ROT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 112, 0)
#define VOIP_UPDATE_AND_FILTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 113, 0)
#define VOIP_V17R_CLOSE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 114, 0)
#define VOIP_V17R_OPEN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 115, 0)
#define VOIP_V17R_PROCESS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 116, 0)
#define VOIP_V17X_BRIDGE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 117, 0)
#define VOIP_V17X_CLOSE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 118, 0)
#define VOIP_V17X_DO_SEGMENT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 119, 0)
#define VOIP_V17X_ENCODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 120, 0)
#define VOIP_V17X_GET_A_BIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 121, 0)
#define VOIP_V17X_GET_BITS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 122, 0)
#define VOIP_V17X_MODULATE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 123, 0)
#define VOIP_V17X_OPEN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 124, 0)
#define VOIP_V17X_PROCESS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 125, 0)
#define VOIP_V17X_PSEUDORANDOM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 126, 0)
#define VOIP_V17X_SCRAMBLED_ONES_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 127, 0)
#define VOIP_V17X_SCRAMBLED_ZEROS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 128, 0)
#define VOIP_V17X_SEND_IMAGE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 129, 0)
#define VOIP_V17X_SYMBOL_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 130, 0)
#define VOIP_V17X_ALTERNATE_SEG_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 131, 0)
#define VOIP_V17X_SEND_EVENT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 132, 0)
#define VOIP_V17_FOUNDBEGRANDLONG_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 133, 0)
#define VOIP_V17_FOUNDBEGRANDSHORT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 134, 0)
#define VOIP_V17_GET_BITS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 135, 0)
#define VOIP_V17_LOOK_FOR_RANDOM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 136, 0)
#define VOIP_V17_PSF_CLEAR_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 137, 0)
#define VOIP_V17_PSF_CLOSE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 138, 0)
#define VOIP_V17_PSF_LOAD_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 139, 0)
#define VOIP_V17_PSF_SHIFT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 140, 0)
#define VOIP_V17_PRE_PROCESS_RANDOM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 141, 0)
#define VOIP_V17_PRE_PROCESS_SB1_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 142, 0)
#define VOIP_V17_PRE_RECEIVE_IMAGE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 143, 0)
#define VOIP_V17_PRE_RECEIVE_TRAIN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 144, 0)
#define VOIP_V17_PROCESS_BAUD_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 145, 0)
#define VOIP_V17_PROCESS_BLOCK_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 146, 0)
#define VOIP_V17_PROCESS_RANDOM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 147, 0)
#define VOIP_V17_PROCESS_SB1_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 148, 0)
#define VOIP_V17_PROCESS_SAMPLE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 149, 0)
#define VOIP_V17_PUT_BITS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 150, 0)
#define VOIP_V17_RECEIVE_IMAGE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 151, 0)
#define VOIP_V17_RECEIVE_TRAIN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 152, 0)
#define VOIP_V17_RX_SEEK_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 153, 0)
#define VOIP_V17_UPDATE_AND_FILTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 154, 0)
#define VOIP_V17_ADEQ_COEFF_UPDATE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 155, 0)
#define VOIP_V17_BAUD_DELAY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 156, 0)
#define VOIP_V21R_CLOSE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 157, 0)
#define VOIP_V21R_OPEN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 158, 0)
#define VOIP_V21R_PROCESS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 159, 0)
#define VOIP_V21R_PUT_A_BIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 160, 0)
#define VOIP_V21X_CLOSE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 161, 0)
#define VOIP_V21X_OPEN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 162, 0)
#define VOIP_V21X_PROCESS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 163, 0)
#define VOIP_V21_DEMOD_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 164, 0)
#define VOIP_V21_DO_AGC_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 165, 0)
#define VOIP_V21_DO_CD_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 166, 0)
#define VOIP_V21_DO_SEGMENT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 167, 0)
#define VOIP_V21_GET_A_BIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 168, 0)
#define VOIP_V21_RECEIVE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 169, 0)
#define VOIP_V21_SEND_CED_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 170, 0)
#define VOIP_V21_SEND_EVENT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 171, 0)
#define VOIP_V21_SEND_SILENCE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 172, 0)
#define VOIP_V21_SEND_TONE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 173, 0)
#define VOIP_V21_TX_DATA_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 174, 0)
#define VOIP_V21_WAIT_DATA_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 175, 0)
#define VOIP_V21R_CNG_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 176, 0)
#define VOIP_V27R_RANDOM_CODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 177, 0)
#define VOIP_V29R_RANDOM_CODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 178, 0)
#define VOIP_V2XR_CLOSE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 179, 0)
#define VOIP_V2XR_DECIDE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 180, 0)
#define VOIP_V2XR_GET_RANDOM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 181, 0)
#define VOIP_V2XR_INIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 182, 0)
#define VOIP_V2XR_OPEN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 183, 0)
#define VOIP_V2XR_PROCESS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 184, 0)
#define VOIP_V2XX_CLOSE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 185, 0)
#define VOIP_V2XX_GET_BITS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 186, 0)
#define VOIP_V2XX_OPEN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 187, 0)
#define VOIP_V2XX_PROCESS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 188, 0)
#define VOIP_VITERBI_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 189, 0)
#define VOIP_VITERBI_DELAY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 190, 0)
#define VOIP_VITERBI_OPEN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 191, 0)
#define VOIP_XCNV_CLEAR_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 192, 0)
#define VOIP_XCNV_CLOSE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 193, 0)
#define VOIP_XCNV_FILTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 194, 0)
#define VOIP_XCNV_OPEN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 195, 0)
#define VOIP__XCNV_SHIFT_ONLY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 196, 0)
#define VOIP_ABS_SS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 197, 0)
#define VOIP_ADD_ADD_LOOPS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 198, 0)
#define VOIP_ADD_LOOP2S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 199, 0)
#define VOIP_ADD_LOOPS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 200, 0)
#define VOIP_ADDS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 201, 0)
#define VOIP_ADEQ_COEFF_UPDATE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 202, 0)
#define VOIP_ADEQ_FLTR_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 203, 0)
#define VOIP_ALAW2LINEAR_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 204, 0)
#define VOIP_APFILTERQ0_Q0_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 205, 0)
#define VOIP_APFILTERQ1_Q0_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 206, 0)
#define VOIP_AZFILTERQ0_Q1_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 207, 0)
#define VOIP_BAUD_DELAY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 208, 0)
#define VOIP_BIN2INTS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 209, 0)
#define VOIP_BITS2PRM_LD8K_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 210, 0)
#define VOIP_BITSTREAM_INIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 211, 0)
#define VOIP_BLOCK_FINDHILBERTCENTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 212, 0)
#define VOIP_BLOCK_DMD_SAMPLE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 213, 0)
#define VOIP_BLOCK_FIR_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 214, 0)
#define VOIP_BLOCK_FIRS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 215, 0)
#define VOIP_BLOCK_HLB_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 216, 0)
#define VOIP_BLOCK_HLBS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 217, 0)
#define VOIP_BLOCK_RX_FILTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 218, 0)
#define VOIP_BLOCK_UPDATE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 219, 0)
#define VOIP_BOTTOM_BIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 220, 0)
#define VOIP_CHECK_CARRIER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 221, 0)
#define VOIP_CLR_ALTERNATE_SEG_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 222, 0)
#define VOIP_COARSEPITCH_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 223, 0)
#define VOIP_COMPUTE_ENERGY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 224, 0)
#define VOIP_DATA_DECISION_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 225, 0)
#define VOIP_DECIDE_120_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 226, 0)
#define VOIP_DECIDE_144_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 227, 0)
#define VOIP_DECIDE_72_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 228, 0)
#define VOIP_DECIDE_96_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 229, 0)
#define VOIP_DECIM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 230, 0)
#define VOIP_DESCRAM2_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 231, 0)
#define VOIP_DESCRAMBLER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 232, 0)
#define VOIP_DIFF_DECODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 233, 0)
#define VOIP_DIV_LS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 234, 0)
#define VOIP_DIV_SS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 235, 0)
#define VOIP_DMD_SAMPLE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 236, 0)
#define VOIP_EXTRACTBUF_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 237, 0)
#define VOIP_FIR_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 238, 0)
#define VOIP_FIR_ASM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 239, 0)
#define VOIP_FIR_INIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 240, 0)
#define VOIP_FIR_RESET_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 241, 0)
#define VOIP_G726_16_DECODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 242, 0)
#define VOIP_G726_16_ENCODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 243, 0)
#define VOIP_G726_24_DECODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 244, 0)
#define VOIP_G726_24_ENCODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 245, 0)
#define VOIP_G726_32_DECODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 246, 0)
#define VOIP_G726_32_ENCODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 247, 0)
#define VOIP_G726_40_DECODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 248, 0)
#define VOIP_G726_40_ENCODER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 249, 0)
#define VOIP_G726_DMEN_MEMCPY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 250, 0)
#define VOIP_GETLAG_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 251, 0)
#define VOIP_IPLL2_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 252, 0)
#define VOIP_IPHASEESTIMATE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 253, 0)
#define VOIP_I_MULT_LOOPS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 254, 0)
#define VOIP_INT2BINS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 255, 0)
#define VOIP_INV_NORMS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 256, 0)
#define VOIP_LEC_ADAPT_ASM_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 257, 0)
#define VOIP_LINEAR2ALAW_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 258, 0)
#define VOIP_LINEAR2ULAW_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 259, 0)
#define VOIP_MAC_RS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 260, 0)
#define VOIP_MEMCPY16S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 261, 0)
#define VOIP_MEMCPY32S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 262, 0)
#define VOIP_MEMCPY64S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 263, 0)
#define VOIP_MEMMOVE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 264, 0)
#define VOIP_MEMSET32S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 265, 0)
#define VOIP_MEMSET64S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 266, 0)
#define VOIP_MERIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 267, 0)
#define VOIP_MSU_RS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 268, 0)
#define VOIP_MULT_R_LOOP1S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 269, 0)
#define VOIP_MULT_RS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 270, 0)
#define VOIP_MULTS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 271, 0)
#define VOIP_NEGATES_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 272, 0)
#define VOIP_NORM_LS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 273, 0)
#define VOIP_NORM_SS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 274, 0)
#define VOIP_NXCORS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 275, 0)
#define VOIP_PPCHANGE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 276, 0)
#define VOIP_PRFN_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 277, 0)
#define VOIP_PRINTK_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 278, 0)
#define VOIP_PRM2BITS_LD8K_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 279, 0)
#define VOIP_PRM2BYTES_LD8K_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 280, 0)
#define VOIP_PRM2BYTES_LD8K_ACTIVES_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 281, 0)
#define VOIP_PRM2BYTES_LD8K_SIDS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 282, 0)
#define VOIP_RAND_V27_SEG_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 283, 0)
#define VOIP_RAND_V29_SEG_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 284, 0)
#define VOIP_REFINELAG_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 285, 0)
#define VOIP_RESAMPLE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 286, 0)
#define VOIP_ROUNDS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 287, 0)
#define VOIP_RX_FILTER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 288, 0)
#define VOIP_SATURES_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 289, 0)
#define VOIP_SCRAMBLED_ONES_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 290, 0)
#define VOIP_SCRAMBLED_V27_FIRST_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 291, 0)
#define VOIP_SCRAMBLED_V29_FIRST_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 292, 0)
#define VOIP_SCRAMBLED_ZEROS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 293, 0)
#define VOIP_SEEK_DEMOD_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 294, 0)
#define VOIP_SEEK_STATE_0_FUNC_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 295, 0)
#define VOIP_SEEK_STATE_1_FUNC_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 296, 0)
#define VOIP_SEEK_STATE_2_FUNC_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 297, 0)
#define VOIP_SEEK_STATE_3_FUNC_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 298, 0)
#define VOIP_SEEK_STATE_4_FUNC_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 299, 0)
#define VOIP_SEEK_STATE_5_FUNC_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 300, 0)
#define VOIP_SEG23_DECISION_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 301, 0)
#define VOIP_SEND_EVENT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 302, 0)
#define VOIP_SEND_IMAGE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 303, 0)
#define VOIP_SET_AGC_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 304, 0)
#define VOIP_SET_MMD_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 305, 0)
#define VOIP_SHL_POSTIVE_LOOPS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 306, 0)
#define VOIP_SHLS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 307, 0)
#define VOIP_SHR_LOOP1S_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 308, 0)
#define VOIP_SHR_LOOPS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 309, 0)
#define VOIP_SHR_POSTIVE_LOOPS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 310, 0)
#define VOIP_SHRS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 311, 0)
#define VOIP_SUB_LOOPS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 312, 0)
#define VOIP_SUBS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 313, 0)
#define VOIP_TESTRPC_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 314, 0)
#define VOIP_TOP_BIT_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 315, 0)
#define VOIP_ULAW2LINEAR_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 316, 0)
#define VOIP_UPDATE_BTR_DELAY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 317, 0)
#define VOIP_UPDATE_IF_DELAY_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 318, 0)
#define VOIP_UPLOAD_EQ_COEFS_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 319, 0)
#define VOIP_UPSAMPLE_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 320, 0)
#define VOIP_V27_RX_SCRAMBLER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 321, 0)
#define VOIP_V27_TX_SCRAMBLER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 322, 0)
#define VOIP_V29_RX_SCRAMBLER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 323, 0)
#define VOIP_V29_TX_SCRAMBLER_FUNC symb_dot_key(SGID_VOIP_DSP_FUNC, SCID_VOIP_ROM_GEN, 324, 0)

extern void ADQ_Filter_and_Rotate(void);
extern void BLOCK_PROCESS(void);
extern void BTR_Adjust(void);
extern void Biquad_Filter(void);
extern void Block_Demod(void);
extern void CN_prm2bits_ld8k(void);
extern void CalcRotValues(void);
extern void Check_Energy(void);
extern void Copyc(void);
extern void Copys(void);
extern void Demod_Sample(void);
extern void Descriminate_AB(void);
extern void Div_32s(void);
extern void Do_BTR(void);
extern void Do_IF_Bank(void);
extern void FFracSqrt_c(void);
extern void FIR_Filter(void);
extern void FIR_Filter_16(void);
extern void FIR_Filter_16_2(void);
extern void FIR_Filter_16_2s(void);
extern void FIR_Filter_16s(void);
extern void FIR_Filter_2(void);
extern void FIR_Filter_Shift(void);
extern void FIR_Hilbert_16(void);
extern void FindHilbertCenter(void);
extern void Get_A_Bit(void);
extern void Get_Bits(void);
extern void Get_Cosine(void);
extern void Get_Data_Bits(void);
extern void Get_Random(void);
extern void Get_Sine(void);
extern void HSTX_Do_Segment(void);
extern void HSTX_Encoder(void);
extern void HSTX_Get_Phase_To_Send(void);
extern void HSTX_Get_Random(void);
extern void HSTX_Modulate(void);
extern void HSTX_Symbol(void);
extern void Inv_sqrt_c(void);
extern void Inv_sqrts(void);
extern void L_Comps(void);
extern void L_Extracts(void);
extern void L_abss(void);
extern void L_adds(void);
extern void L_deposit_h_L_shrs(void);
extern void L_mac0s(void);
extern void L_mac22s(void);
extern void L_mac_loop1s(void);
extern void L_mac_loop2s(void);
extern void L_macs(void);
extern void L_mlss(void);
extern void L_msu0s(void);
extern void L_msu_loops(void);
extern void L_msus(void);
extern void L_mult0s(void);
extern void L_mults(void);
extern void L_negates(void);
extern void L_shl2_L_mac_rounds(void);
extern void L_shl2_extract_hs(void);
extern void L_shl2_rounds(void);
extern void L_shls(void);
extern void L_shr_rs(void);
extern void L_shrs(void);
extern void L_subs(void);
extern void Locate_Sampling_Time(void);
extern void Log2_c(void);
extern void Log2s(void);
extern void Look_For_Random(void);
extern void Mpy_32_16s(void);
extern void Mpy_32s(void);
extern void PSF_Clear(void);
extern void PSF_Close(void);
extern void PSF_Filter(void);
extern void PSF_Load(void);
extern void PSF_Open(void);
extern void PSF_Shift(void);
extern void PSF_V17_Filter(void);
extern void PSF_V17_Open(void);
extern void Pow2s(void);
extern void Pre_Process_Random(void);
extern void Pre_Process_Rate_Signal(void);
extern void Pre_Process_SB1(void);
extern void Pre_Receive_Image(void);
extern void Pre_Receive_Train(void);
extern void Pre_Viterbi_Delay(void);
extern void Process_Baud(void);
extern void Process_Block(void);
extern void Process_Random(void);
extern void Process_Rate_Signal(void);
extern void Process_SB1(void);
extern void Process_Sample(void);
extern void Put_Bits(void);
extern void Receive_Image(void);
extern void Receive_Train(void);
extern void Rotate(void);
extern void Rotate_Carrier(void);
extern void Rotate_Carrier_96(void);
extern void Rotates(void);
extern void Rx17_Init(void);
extern void Rx_Seek(void);
extern void STM_Get_Sample(void);
extern void STM_Is_Empty(void);
extern void STM_Is_Full(void);
extern void STM_Put_Data(void);
extern void STM_Put_Sample(void);
extern void Set_zero(void);
extern void Shift_Only(void);
extern void Shift_Only_2(void);
extern void Shift_Only_2s(void);
extern void Shift_Onlys(void);
extern void TX_Alternate_seg(void);
extern void UpdateDelay(void);
extern void UpdateDelay_2(void);
extern void Update_Rot(void);
extern void Update_and_Filter(void);
extern void V17R_Close(void);
extern void V17R_Open(void);
extern void V17R_Process(void);
extern void V17X_Bridge(void);
extern void V17X_Close(void);
extern void V17X_Do_Segment(void);
extern void V17X_Encoder(void);
extern void V17X_Get_A_Bit(void);
extern void V17X_Get_Bits(void);
extern void V17X_Modulate(void);
extern void V17X_Open(void);
extern void V17X_Process(void);
extern void V17X_PseudoRandom(void);
extern void V17X_Scrambled_Ones(void);
extern void V17X_Scrambled_Zeros(void);
extern void V17X_Send_Image(void);
extern void V17X_Symbol(void);
extern void V17X_alternate_seg(void);
extern void V17X_send_event(void);
extern void V17_FoundBegRandLong(void);
extern void V17_FoundBegRandShort(void);
extern void V17_Get_Bits(void);
extern void V17_Look_For_Random(void);
extern void V17_PSF_Clear(void);
extern void V17_PSF_Close(void);
extern void V17_PSF_Load(void);
extern void V17_PSF_Shift(void);
extern void V17_Pre_Process_Random(void);
extern void V17_Pre_Process_SB1(void);
extern void V17_Pre_Receive_Image(void);
extern void V17_Pre_Receive_Train(void);
extern void V17_Process_Baud(void);
extern void V17_Process_Block(void);
extern void V17_Process_Random(void);
extern void V17_Process_SB1(void);
extern void V17_Process_Sample(void);
extern void V17_Put_Bits(void);
extern void V17_Receive_Image(void);
extern void V17_Receive_Train(void);
extern void V17_Rx_Seek(void);
extern void V17_Update_and_Filter(void);
extern void V17_adeq_coeff_update(void);
extern void V17_baud_delay(void);
extern void V21R_Close(void);
extern void V21R_Open(void);
extern void V21R_Process(void);
extern void V21R_Put_A_Bit(void);
extern void V21X_Close(void);
extern void V21X_Open(void);
extern void V21X_Process(void);
extern void V21_Demod(void);
extern void V21_Do_AGC(void);
extern void V21_Do_CD(void);
extern void V21_Do_Segment(void);
extern void V21_Get_A_Bit(void);
extern void V21_Receive(void);
extern void V21_Send_CED(void);
extern void V21_Send_Event(void);
extern void V21_Send_Silence(void);
extern void V21_Send_Tone(void);
extern void V21_Tx_Data(void);
extern void V21_Wait_Data(void);
extern void V21r_CNG(void);
extern void V27R_Random_Coder(void);
extern void V29R_Random_Coder(void);
extern void V2xR_Close(void);
extern void V2xR_Decide(void);
extern void V2xR_Get_Random(void);
extern void V2xR_Init(void);
extern void V2xR_Open(void);
extern void V2xR_Process(void);
extern void V2xX_Close(void);
extern void V2xX_Get_Bits(void);
extern void V2xX_Open(void);
extern void V2xX_Process(void);
extern void Viterbi(void);
extern void Viterbi_Delay(void);
extern void Viterbi_Open(void);
extern void XCNV_Clear(void);
extern void XCNV_Close(void);
extern void XCNV_Filter(void);
extern void XCNV_Open(void);
extern void _XCNV_Shift_Only(void);
extern void abs_ss(void);
extern void add_add_loops(void);
extern void add_loop2s(void);
extern void add_loops(void);
extern void adds(void);
extern void adeq_coeff_update(void);
extern void adeq_fltr(void);
extern void alaw2linear(void);
extern void apfilterQ0_Q0(void);
extern void apfilterQ1_Q0(void);
extern void azfilterQ0_Q1(void);
extern void baud_delay(void);
extern void bin2ints(void);
extern void bits2prm_ld8k(void);
extern void bitstream_init(void);
extern void block_FindHilbertCenter(void);
extern void block_dmd_sample(void);
extern void block_fir(void);
extern void block_firs(void);
extern void block_hlb(void);
extern void block_hlbs(void);
extern void block_rx_filter(void);
extern void block_update(void);
extern void bottom_bit(void);
extern void check_carrier(void);
extern void clr_alternate_seg(void);
extern void coarsepitch(void);
extern void compute_energy(void);
extern void data_decision(void);
extern void decide_120(void);
extern void decide_144(void);
extern void decide_72(void);
extern void decide_96(void);
extern void decim(void);
extern void descram2(void);
extern void descrambler(void);
extern void diff_decoder(void);
extern void div_ls(void);
extern void div_ss(void);
extern void dmd_sample(void);
extern void extractbuf(void);
extern void fir(void);
extern void fir_asm(void);
extern void fir_init(void);
extern void fir_reset(void);
extern void g726_16_decoder(void);
extern void g726_16_encoder(void);
extern void g726_24_decoder(void);
extern void g726_24_encoder(void);
extern void g726_32_decoder(void);
extern void g726_32_encoder(void);
extern void g726_40_decoder(void);
extern void g726_40_encoder(void);
extern void g726_dmen_memcpy(void);
extern void getlag(void);
extern void iPLL2(void);
extern void iPhaseEstimate(void);
extern void i_mult_loops(void);
extern void int2bins(void);
extern void inv_norms(void);
extern void lec_adapt_asm(void);
extern void linear2alaw(void);
extern void linear2ulaw(void);
extern void mac_rs(void);
extern void memcpy16s(void);
extern void memcpy32s(void);
extern void memcpy64s(void);
extern void memmove(void);
extern void memset32s(void);
extern void memset64s(void);
extern void merit(void);
extern void msu_rs(void);
extern void mult_r_loop1s(void);
extern void mult_rs(void);
extern void mults(void);
extern void negates(void);
extern void norm_ls(void);
extern void norm_ss(void);
extern void nxcors(void);
extern void ppchange(void);
extern void prfn(void);
extern void printk(void);
extern void prm2bits_ld8k(void);
extern void prm2bytes_ld8k(void);
extern void prm2bytes_ld8k_actives(void);
extern void prm2bytes_ld8k_sids(void);
extern void rand_v27_seg(void);
extern void rand_v29_seg(void);
extern void refinelag(void);
extern void resample(void);
extern void rounds(void);
extern void rx_filter(void);
extern void satures(void);
extern void scrambled_ones(void);
extern void scrambled_v27_first(void);
extern void scrambled_v29_first(void);
extern void scrambled_zeros(void);
extern void seek_demod(void);
extern void seek_state_0_func(void);
extern void seek_state_1_func(void);
extern void seek_state_2_func(void);
extern void seek_state_3_func(void);
extern void seek_state_4_func(void);
extern void seek_state_5_func(void);
extern void seg23_decision(void);
extern void send_event(void);
extern void send_image(void);
extern void set_agc(void);
extern void set_mmd(void);
extern void shl_postive_loops(void);
extern void shls(void);
extern void shr_loop1s(void);
extern void shr_loops(void);
extern void shr_postive_loops(void);
extern void shrs(void);
extern void sub_loops(void);
extern void subs(void);
extern void testrpc(void);
extern void top_bit(void);
extern void ulaw2linear(void);
extern void update_btr_delay(void);
extern void update_if_delay(void);
extern void upload_eq_coefs(void);
extern void upsample(void);
extern void v27_rx_scrambler(void);
extern void v27_tx_scrambler(void);
extern void v29_rx_scrambler(void);
extern void v29_tx_scrambler(void);

symb_fdefine(VOIP_ADQ_FILTER_AND_ROTATE_FUNC, ADQ_Filter_and_Rotate);
symb_fdefine(VOIP_BLOCK_PROCESS_FUNC, BLOCK_PROCESS);
symb_fdefine(VOIP_BTR_ADJUST_FUNC, BTR_Adjust);
symb_fdefine(VOIP_BIQUAD_FILTER_FUNC, Biquad_Filter);
symb_fdefine(VOIP_BLOCK_DEMOD_FUNC, Block_Demod);
symb_fdefine(VOIP_CN_PRM2BITS_LD8K_FUNC, CN_prm2bits_ld8k);
symb_fdefine(VOIP_CALCROTVALUES_FUNC, CalcRotValues);
symb_fdefine(VOIP_CHECK_ENERGY_FUNC, Check_Energy);
symb_fdefine(VOIP_COPYC_FUNC, Copyc);
symb_fdefine(VOIP_COPYS_FUNC, Copys);
symb_fdefine(VOIP_DEMOD_SAMPLE_FUNC, Demod_Sample);
symb_fdefine(VOIP_DESCRIMINATE_AB_FUNC, Descriminate_AB);
symb_fdefine(VOIP_DIV_32S_FUNC, Div_32s);
symb_fdefine(VOIP_DO_BTR_FUNC, Do_BTR);
symb_fdefine(VOIP_DO_IF_BANK_FUNC, Do_IF_Bank);
symb_fdefine(VOIP_FFRACSQRT_C_FUNC, FFracSqrt_c);
symb_fdefine(VOIP_FIR_FILTER_FUNC, FIR_Filter);
symb_fdefine(VOIP_FIR_FILTER_16_FUNC, FIR_Filter_16);
symb_fdefine(VOIP_FIR_FILTER_16_2_FUNC, FIR_Filter_16_2);
symb_fdefine(VOIP_FIR_FILTER_16_2S_FUNC, FIR_Filter_16_2s);
symb_fdefine(VOIP_FIR_FILTER_16S_FUNC, FIR_Filter_16s);
symb_fdefine(VOIP_FIR_FILTER_2_FUNC, FIR_Filter_2);
symb_fdefine(VOIP_FIR_FILTER_SHIFT_FUNC, FIR_Filter_Shift);
symb_fdefine(VOIP_FIR_HILBERT_16_FUNC, FIR_Hilbert_16);
symb_fdefine(VOIP_FINDHILBERTCENTER_FUNC, FindHilbertCenter);
symb_fdefine(VOIP_GET_A_BIT_FUNC, Get_A_Bit);
symb_fdefine(VOIP_GET_BITS_FUNC, Get_Bits);
symb_fdefine(VOIP_GET_COSINE_FUNC, Get_Cosine);
symb_fdefine(VOIP_GET_DATA_BITS_FUNC, Get_Data_Bits);
symb_fdefine(VOIP_GET_RANDOM_FUNC, Get_Random);
symb_fdefine(VOIP_GET_SINE_FUNC, Get_Sine);
symb_fdefine(VOIP_HSTX_DO_SEGMENT_FUNC, HSTX_Do_Segment);
symb_fdefine(VOIP_HSTX_ENCODER_FUNC, HSTX_Encoder);
symb_fdefine(VOIP_HSTX_GET_PHASE_TO_SEND_FUNC, HSTX_Get_Phase_To_Send);
symb_fdefine(VOIP_HSTX_GET_RANDOM_FUNC, HSTX_Get_Random);
symb_fdefine(VOIP_HSTX_MODULATE_FUNC, HSTX_Modulate);
symb_fdefine(VOIP_HSTX_SYMBOL_FUNC, HSTX_Symbol);
symb_fdefine(VOIP_INV_SQRT_C_FUNC, Inv_sqrt_c);
symb_fdefine(VOIP_INV_SQRTS_FUNC, Inv_sqrts);
symb_fdefine(VOIP_L_COMPS_FUNC, L_Comps);
symb_fdefine(VOIP_L_EXTRACTS_FUNC, L_Extracts);
symb_fdefine(VOIP_L_ABSS_FUNC, L_abss);
symb_fdefine(VOIP_L_ADDS_FUNC, L_adds);
symb_fdefine(VOIP_L_DEPOSIT_H_L_SHRS_FUNC, L_deposit_h_L_shrs);
symb_fdefine(VOIP_L_MAC0S_FUNC, L_mac0s);
symb_fdefine(VOIP_L_MAC22S_FUNC, L_mac22s);
symb_fdefine(VOIP_L_MAC_LOOP1S_FUNC, L_mac_loop1s);
symb_fdefine(VOIP_L_MAC_LOOP2S_FUNC, L_mac_loop2s);
symb_fdefine(VOIP_L_MACS_FUNC, L_macs);
symb_fdefine(VOIP_L_MLSS_FUNC, L_mlss);
symb_fdefine(VOIP_L_MSU0S_FUNC, L_msu0s);
symb_fdefine(VOIP_L_MSU_LOOPS_FUNC, L_msu_loops);
symb_fdefine(VOIP_L_MSUS_FUNC, L_msus);
symb_fdefine(VOIP_L_MULT0S_FUNC, L_mult0s);
symb_fdefine(VOIP_L_MULTS_FUNC, L_mults);
symb_fdefine(VOIP_L_NEGATES_FUNC, L_negates);
symb_fdefine(VOIP_L_SHL2_L_MAC_ROUNDS_FUNC, L_shl2_L_mac_rounds);
symb_fdefine(VOIP_L_SHL2_EXTRACT_HS_FUNC, L_shl2_extract_hs);
symb_fdefine(VOIP_L_SHL2_ROUNDS_FUNC, L_shl2_rounds);
symb_fdefine(VOIP_L_SHLS_FUNC, L_shls);
symb_fdefine(VOIP_L_SHR_RS_FUNC, L_shr_rs);
symb_fdefine(VOIP_L_SHRS_FUNC, L_shrs);
symb_fdefine(VOIP_L_SUBS_FUNC, L_subs);
symb_fdefine(VOIP_LOCATE_SAMPLING_TIME_FUNC, Locate_Sampling_Time);
symb_fdefine(VOIP_LOG2_C_FUNC, Log2_c);
symb_fdefine(VOIP_LOG2S_FUNC, Log2s);
symb_fdefine(VOIP_LOOK_FOR_RANDOM_FUNC, Look_For_Random);
symb_fdefine(VOIP_MPY_32_16S_FUNC, Mpy_32_16s);
symb_fdefine(VOIP_MPY_32S_FUNC, Mpy_32s);
symb_fdefine(VOIP_PSF_CLEAR_FUNC, PSF_Clear);
symb_fdefine(VOIP_PSF_CLOSE_FUNC, PSF_Close);
symb_fdefine(VOIP_PSF_FILTER_FUNC, PSF_Filter);
symb_fdefine(VOIP_PSF_LOAD_FUNC, PSF_Load);
symb_fdefine(VOIP_PSF_OPEN_FUNC, PSF_Open);
symb_fdefine(VOIP_PSF_SHIFT_FUNC, PSF_Shift);
symb_fdefine(VOIP_PSF_V17_FILTER_FUNC, PSF_V17_Filter);
symb_fdefine(VOIP_PSF_V17_OPEN_FUNC, PSF_V17_Open);
symb_fdefine(VOIP_POW2S_FUNC, Pow2s);
symb_fdefine(VOIP_PRE_PROCESS_RANDOM_FUNC, Pre_Process_Random);
symb_fdefine(VOIP_PRE_PROCESS_RATE_SIGNAL_FUNC, Pre_Process_Rate_Signal);
symb_fdefine(VOIP_PRE_PROCESS_SB1_FUNC, Pre_Process_SB1);
symb_fdefine(VOIP_PRE_RECEIVE_IMAGE_FUNC, Pre_Receive_Image);
symb_fdefine(VOIP_PRE_RECEIVE_TRAIN_FUNC, Pre_Receive_Train);
symb_fdefine(VOIP_PRE_VITERBI_DELAY_FUNC, Pre_Viterbi_Delay);
symb_fdefine(VOIP_PROCESS_BAUD_FUNC, Process_Baud);
symb_fdefine(VOIP_PROCESS_BLOCK_FUNC, Process_Block);
symb_fdefine(VOIP_PROCESS_RANDOM_FUNC, Process_Random);
symb_fdefine(VOIP_PROCESS_RATE_SIGNAL_FUNC, Process_Rate_Signal);
symb_fdefine(VOIP_PROCESS_SB1_FUNC, Process_SB1);
symb_fdefine(VOIP_PROCESS_SAMPLE_FUNC, Process_Sample);
symb_fdefine(VOIP_PUT_BITS_FUNC, Put_Bits);
symb_fdefine(VOIP_RECEIVE_IMAGE_FUNC, Receive_Image);
symb_fdefine(VOIP_RECEIVE_TRAIN_FUNC, Receive_Train);
symb_fdefine(VOIP_ROTATE_FUNC, Rotate);
symb_fdefine(VOIP_ROTATE_CARRIER_FUNC, Rotate_Carrier);
symb_fdefine(VOIP_ROTATE_CARRIER_96_FUNC, Rotate_Carrier_96);
symb_fdefine(VOIP_ROTATES_FUNC, Rotates);
symb_fdefine(VOIP_RX17_INIT_FUNC, Rx17_Init);
symb_fdefine(VOIP_RX_SEEK_FUNC, Rx_Seek);
symb_fdefine(VOIP_STM_GET_SAMPLE_FUNC, STM_Get_Sample);
symb_fdefine(VOIP_STM_IS_EMPTY_FUNC, STM_Is_Empty);
symb_fdefine(VOIP_STM_IS_FULL_FUNC, STM_Is_Full);
symb_fdefine(VOIP_STM_PUT_DATA_FUNC, STM_Put_Data);
symb_fdefine(VOIP_STM_PUT_SAMPLE_FUNC, STM_Put_Sample);
symb_fdefine(VOIP_SET_ZERO_FUNC, Set_zero);
symb_fdefine(VOIP_SHIFT_ONLY_FUNC, Shift_Only);
symb_fdefine(VOIP_SHIFT_ONLY_2_FUNC, Shift_Only_2);
symb_fdefine(VOIP_SHIFT_ONLY_2S_FUNC, Shift_Only_2s);
symb_fdefine(VOIP_SHIFT_ONLYS_FUNC, Shift_Onlys);
symb_fdefine(VOIP_TX_ALTERNATE_SEG_FUNC, TX_Alternate_seg);
symb_fdefine(VOIP_UPDATEDELAY_FUNC, UpdateDelay);
symb_fdefine(VOIP_UPDATEDELAY_2_FUNC, UpdateDelay_2);
symb_fdefine(VOIP_UPDATE_ROT_FUNC, Update_Rot);
symb_fdefine(VOIP_UPDATE_AND_FILTER_FUNC, Update_and_Filter);
symb_fdefine(VOIP_V17R_CLOSE_FUNC, V17R_Close);
symb_fdefine(VOIP_V17R_OPEN_FUNC, V17R_Open);
symb_fdefine(VOIP_V17R_PROCESS_FUNC, V17R_Process);
symb_fdefine(VOIP_V17X_BRIDGE_FUNC, V17X_Bridge);
symb_fdefine(VOIP_V17X_CLOSE_FUNC, V17X_Close);
symb_fdefine(VOIP_V17X_DO_SEGMENT_FUNC, V17X_Do_Segment);
symb_fdefine(VOIP_V17X_ENCODER_FUNC, V17X_Encoder);
symb_fdefine(VOIP_V17X_GET_A_BIT_FUNC, V17X_Get_A_Bit);
symb_fdefine(VOIP_V17X_GET_BITS_FUNC, V17X_Get_Bits);
symb_fdefine(VOIP_V17X_MODULATE_FUNC, V17X_Modulate);
symb_fdefine(VOIP_V17X_OPEN_FUNC, V17X_Open);
symb_fdefine(VOIP_V17X_PROCESS_FUNC, V17X_Process);
symb_fdefine(VOIP_V17X_PSEUDORANDOM_FUNC, V17X_PseudoRandom);
symb_fdefine(VOIP_V17X_SCRAMBLED_ONES_FUNC, V17X_Scrambled_Ones);
symb_fdefine(VOIP_V17X_SCRAMBLED_ZEROS_FUNC, V17X_Scrambled_Zeros);
symb_fdefine(VOIP_V17X_SEND_IMAGE_FUNC, V17X_Send_Image);
symb_fdefine(VOIP_V17X_SYMBOL_FUNC, V17X_Symbol);
symb_fdefine(VOIP_V17X_ALTERNATE_SEG_FUNC, V17X_alternate_seg);
symb_fdefine(VOIP_V17X_SEND_EVENT_FUNC, V17X_send_event);
symb_fdefine(VOIP_V17_FOUNDBEGRANDLONG_FUNC, V17_FoundBegRandLong);
symb_fdefine(VOIP_V17_FOUNDBEGRANDSHORT_FUNC, V17_FoundBegRandShort);
symb_fdefine(VOIP_V17_GET_BITS_FUNC, V17_Get_Bits);
symb_fdefine(VOIP_V17_LOOK_FOR_RANDOM_FUNC, V17_Look_For_Random);
symb_fdefine(VOIP_V17_PSF_CLEAR_FUNC, V17_PSF_Clear);
symb_fdefine(VOIP_V17_PSF_CLOSE_FUNC, V17_PSF_Close);
symb_fdefine(VOIP_V17_PSF_LOAD_FUNC, V17_PSF_Load);
symb_fdefine(VOIP_V17_PSF_SHIFT_FUNC, V17_PSF_Shift);
symb_fdefine(VOIP_V17_PRE_PROCESS_RANDOM_FUNC, V17_Pre_Process_Random);
symb_fdefine(VOIP_V17_PRE_PROCESS_SB1_FUNC, V17_Pre_Process_SB1);
symb_fdefine(VOIP_V17_PRE_RECEIVE_IMAGE_FUNC, V17_Pre_Receive_Image);
symb_fdefine(VOIP_V17_PRE_RECEIVE_TRAIN_FUNC, V17_Pre_Receive_Train);
symb_fdefine(VOIP_V17_PROCESS_BAUD_FUNC, V17_Process_Baud);
symb_fdefine(VOIP_V17_PROCESS_BLOCK_FUNC, V17_Process_Block);
symb_fdefine(VOIP_V17_PROCESS_RANDOM_FUNC, V17_Process_Random);
symb_fdefine(VOIP_V17_PROCESS_SB1_FUNC, V17_Process_SB1);
symb_fdefine(VOIP_V17_PROCESS_SAMPLE_FUNC, V17_Process_Sample);
symb_fdefine(VOIP_V17_PUT_BITS_FUNC, V17_Put_Bits);
symb_fdefine(VOIP_V17_RECEIVE_IMAGE_FUNC, V17_Receive_Image);
symb_fdefine(VOIP_V17_RECEIVE_TRAIN_FUNC, V17_Receive_Train);
symb_fdefine(VOIP_V17_RX_SEEK_FUNC, V17_Rx_Seek);
symb_fdefine(VOIP_V17_UPDATE_AND_FILTER_FUNC, V17_Update_and_Filter);
symb_fdefine(VOIP_V17_ADEQ_COEFF_UPDATE_FUNC, V17_adeq_coeff_update);
symb_fdefine(VOIP_V17_BAUD_DELAY_FUNC, V17_baud_delay);
symb_fdefine(VOIP_V21R_CLOSE_FUNC, V21R_Close);
symb_fdefine(VOIP_V21R_OPEN_FUNC, V21R_Open);
symb_fdefine(VOIP_V21R_PROCESS_FUNC, V21R_Process);
symb_fdefine(VOIP_V21R_PUT_A_BIT_FUNC, V21R_Put_A_Bit);
symb_fdefine(VOIP_V21X_CLOSE_FUNC, V21X_Close);
symb_fdefine(VOIP_V21X_OPEN_FUNC, V21X_Open);
symb_fdefine(VOIP_V21X_PROCESS_FUNC, V21X_Process);
symb_fdefine(VOIP_V21_DEMOD_FUNC, V21_Demod);
symb_fdefine(VOIP_V21_DO_AGC_FUNC, V21_Do_AGC);
symb_fdefine(VOIP_V21_DO_CD_FUNC, V21_Do_CD);
symb_fdefine(VOIP_V21_DO_SEGMENT_FUNC, V21_Do_Segment);
symb_fdefine(VOIP_V21_GET_A_BIT_FUNC, V21_Get_A_Bit);
symb_fdefine(VOIP_V21_RECEIVE_FUNC, V21_Receive);
symb_fdefine(VOIP_V21_SEND_CED_FUNC, V21_Send_CED);
symb_fdefine(VOIP_V21_SEND_EVENT_FUNC, V21_Send_Event);
symb_fdefine(VOIP_V21_SEND_SILENCE_FUNC, V21_Send_Silence);
symb_fdefine(VOIP_V21_SEND_TONE_FUNC, V21_Send_Tone);
symb_fdefine(VOIP_V21_TX_DATA_FUNC, V21_Tx_Data);
symb_fdefine(VOIP_V21_WAIT_DATA_FUNC, V21_Wait_Data);
symb_fdefine(VOIP_V21R_CNG_FUNC, V21r_CNG);
symb_fdefine(VOIP_V27R_RANDOM_CODER_FUNC, V27R_Random_Coder);
symb_fdefine(VOIP_V29R_RANDOM_CODER_FUNC, V29R_Random_Coder);
symb_fdefine(VOIP_V2XR_CLOSE_FUNC, V2xR_Close);
symb_fdefine(VOIP_V2XR_DECIDE_FUNC, V2xR_Decide);
symb_fdefine(VOIP_V2XR_GET_RANDOM_FUNC, V2xR_Get_Random);
symb_fdefine(VOIP_V2XR_INIT_FUNC, V2xR_Init);
symb_fdefine(VOIP_V2XR_OPEN_FUNC, V2xR_Open);
symb_fdefine(VOIP_V2XR_PROCESS_FUNC, V2xR_Process);
symb_fdefine(VOIP_V2XX_CLOSE_FUNC, V2xX_Close);
symb_fdefine(VOIP_V2XX_GET_BITS_FUNC, V2xX_Get_Bits);
symb_fdefine(VOIP_V2XX_OPEN_FUNC, V2xX_Open);
symb_fdefine(VOIP_V2XX_PROCESS_FUNC, V2xX_Process);
symb_fdefine(VOIP_VITERBI_FUNC, Viterbi);
symb_fdefine(VOIP_VITERBI_DELAY_FUNC, Viterbi_Delay);
symb_fdefine(VOIP_VITERBI_OPEN_FUNC, Viterbi_Open);
symb_fdefine(VOIP_XCNV_CLEAR_FUNC, XCNV_Clear);
symb_fdefine(VOIP_XCNV_CLOSE_FUNC, XCNV_Close);
symb_fdefine(VOIP_XCNV_FILTER_FUNC, XCNV_Filter);
symb_fdefine(VOIP_XCNV_OPEN_FUNC, XCNV_Open);
symb_fdefine(VOIP__XCNV_SHIFT_ONLY_FUNC, _XCNV_Shift_Only);
symb_fdefine(VOIP_ABS_SS_FUNC, abs_ss);
symb_fdefine(VOIP_ADD_ADD_LOOPS_FUNC, add_add_loops);
symb_fdefine(VOIP_ADD_LOOP2S_FUNC, add_loop2s);
symb_fdefine(VOIP_ADD_LOOPS_FUNC, add_loops);
symb_fdefine(VOIP_ADDS_FUNC, adds);
symb_fdefine(VOIP_ADEQ_COEFF_UPDATE_FUNC, adeq_coeff_update);
symb_fdefine(VOIP_ADEQ_FLTR_FUNC, adeq_fltr);
symb_fdefine(VOIP_ALAW2LINEAR_FUNC, alaw2linear);
symb_fdefine(VOIP_APFILTERQ0_Q0_FUNC, apfilterQ0_Q0);
symb_fdefine(VOIP_APFILTERQ1_Q0_FUNC, apfilterQ1_Q0);
symb_fdefine(VOIP_AZFILTERQ0_Q1_FUNC, azfilterQ0_Q1);
symb_fdefine(VOIP_BAUD_DELAY_FUNC, baud_delay);
symb_fdefine(VOIP_BIN2INTS_FUNC, bin2ints);
symb_fdefine(VOIP_BITS2PRM_LD8K_FUNC, bits2prm_ld8k);
symb_fdefine(VOIP_BITSTREAM_INIT_FUNC, bitstream_init);
symb_fdefine(VOIP_BLOCK_FINDHILBERTCENTER_FUNC, block_FindHilbertCenter);
symb_fdefine(VOIP_BLOCK_DMD_SAMPLE_FUNC, block_dmd_sample);
symb_fdefine(VOIP_BLOCK_FIR_FUNC, block_fir);
symb_fdefine(VOIP_BLOCK_FIRS_FUNC, block_firs);
symb_fdefine(VOIP_BLOCK_HLB_FUNC, block_hlb);
symb_fdefine(VOIP_BLOCK_HLBS_FUNC, block_hlbs);
symb_fdefine(VOIP_BLOCK_RX_FILTER_FUNC, block_rx_filter);
symb_fdefine(VOIP_BLOCK_UPDATE_FUNC, block_update);
symb_fdefine(VOIP_BOTTOM_BIT_FUNC, bottom_bit);
symb_fdefine(VOIP_CHECK_CARRIER_FUNC, check_carrier);
symb_fdefine(VOIP_CLR_ALTERNATE_SEG_FUNC, clr_alternate_seg);
symb_fdefine(VOIP_COARSEPITCH_FUNC, coarsepitch);
symb_fdefine(VOIP_COMPUTE_ENERGY_FUNC, compute_energy);
symb_fdefine(VOIP_DATA_DECISION_FUNC, data_decision);
symb_fdefine(VOIP_DECIDE_120_FUNC, decide_120);
symb_fdefine(VOIP_DECIDE_144_FUNC, decide_144);
symb_fdefine(VOIP_DECIDE_72_FUNC, decide_72);
symb_fdefine(VOIP_DECIDE_96_FUNC, decide_96);
symb_fdefine(VOIP_DECIM_FUNC, decim);
symb_fdefine(VOIP_DESCRAM2_FUNC, descram2);
symb_fdefine(VOIP_DESCRAMBLER_FUNC, descrambler);
symb_fdefine(VOIP_DIFF_DECODER_FUNC, diff_decoder);
symb_fdefine(VOIP_DIV_LS_FUNC, div_ls);
symb_fdefine(VOIP_DIV_SS_FUNC, div_ss);
symb_fdefine(VOIP_DMD_SAMPLE_FUNC, dmd_sample);
symb_fdefine(VOIP_EXTRACTBUF_FUNC, extractbuf);
symb_fdefine(VOIP_FIR_FUNC, fir);
symb_fdefine(VOIP_FIR_ASM_FUNC, fir_asm);
symb_fdefine(VOIP_FIR_INIT_FUNC, fir_init);
symb_fdefine(VOIP_FIR_RESET_FUNC, fir_reset);
symb_fdefine(VOIP_G726_16_DECODER_FUNC, g726_16_decoder);
symb_fdefine(VOIP_G726_16_ENCODER_FUNC, g726_16_encoder);
symb_fdefine(VOIP_G726_24_DECODER_FUNC, g726_24_decoder);
symb_fdefine(VOIP_G726_24_ENCODER_FUNC, g726_24_encoder);
symb_fdefine(VOIP_G726_32_DECODER_FUNC, g726_32_decoder);
symb_fdefine(VOIP_G726_32_ENCODER_FUNC, g726_32_encoder);
symb_fdefine(VOIP_G726_40_DECODER_FUNC, g726_40_decoder);
symb_fdefine(VOIP_G726_40_ENCODER_FUNC, g726_40_encoder);
symb_fdefine(VOIP_G726_DMEN_MEMCPY_FUNC, g726_dmen_memcpy);
symb_fdefine(VOIP_GETLAG_FUNC, getlag);
symb_fdefine(VOIP_IPLL2_FUNC, iPLL2);
symb_fdefine(VOIP_IPHASEESTIMATE_FUNC, iPhaseEstimate);
symb_fdefine(VOIP_I_MULT_LOOPS_FUNC, i_mult_loops);
symb_fdefine(VOIP_INT2BINS_FUNC, int2bins);
symb_fdefine(VOIP_INV_NORMS_FUNC, inv_norms);
symb_fdefine(VOIP_LEC_ADAPT_ASM_FUNC, lec_adapt_asm);
symb_fdefine(VOIP_LINEAR2ALAW_FUNC, linear2alaw);
symb_fdefine(VOIP_LINEAR2ULAW_FUNC, linear2ulaw);
symb_fdefine(VOIP_MAC_RS_FUNC, mac_rs);
symb_fdefine(VOIP_MEMCPY16S_FUNC, memcpy16s);
symb_fdefine(VOIP_MEMCPY32S_FUNC, memcpy32s);
symb_fdefine(VOIP_MEMCPY64S_FUNC, memcpy64s);
symb_fdefine(VOIP_MEMMOVE_FUNC, memmove);
symb_fdefine(VOIP_MEMSET32S_FUNC, memset32s);
symb_fdefine(VOIP_MEMSET64S_FUNC, memset64s);
symb_fdefine(VOIP_MERIT_FUNC, merit);
symb_fdefine(VOIP_MSU_RS_FUNC, msu_rs);
symb_fdefine(VOIP_MULT_R_LOOP1S_FUNC, mult_r_loop1s);
symb_fdefine(VOIP_MULT_RS_FUNC, mult_rs);
symb_fdefine(VOIP_MULTS_FUNC, mults);
symb_fdefine(VOIP_NEGATES_FUNC, negates);
symb_fdefine(VOIP_NORM_LS_FUNC, norm_ls);
symb_fdefine(VOIP_NORM_SS_FUNC, norm_ss);
symb_fdefine(VOIP_NXCORS_FUNC, nxcors);
symb_fdefine(VOIP_PPCHANGE_FUNC, ppchange);
symb_fdefine(VOIP_PRFN_FUNC, prfn);
symb_fdefine(VOIP_PRINTK_FUNC, printk);
symb_fdefine(VOIP_PRM2BITS_LD8K_FUNC, prm2bits_ld8k);
symb_fdefine(VOIP_PRM2BYTES_LD8K_FUNC, prm2bytes_ld8k);
symb_fdefine(VOIP_PRM2BYTES_LD8K_ACTIVES_FUNC, prm2bytes_ld8k_actives);
symb_fdefine(VOIP_PRM2BYTES_LD8K_SIDS_FUNC, prm2bytes_ld8k_sids);
symb_fdefine(VOIP_RAND_V27_SEG_FUNC, rand_v27_seg);
symb_fdefine(VOIP_RAND_V29_SEG_FUNC, rand_v29_seg);
symb_fdefine(VOIP_REFINELAG_FUNC, refinelag);
symb_fdefine(VOIP_RESAMPLE_FUNC, resample);
symb_fdefine(VOIP_ROUNDS_FUNC, rounds);
symb_fdefine(VOIP_RX_FILTER_FUNC, rx_filter);
symb_fdefine(VOIP_SATURES_FUNC, satures);
symb_fdefine(VOIP_SCRAMBLED_ONES_FUNC, scrambled_ones);
symb_fdefine(VOIP_SCRAMBLED_V27_FIRST_FUNC, scrambled_v27_first);
symb_fdefine(VOIP_SCRAMBLED_V29_FIRST_FUNC, scrambled_v29_first);
symb_fdefine(VOIP_SCRAMBLED_ZEROS_FUNC, scrambled_zeros);
symb_fdefine(VOIP_SEEK_DEMOD_FUNC, seek_demod);
symb_fdefine(VOIP_SEEK_STATE_0_FUNC_FUNC, seek_state_0_func);
symb_fdefine(VOIP_SEEK_STATE_1_FUNC_FUNC, seek_state_1_func);
symb_fdefine(VOIP_SEEK_STATE_2_FUNC_FUNC, seek_state_2_func);
symb_fdefine(VOIP_SEEK_STATE_3_FUNC_FUNC, seek_state_3_func);
symb_fdefine(VOIP_SEEK_STATE_4_FUNC_FUNC, seek_state_4_func);
symb_fdefine(VOIP_SEEK_STATE_5_FUNC_FUNC, seek_state_5_func);
symb_fdefine(VOIP_SEG23_DECISION_FUNC, seg23_decision);
symb_fdefine(VOIP_SEND_EVENT_FUNC, send_event);
symb_fdefine(VOIP_SEND_IMAGE_FUNC, send_image);
symb_fdefine(VOIP_SET_AGC_FUNC, set_agc);
symb_fdefine(VOIP_SET_MMD_FUNC, set_mmd);
symb_fdefine(VOIP_SHL_POSTIVE_LOOPS_FUNC, shl_postive_loops);
symb_fdefine(VOIP_SHLS_FUNC, shls);
symb_fdefine(VOIP_SHR_LOOP1S_FUNC, shr_loop1s);
symb_fdefine(VOIP_SHR_LOOPS_FUNC, shr_loops);
symb_fdefine(VOIP_SHR_POSTIVE_LOOPS_FUNC, shr_postive_loops);
symb_fdefine(VOIP_SHRS_FUNC, shrs);
symb_fdefine(VOIP_SUB_LOOPS_FUNC, sub_loops);
symb_fdefine(VOIP_SUBS_FUNC, subs);
symb_fdefine(VOIP_TESTRPC_FUNC, testrpc);
symb_fdefine(VOIP_TOP_BIT_FUNC, top_bit);
symb_fdefine(VOIP_ULAW2LINEAR_FUNC, ulaw2linear);
symb_fdefine(VOIP_UPDATE_BTR_DELAY_FUNC, update_btr_delay);
symb_fdefine(VOIP_UPDATE_IF_DELAY_FUNC, update_if_delay);
symb_fdefine(VOIP_UPLOAD_EQ_COEFS_FUNC, upload_eq_coefs);
symb_fdefine(VOIP_UPSAMPLE_FUNC, upsample);
symb_fdefine(VOIP_V27_RX_SCRAMBLER_FUNC, v27_rx_scrambler);
symb_fdefine(VOIP_V27_TX_SCRAMBLER_FUNC, v27_tx_scrambler);
symb_fdefine(VOIP_V29_RX_SCRAMBLER_FUNC, v29_rx_scrambler);
symb_fdefine(VOIP_V29_TX_SCRAMBLER_FUNC, v29_tx_scrambler);

