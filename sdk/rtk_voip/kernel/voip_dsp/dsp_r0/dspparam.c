//
// dspparam.c
//
#ifdef PLAYTONE_UNITTEST
#include "playtone_unittest.h"
#endif
#include "dspparam.h"
//20121031 support 32 cadence tone, add cad4~cad31 (280items)
/* FORMAT :(45+280 items)

   ToneType,
   Cycle, 
   CadNUM or RptNUM, 
   CadOn[0], 
   CadOff[0], 
   
   CadOn[1], 
   CadOff[1], 
   
   CadOn[2],
   CadOff[2], 
   
   CadOn[3], 
   CadOff[3], 
   
   PatternOff, 
   ToneNUM, 
   Freq0, 
   Freq1, 
   Freq2, 
   Freq3, 
   Gain0,
   Gain1, 
   Gain2, 
   Gain3,
   C1_Freq0, 
   C1_Freq1, 
   C1_Freq2, 
   C1_Freq3, 
   C1_Gain0,
   C1_Gain1, 
   C1_Gain2, 
   C1_Gain3,
   C2_Freq0, 
   C2_Freq1, 
   C2_Freq2, 
   C2_Freq3, 
   C2_Gain0,
   C2_Gain1, 
   C2_Gain2, 
   C2_Gain3,
   C3_Freq0, 
   C3_Freq1, 
   C3_Freq2, 
   C3_Freq3, 
   C3_Gain0,
   C3_Gain1, 
   C3_Gain2, 
   C3_Gain3,
   CadOn[4], 
   CadOff[4], 
   CadOn[5], 
   CadOff[5], 
   CadOn[6], 
   CadOff[6], 
   CadOn[7], 
   CadOff[7], 
   C4_Freq0, 
   C4_Freq1, 
   C4_Freq2, 
   C4_Freq3, 
   C4_Gain0,
   C4_Gain1, 
   C4_Gain2, 
   C4_Gain3,
   C5_Freq0, 
   C5_Freq1, 
   C5_Freq2, 
   C5_Freq3, 
   C5_Gain0,
   C5_Gain1, 
   C5_Gain2, 
   C5_Gain3,
   C6_Freq0, 
   C6_Freq1, 
   C6_Freq2, 
   C6_Freq3, 
   C6_Gain0,
   C6_Gain1, 
   C6_Gain2, 
   C6_Gain3,
   C7_Freq0, 
   C7_Freq1, 
   C7_Freq2, 
   C7_Freq3, 
   C7_Gain0,
   C7_Gain1, 
   C7_Gain2, 
   C7_Gain3,
   CadOn[8], 
   CadOff[8], 
   CadOn[9], 
   CadOff[9], 
   CadOn[10], 
   CadOff[10], 
   CadOn[11], 
   CadOff[11], 
   C8_Freq0, 
   C8_Freq1, 
   C8_Freq2, 
   C8_Freq3, 
   C8_Gain0,
   C8_Gain1, 
   C8_Gain2, 
   C8_Gain3,
   C9_Freq0, 
   C9_Freq1, 
   C9_Freq2, 
   C9_Freq3, 
   C9_Gain0,
   C9_Gain1, 
   C9_Gain2, 
   C9_Gain3,
   C10_Freq0, 
   C10_Freq1, 
   C10_Freq2, 
   C10_Freq3, 
   C10_Gain0,
   C10_Gain1, 
   C10_Gain2, 
   C10_Gain3,
   C11_Freq0, 
   C11_Freq1, 
   C11_Freq2, 
   C11_Freq3, 
   C11_Gain0,
   C11_Gain1, 
   C11_Gain2, 
   C11_Gain3,
   CadOn[12], 
   CadOff[12], 
   CadOn[13], 
   CadOff[13], 
   CadOn[14], 
   CadOff[14], 
   CadOn[15], 
   CadOff[15], 
   C12_Freq0, 
   C12_Freq1, 
   C12_Freq2, 
   C12_Freq3, 
   C12_Gain0,
   C12_Gain1, 
   C12_Gain2, 
   C12_Gain3,
   C13_Freq0, 
   C13_Freq1, 
   C13_Freq2, 
   C13_Freq3, 
   C13_Gain0,
   C13_Gain1, 
   C13_Gain2, 
   C13_Gain3,
   C14_Freq0, 
   C14_Freq1, 
   C14_Freq2, 
   C14_Freq3, 
   C14_Gain0,
   C14_Gain1, 
   C14_Gain2, 
   C14_Gain3,
   C15_Freq0, 
   C15_Freq1, 
   C15_Freq2, 
   C15_Freq3, 
   C15_Gain0,
   C15_Gain1, 
   C15_Gain2, 
   C15_Gain3,
   CadOn[16], 
   CadOff[16], 
   CadOn[17], 
   CadOff[17], 
   CadOn[18], 
   CadOff[18], 
   CadOn[19], 
   CadOff[19], 
   C16_Freq0, 
   C16_Freq1, 
   C16_Freq2, 
   C16_Freq3, 
   C16_Gain0,
   C16_Gain1, 
   C16_Gain2, 
   C16_Gain3,
   C17_Freq0, 
   C17_Freq1, 
   C17_Freq2, 
   C17_Freq3, 
   C17_Gain0,
   C17_Gain1, 
   C17_Gain2, 
   C17_Gain3,
   C18_Freq0, 
   C18_Freq1, 
   C18_Freq2, 
   C18_Freq3, 
   C18_Gain0,
   C18_Gain1, 
   C18_Gain2, 
   C18_Gain3,
   C19_Freq0, 
   C19_Freq1, 
   C19_Freq2, 
   C19_Freq3, 
   C19_Gain0,
   C19_Gain1, 
   C19_Gain2, 
   C19_Gain3,
   CadOn[20], 
   CadOff[20], 
   CadOn[21], 
   CadOff[21], 
   CadOn[22], 
   CadOff[22], 
   CadOn[23], 
   CadOff[23], 
   C20_Freq0, 
   C20_Freq1, 
   C20_Freq2, 
   C20_Freq3, 
   C20_Gain0,
   C20_Gain1, 
   C20_Gain2, 
   C20_Gain3,
   C21_Freq0, 
   C21_Freq1, 
   C21_Freq2, 
   C21_Freq3, 
   C21_Gain0,
   C21_Gain1, 
   C21_Gain2, 
   C21_Gain3,
   C22_Freq0, 
   C22_Freq1, 
   C22_Freq2, 
   C22_Freq3, 
   C22_Gain0,
   C22_Gain1, 
   C22_Gain2, 
   C22_Gain3,
   C23_Freq0, 
   C23_Freq1, 
   C23_Freq2, 
   C23_Freq3, 
   C23_Gain0,
   C23_Gain1, 
   C23_Gain2, 
   C23_Gain3,
   CadOn[24], 
   CadOff[24], 
   CadOn[25], 
   CadOff[25], 
   CadOn[26], 
   CadOff[26], 
   CadOn[27], 
   CadOff[27], 
   C24_Freq0, 
   C24_Freq1, 
   C24_Freq2, 
   C24_Freq3, 
   C24_Gain0,
   C24_Gain1, 
   C24_Gain2, 
   C24_Gain3,
   C25_Freq0, 
   C25_Freq1, 
   C25_Freq2, 
   C25_Freq3, 
   C25_Gain0,
   C25_Gain1, 
   C25_Gain2, 
   C25_Gain3,
   C26_Freq0, 
   C26_Freq1, 
   C26_Freq2, 
   C26_Freq3, 
   C26_Gain0,
   C26_Gain1, 
   C26_Gain2, 
   C26_Gain3,
   C27_Freq0, 
   C27_Freq1, 
   C27_Freq2, 
   C27_Freq3, 
   C27_Gain0,
   C27_Gain1, 
   C27_Gain2, 
   C27_Gain3,
   CadOn[28], 
   CadOff[28], 
   CadOn[29], 
   CadOff[29], 
   CadOn[30], 
   CadOff[30], 
   CadOn[31], 
   CadOff[31], 
   C28_Freq0, 
   C28_Freq1, 
   C28_Freq2, 
   C28_Freq3, 
   C28_Gain0,
   C28_Gain1, 
   C28_Gain2, 
   C28_Gain3,
   C29_Freq0, 
   C29_Freq1, 
   C29_Freq2, 
   C29_Freq3, 
   C29_Gain0,
   C29_Gain1, 
   C29_Gain2, 
   C29_Gain3,
   C30_Freq0, 
   C30_Freq1, 
   C30_Freq2, 
   C30_Freq3, 
   C30_Gain0,
   C30_Gain1, 
   C30_Gain2, 
   C30_Gain3,
   C31_Freq0, 
   C31_Freq1, 
   C31_Freq2, 
   C31_Freq3, 
   C31_Gain0,
   C31_Gain1, 
   C31_Gain2, 
   C31_Gain3,
   
   NOTE. ToneType : {ADDITIVE, MODULATED, SUCC, SUCC_ADD, FOUR_FREQ, STEP_INC, TWO_STEP}
   		 Cycle : < 0 illegal value
   		 		 0 represent "continuous tone"
   		 		 > 0 cycle number
   		 CadNUM(RptNUM) : Cadence number (in SUCC and SUCC_ADD mode, it represent repeat number of sequence)
   		 CadOn and CadOff : Cadence On/Off time (ms)
   		 PatternOff : pattern Off time (ms)
   	     Gain : dB (range:0~-60)
   	     Freq : Hz (range:1~3999)
*/

/* Index is at dspparm.h TONES enumerator */
#ifdef SUPPORT_TONE_PROFILE
ToneCfgParam_t ToneTable[] =
#else
short ToneTable[][21] =
#endif
{
	/* Dial Pad */
	/*WJF 920610 changed, duration from 245 to 80, gain from -25 to -8 */
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 941, 1336, 0, 0, -9, -7, 0, 0},/*DIGIT_0*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 697, 1209, 0, 0, -9, -7, 0, 0},/*DIGIT_1*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 697, 1336, 0, 0, -9, -7, 0, 0},/*DIGIT_2*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 697, 1477, 0, 0, -9, -7, 0, 0},/*DIGIT_3*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 770, 1209, 0, 0, -9, -7, 0, 0},/*DIGIT_4*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 770, 1336, 0, 0, -9, -7, 0, 0},/*DIGIT_5*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 770, 1477, 0, 0, -9, -7, 0, 0},/*DIGIT_6*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 852, 1209, 0, 0, -9, -7, 0, 0},/*DIGIT_7*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 852, 1336, 0, 0, -9, -7, 0, 0},/*DIGIT_8*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 852, 1477, 0, 0, -9, -7, 0, 0},/*DIGIT_9*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 941, 1209, 0, 0, -9, -7, 0, 0},/*DIGIT_STAR*/
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 941, 1477, 0, 0, -9, -7, 0, 0},/*DIGIT_PONDA*/
	
	// OFFHOOKWARING, HOLDING, SOUNDRING
	//{SUCC, 0, 3, 330, 0, 330, 0, 330, 0, 0, 0, 0, 3, 950, 1400, 1800, 0, -4, -4, -4, 0},
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 4, 1400, 2060, 2450, 2600, -12, -12, -12, -12},	//USA
	//WJF 920610 changed for louder dial tone
	{SUCC, 0, 2, 500, 2500, 500, 2000, 0, 0, 0, 0, 0, 2, 400, 524, 0, 0, -7, -7, 0, 0},
	
	// ringing tone
	//WJF 920610 changed for louder ring tone
	{SUCC, 0, 12, 43, 0, 43, 0,  0, 0, 0, 0, 2000, 2, 2000, 1267, 0, 0, 0, 0, 0, 0},
	{SUCC, 0, 16, 33, 0, 33, 0,  0, 0, 0, 0, 2000, 2, 2000, 1267, 0, 0, 0, 0, 0, 0},
	{SUCC, 0, 22, 23, 0, 23, 0,  0, 0, 0, 0, 2000, 2, 2000, 1267, 0, 0, 0, 0, 0, 0},
	{SUCC, 0, 12, 43, 0, 43, 0,  0, 0, 0, 0, 2000, 2, 2300, 1467, 0, 0, 0, 0, 0, 0},
	{SUCC, 0,  8, 63, 0, 63, 0,  0, 0, 0, 0, 2000, 2, 2300, 1467, 0, 0, 0, 0, 0, 0},
	{SUCC, 0,  6, 83, 0, 83, 0,  0, 0, 0, 0, 2000, 2, 2300, 1467, 0, 0, 0, 0, 0, 0},
	{SUCC, 0, 30, 33, 0, 33, 0,  0, 0, 0, 0, 3000, 2, 800,  1067, 0, 0, 0, 0, 0, 0},	
	{SUCC, 0, 40, 17, 0, 17, 0, 17, 0, 0, 0, 3000, 3, 1333, 1455, 1621, 0, 0, 0, 0, 0},
	{SUCC, 0, 30, 17, 0, 17, 0, 17, 0, 0, 0, 3000, 3, 800,  1067, 1333, 0, 0, 0, 0, 0},
	{SUCC, 0, 18, 35, 0, 35, 0, 35, 0, 0, 0, 3000, 3, 800,  925,  1037, 0, 0, 0, 0, 0},

	//////////////////////////////////////////////////////////////////////////////////////////
#ifdef SUPPORT_TONE_PROFILE

	//equivalent tones: Congestion tone, Fast busy tone and the Re-order tone
	//sit-vacant tone == Number Unobtainable ?

	// USA
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// dial tone
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// stutter-dial tone
	{ADDITIVE, 1, 1, 500, 14500, 0, 0, 0, 0, 0, 0, 0, 1, 1400, 0, 0, 0, -7, -7, 0, 0},			// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// confirmation tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},			// busy tone
	{ADDITIVE, 0, 1, 300, 200, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 480, 0, 0, 0, -12, -12, -12, -12},		// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{ADDITIVE, 0, 1, 500, 600, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, -7, -7, 0},			// sit-vacant tone (i.e. Number Unobtainable? )
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 2, 1, 300, 10000, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 10000, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 10000, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 10000, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// UK
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// dial tone
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 10, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// confirmation tone
	{ADDITIVE, 0, 2, 400, 200, 400, 2000, 0, 0, 0, 0, 0, 2, 400, 450, 0, 0, -7, -7, 0, 0},		// ring-back tone
	{ADDITIVE, 0, 1, 375, 375, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 2, 400, 350, 225, 525, 0, 0, 0, 0, 0, 2, 400, 400, 0, 0, -7, -7, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 0, 1, 300, 32767, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// AUSTRALIA
	{MODULATED, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 425, 25, 0, 0, -7, -7, 0, 0},			// dial tone
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 10, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// confirmation tone
	{MODULATED, 0, 2, 400, 200, 400, 2000, 0, 0, 0, 0, 0, 2, 400, 425, 0, 0, -7, -7, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 375, 375, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 375, 375, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0 ,0 ,0 ,-7, 0, 0, 0},				// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{ADDITIVE, 0, 1, 2500, 500, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 0, 2, 200, 200, 200, 4400, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, -7, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// Hong Kong
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -13, -13, 0, 0},			// dial tone
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -13, -13, 0, 0},			// stutter-dial tone
	{ADDITIVE, 0, 1, 400, 40, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -13, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 0, 2, 100, 100, 300, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -13, -13, 0, 0},			// confirmation tone
	{ADDITIVE, 0, 2, 400, 200, 400, 3000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -13, -13, 0, 0},		// ring-back tone
	{ADDITIVE, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -13, -13, 0, 0},			// busy tone
	{ADDITIVE, 0, 1, 250, 250, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -13, -13, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, 0, 0, 0, 0},				// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -13, -13, 0, 0},		// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -13, -13, -13, 0},		// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -13, -13, -13, 0},		// sit-intercept tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -13, -13, 0, 0},			// sit-vacant tone(Number Unobtainable)
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -13, -13, -13, 0},		// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -13, -13, -13, -13},	// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -13, -13, -13, -13},		// calling card tone
	{ADDITIVE, 3, 2, 500, 500, 500, 8000, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -13, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 1, 300, 8000, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -13, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -13, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 1000, 0, 0, 0, 1, 440, 0, 0, 0, -13, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -13, -13, -13, 0},		// ingress ringback tone

	// Japan
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// stutter-dial tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// message waiting tone
	{ADDITIVE, 0, 2, 125, 125, 125, 625, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, -7, 0, 0},			// confirmation tone
	{MODULATED, 0, 1, 1000, 2000, 0, 0, 0, 0, 0, 0, 0, 2, 400, 16, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// roh tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 0, 1, 300, 32767, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// Sweden
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 10, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// confirmation tone
	{ADDITIVE, 0, 1, 1000, 5000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 250, 250, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 250, 750, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 330, 0, 330, 0, 330, 1000, 0, 0, 0, 3, 950, 1400, 1800, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 0, 2, 200, 500, 200, 32767, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 4, 200, 600, 200, 3000, 200, 600, 200, 32767, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// Germany
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 425, 400, 0, 0, -7, -7, 0, 0},			// stutter-dial tone
	{ADDITIVE, 10, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// confirmation tone
	{ADDITIVE, 0, 1, 1000, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 480, 480, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 240, 240, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 4, 1400, 2060, 2450, 2600, -12, -12, -12, -12},		// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 330, 0, 330, 0, 330, 1000, 0, 0, 0, 3, 950, 1400, 1800, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
        {ADDITIVE, 0, 2, 200, 200, 200, 5000, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},                     // call waiting #1 tone
	{ADDITIVE, 0, 1, 300, 32767, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// France
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 10, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},				// confirmation tone
	{ADDITIVE, 0, 1, 1500, 3500, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},				// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},				// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 0, 1, 300, 10000, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

#if 0
	// Tr57
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 10, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// confirmation tone
	{ADDITIVE, 0, 1, 1500, 3000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 200, 200, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 3, 200, 200, 200, 200, 200, 600, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 0, 1, 300, 32767, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -7, -7, -7, 0},			// ingress ringback tone
#else
	// Taiwan
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -13, -13, 0, 0},			// dial tone
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// stutter-dial tone
	{ADDITIVE, 10, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// confirmation tone
	{ADDITIVE, 0, 1, 1000, 2000, 0, 0, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -19, -19, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -24, -24, 0, 0},			// busy tone
	{ADDITIVE, 0, 1, 240, 260, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -24, -24, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 4, 1400, 2060, 2450, 2600, -12, -12, -12, -12},		// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 1, 1, 1500, 0, 0, 0, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -13, -13, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 250, 250, 250, 5250, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -13, -13, 0, 0},		// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 10000, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 10000, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone


#endif

	// Belgium
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 1000, 250, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 10, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},				// confirmation tone
	{ADDITIVE, 0, 1, 1000, 3000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 167, 167, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},				// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 330, 0, 330, 0, 330, 1000, 0, 0, 0, 3, 950, 1400, 1800, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 0, 2, 175, 175, 175, 3500, 0, 0, 0, 0, 0, 1, 1400, 0, 0, 0, -7, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// Finland
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 650, 25, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 10, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// confirmation tone
	{ADDITIVE, 0, 1, 1000, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 300, 300, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 200, 200, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 333, 0, 333, 0, 333, 1000, 0, 0, 0, 3, 950, 1400, 1800, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 0, 2, 150, 150, 150, 8000, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// Italy
	{ADDITIVE, 0, 2, 200, 200, 600, 1000, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 10, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// confirmation tone
	{ADDITIVE, 0, 1, 1000, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 200, 200, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},				// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 0, 1, 300, 32767, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 400, 100, 250, 100, 150, 32767, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// China
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 400, 40, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// confirmation tone
	{ADDITIVE, 0, 1, 1000, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 350, 350, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 240, 260, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 4, 1400, 2060, 2450, 2600, -12, -12, -12, -12},		// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 5, 1, 400, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// Extend country #1
	{FOUR_FREQ, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -10, 0, 0, 0},				// dial tone "DialT from C1"
	{FOUR_FREQ, 0, 1, 400, 40, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -5, 0, 0, 0},				// stutter-dial tone "SpecialDialT from C1"
	{FOUR_FREQ, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -10, 0, 0, 0},				// message waiting tone "MessageWaitingT from C1"
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// confirmation tone
	{FOUR_FREQ, 0, 1, 1000, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -5, 0, 0, 0},			// ring-back tone "RingingT form C1"
	{FOUR_FREQ, 0, 1, 350, 350, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -5, 0, 0, 0},				// busy tone "BusyT form C1"
	{FOUR_FREQ, 0, 1, 700, 700, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -10, 0, 0, 0},			// congestion tone (fast busy tone) "CongestionT from C1"
	{FOUR_FREQ, 0, 4, 100, 100, 100, 100, 100, 100, 400, 400, 0, 1, 450, 0, 0, 0, -10, 0, 0, 0, 450, 0, 0, 0, -10, 0, 0, 0, 450, 0, 0, 0, -10, 0, 0, 0, 450, 0, 0, 0, -10, 0, 0, 0},		// roh tone "NUT from C1"
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{FOUR_FREQ, 0, 1, 400, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -13, 0, 0, 0},				// call waiting #1 tone "CallWaitingT from C1"
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{FOUR_FREQ, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 400, 400, 0, 0, -14, -47, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{FOUR_FREQ, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 950, 0, 0, 0, 0, 0, 0, 0},			// extend #1 tone "HowlerT from C1"
	{FOUR_FREQ, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 400, 400, 425, 425, -14, -47, -14, -47},			// extend #2 tone "RecallDialT from C1"
	{FOUR_FREQ, 0, 4, 100, 100, 100, 100, 100, 100, 0, 0, 0, 2, 350, 440, 0, 0, -13, -13, 0, 0, 350, 440, 0, 0, -13, -13, 0, 0, 350, 440, 0, 0, -13, -13, 0, 0, 350, 440, 0, 0, -13, -13, 0, 0},			// extend #3 tone usa second dial tone
	{FOUR_FREQ, 0, 17, 100, 100, 100, 100, 100, 100, 100, 100, 0, 1, 480, 0, 0, 0, -16, 0, 0, 0, 480, 0, 0, 0, -15, 0, 0, 0, 480, 0, 0, 0, -14, 0, 0, 0, 480, 0, 0, 0, -13, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, -12, 0, 0, 0, 480, 0, 0, 0, -11, 0, 0, 0, 480, 0, 0, 0, -10, 0, 0, 0, 480, 0, 0, 0, -9, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, -8, 0, 0, 0, 480, 0, 0, 0, -7, 0, 0, 0, 480, 0, 0, 0, -6, 0, 0, 0, 480, 0, 0, 0, -5, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, -4, 0, 0, 0, 480, 0, 0, 0, -3, 0, 0, 0, 480, 0, 0, 0, -2, 0, 0, 0, 480, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 480, 0, 0, 0, 0, 0, 0, 0},			// extend #4 tone uk howler tone
	{FOUR_FREQ, 0, 32, 100, 100, 100, 100, 100, 100, 100, 100, 0, 1, 480, 0, 0, 0, -16, 0, 0, 0, 480, 0, 0, 0, -16, 0, 0, 0, 480, 0, 0, 0, -15, 0, 0, 0, 480, 0, 0, 0, -15, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, -14, 0, 0, 0, 480, 0, 0, 0, -14, 0, 0, 0, 480, 0, 0, 0, -13, 0, 0, 0, 480, 0, 0, 0, -13, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, -12, 0, 0, 0, 480, 0, 0, 0, -12, 0, 0, 0, 480, 0, 0, 0, -11, 0, 0, 0, 480, 0, 0, 0, -11, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, -10, 0, 0, 0, 480, 0, 0, 0, -10, 0, 0, 0, 480, 0, 0, 0, -9, 0, 0, 0, 480, 0, 0, 0, -9, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, -8, 0, 0, 0, 480, 0, 0, 0, -8, 0, 0, 0, 480, 0, 0, 0, -7, 0, 0, 0, 480, 0, 0, 0, -7, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, -6, 0, 0, 0, 480, 0, 0, 0, -6, 0, 0, 0, 480, 0, 0, 0, -5, 0, 0, 0, 480, 0, 0, 0, -5, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, -4, 0, 0, 0, 480, 0, 0, 0, -4, 0, 0, 0, 480, 0, 0, 0, -3, 0, 0, 0, 480, 0, 0, 0, -3, 0, 0, 0, 100, 100, 100, 100, 100, 100, 0, 0, 480, 0, 0, 0, -2, 0, 0, 0, 480, 0, 0, 0, -2, 0, 0, 0, 480, 0, 0, 0, -1, 0, 0, 0, 480, 0, 0, 0, 0, 0, 0, 0},			// extend #5 tone "32cad"
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// Extend country #2
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 452, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 400, 40, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// confirmation tone
	{ADDITIVE, 0, 1, 1000, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 350, 350, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 240, 260, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 4, 1400, 2060, 2450, 2600, -12, -12, -12, -12},		// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 5, 1, 400, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// Extend country #3
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 453, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 400, 40, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// confirmation tone
	{ADDITIVE, 0, 1, 1000, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 350, 350, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 240, 260, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 4, 1400, 2060, 2450, 2600, -12, -12, -12, -12},		// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 5, 1, 400, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

	// Extend country #4
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 454, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 400, 40, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// confirmation tone
	{ADDITIVE, 0, 1, 1000, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 350, 350, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 240, 260, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 4, 1400, 2060, 2450, 2600, -12, -12, -12, -12},		// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 5, 1, 400, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

#ifdef COUNTRY_TONE_RESERVED
	// Reserve
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// dial tone
	{ADDITIVE, 0, 1, 400, 40, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// stutter-dial tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},				// message waiting tone
	{ADDITIVE, 3, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// confirmation tone
	{ADDITIVE, 0, 1, 1000, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},			// ring-back tone
	{ADDITIVE, 0, 1, 350, 350, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// busy tone
	{ADDITIVE, 0, 1, 240, 260, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 4, 1400, 2060, 2450, 2600, -12, -12, -12, -12},		// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{SUCC, 0, 3, 400, 0, 250, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 5, 1, 400, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},				// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 32767, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 32767, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #2 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #3 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #5 tone
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

#endif

	// Customer Spec.
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// dial tone
	{ADDITIVE, 0, 4, 100, 100, 100, 100, 100, 100, 2000, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},		// stutter-dial tone
	{ADDITIVE, 0, 2, 100, 100, 100, 100, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},			// message waiting tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 100, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},		// confirmation tone
	{ADDITIVE, 0, 2, 2000, 4000, 2000, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},		// ring-back tone
	{ADDITIVE, 0, 2, 500, 500, 500, 500, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},			// busy tone
	{ADDITIVE, 0, 1, 240, 260, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},			// congestion tone (fast busy tone)
	{ADDITIVE, 0, 2, 1000, 1000, 1000, 1000, 0, 0, 0, 0, 0, 2, 1400, 2600, 0, 0, -7, -7, 0, 0},		// roh tone
	{ADDITIVE, 0, 2, 500, 200, 500, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},			// double ringback tone
	{SUCC, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-no circuit tone
	{SUCC, 0, 3, 250, 0, 250, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-intercept tone
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},			// sit-vacant tone
	{SUCC, 0, 3, 250, 0, 400, 0, 400, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, -7, -7, -7, 0},			// sit-reorder tone
	{SUCC_ADD, 0, 2, 60, 1000, 940, 1000, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},		// calling card with event tone
	{SUCC_ADD, 0, 2, 60, 0, 940, 0, 0, 0, 0, 0, 0, 4, 941, 1477, 440, 350, -7, -7, -7, -7},			// calling card tone
	{ADDITIVE, 0, 3, 500, 500, 500, 500, 500, 8500, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #1 tone
	{ADDITIVE, 0, 2, 100, 100, 100, 10000, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},			// call waiting #2 tone
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 10000, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #3 tone
	{ADDITIVE, 0, 3, 100, 100, 300, 100, 100, 10000, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// call waiting #4 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS1 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS2 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS3 tone
//	{ADDITIVE, 0, 3, 100, 100, 100, 30, 80, 10250, 0, 0, 0, 3, 440, 2130, 2750, 0, -7, -7, -7, 0},		// class2SAS&CAS4 tone
	{ADDITIVE, 0, 1, 450, 550, 0, 0, 0, 0, 0, 0, 0, 2, 580, 860, 0, 0, -13, -13, 0, 0},			// extend #1 tone
	{STEP_INC, 16, 2, 100, 100, 3000, 0, 0, 0, 0, 0, 0, 1, 480, 0, 0, 0, -16, 0, 0, 0, 480, 0, 0, 0, 0, 0, 0, 0},			// extend #2 tone
#if 1
	{FOUR_FREQ, 0, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 4, 1400, 2060, 2450, 2600, -12, -12, -12, -12, 0, 0, 0, 0, 0, 0, 0, 0},	// extend #3 tone
	{TWO_STEP, 3, 2, 100, 100, 3000, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0, 350, 440, 0, 0, -7, -7, 0, 0},		// extend #4 tone
	{TWO_STEP, 10, 2, 100, 100, 3000, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0, 350, 440, 0, 0, -7, -7, 0, 0},		// extend #5 tone
#else
	{STEP_INC, 21, 2, 100, 100, 3000, 0, 0, 0, 0, 0, 250, 1, 480, 0, 0, 0, -16, 0, 0, 0, 580, 0, 0, 0, -13, 0, 0, 0},			// extend #3 tone
	{STEP_INC, 15, 2, 250, 150, 3000, 0, 0, 0, 0, 0, 250, 4, 1050, 1550, 2050, 2550, -35, -35, -35, -35, 950, 1450, 1550, 1650, -13, -13, -13, -13},	// extend #4 tone
	{STEP_INC, 8, 2, 150, 250, 500, 100, 0, 0, 0, 0, 650, 4, 1050, 1550, 2050, 2550, -31, -31, -31, -31, 950, 1450, 1950, 2450, -14, -14, -14, -14},	// extend #5 tone
#endif
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 3, 440, 480, 100, 0, -10, -10, -10, 0},			// ingress ringback tone

#else
///////////////////////////////////////////////////////////////////
	// USA
	//WJF 920610 changed gain from -14 to -7
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 1, 250, 250, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},
	{SUCC, 3, 3, 330, 0, 330, 0, 330, 0, 0, 0, 0, 3, 950, 1400, 1800, 0, -7, -7, -7, 0},
	{ADDITIVE, 3, 2, 2000, 10000, 500, 10000, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 3, 1, 500, 15000, 0, 0, 0, 0, 0, 0, 0, 1, 1400, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 1, 300, 10000, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},
	
	// UK
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 2, 400, 200, 400, 2000, 0, 0, 0, 0, 0, 2, 400, 450, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 1, 375, 375, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 400, 350, 225, 525, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0 , 0},
	{SUCC, 3, 2, 200, 200, 200, 2000, 0, 0, 0, 0, 0, 2, 1200, 800, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 3, 1, 125, 125, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0 ,0 ,0},
	{ADDITIVE, 10, 2, 100, 100, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},
	
	// AUSTRALIA
	{MODULATED, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 425, 25, 0, 0, -7, -7, 0, 0},
	{MODULATED, 0, 2, 400, 200, 400, 2000, 0, 0, 0, 0, 0, 2, 400, 17, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 1, 375, 375, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 1, 375, 375, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0 ,0 ,0 ,-7, 0, 0, 0},
	{ADDITIVE, 3, 2, 100, 100, 100, 4700, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7 , 0, 0, 0},
	{ADDITIVE, 3, 1, 2500, 500, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0 ,0 ,0 ,-7, 0, 0, 0},
	{ADDITIVE, 3, 1, 425, 14525, 0, 0, 0, 0, 0, 0, 0, 1, 1400, 0 ,0 , 0, -7, 0, 0, 0},
	{SUCC_ADD, 3, 2, 75, 150, 75, 2700, 0, 0, 0, 0, 0, 4, 1100, 1750, 750, 1450, -7, -7, -7, -7},
	{SUCC, 0, 2, 100, 100, 100, 4700, 0, 0, 0, 0, 0, 2, 425, 525, 0, 0, -7, -7, 0, 0},

	//Hong Kong
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 4, 400 , 200, 400, 3000, 0, 0, 0, 0, 0, 2, 340, 480, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 4, 500 , 500, 500, 500, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 2, 250 , 250, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 2, 400 , 400, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},

	//Japan
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 1000 , 2000, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 500 , 500, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},

	//Sweden
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 1000 , 5000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 250 , 250, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 250 , 750, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 10, 2, 100 , 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},

	//Germany
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 1000 , 4000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 500 , 500, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 250 , 250, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 10, 2, 100 , 100, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},

	//France
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 1500 , 3500, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 500 , 500, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 500 , 500, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 10, 2, 100 , 100, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},

	//Tr57
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 1500 , 3000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 200 , 200, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 6, 200 , 200, 200, 200, 200, 600, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 10, 2, 100 , 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},

	//Belgium
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 1000 , 3000, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 500 , 500, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 167 , 167, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 10, 2, 100 , 100, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},

	//Finland
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 1000 , 4000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 300 , 300, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 200 , 200, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 10, 2, 100 , 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},

	//Italy
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 1000 , 4000, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 500 , 500, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 200 , 200, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 10, 2, 100 , 100, 0, 0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0, -7, 0, 0, 0},

	//China
	{ADDITIVE, 0, 1, 2000 , 0, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 1000 , 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 350 , 350, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, -7, 0, 0, 0},
	{ADDITIVE, 0, 2, 240, 260, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},
	{ADDITIVE, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -7, 0, 0, 0},

	//Self set
/*	handsome add function 2005.12.1     */
#endif	/* #ifdef SUPPORT_TONE_PROFILE */

	{ADDITIVE, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},		// custom tone 1
	{ADDITIVE, 0, 2, 2000, 4000, 2000, 4000, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, -7, -7, 0, 0},	// custom tone 2
	{ADDITIVE, 0, 2, 500, 500, 500, 500, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},		// custom tone 3
	{ADDITIVE, 0, 3, 500, 500, 500, 500, 500, 8500, 0, 0, 0, 1, 440, 0, 0, 0, -7, 0, 0, 0},		// custom tone 4
	{ADDITIVE, 0, 2, 100, 100, 100, 100, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},		// custom tone 5
	{ADDITIVE, 0, 3, 100, 100, 100, 100, 100, 100, 0, 0, 0, 2, 350, 440, 0, 0, -7, -7, 0, 0},	// custom tone 6
	{ADDITIVE, 0, 1, 240, 260, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, -7, -7, 0, 0},		// custom tone 7
	{ADDITIVE, 0, 2, 1000, 1000, 1000, 1000, 0, 0, 0, 0, 0, 2, 1400, 2600, 0, 0, -7, -7, 0, 0}	// custom tone 8

#ifdef SW_DTMF_CID
	// hc+ 1124 for DTMF CID =====================================================
																							   , 
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 697, 1633, 0, 0, -9, -7, 0, 0},	// DTMF digit A transmit start	
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 770, 1633, 0, 0, -9, -7, 0, 0},	// DTMF digit B transmit info
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 852, 1633, 0, 0, -9, -7, 0, 0},	// DTMF digit C stop
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 941, 1633, 0, 0, -9, -7, 0, 0},	// DTMF digit D forward start
#endif
	// SAS, FSK_SAS
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 440, 0, 0, 0, -8, 0, 0, 0},	// off hook CID SAS alert signal

	// CAS, FSK_ALERT
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2130, 2750, 0, 0, -8, -8, 0, 0},// off hook CID CAS alert signal

	// FSK_MUTE
	{ADDITIVE, 0, 1, 1, 32000, 0, 0, 0, 0, 0, 0, 0, 1, 101, 0, 0, 0, -60, 0, 0, 0},// off hook FSK CID mute voice, the cycle is modified to continuous tone
	
	{ADDITIVE, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 400, 0, 0, 0, -12, 0, 0, 0},// For japan type 2 cid, the IIT tone.

	// thlin+ continous DTMF tone play for RFC2833, SIP Info (twist ration: 2dB)
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 941, 1336, 0, 0, -8, -6, 0, 0},/*DIGIT_0*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 697, 1209, 0, 0, -8, -6, 0, 0},/*DIGIT_1*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 697, 1336, 0, 0, -8, -6, 0, 0},/*DIGIT_2*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 697, 1477, 0, 0, -8, -6, 0, 0},/*DIGIT_3*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 770, 1209, 0, 0, -8, -6, 0, 0},/*DIGIT_4*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 770, 1336, 0, 0, -8, -6, 0, 0},/*DIGIT_5*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 770, 1477, 0, 0, -8, -6, 0, 0},/*DIGIT_6*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 852, 1209, 0, 0, -8, -6, 0, 0},/*DIGIT_7*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 852, 1336, 0, 0, -8, -6, 0, 0},/*DIGIT_8*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 852, 1477, 0, 0, -8, -6, 0, 0},/*DIGIT_9*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 941, 1209, 0, 0, -8, -6, 0, 0},/*DIGIT_STAR*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 941, 1477, 0, 0, -8, -6, 0, 0},/*DIGIT_PONDA*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 697, 1633, 0, 0, -8, -6, 0, 0},/*DIGIT_A*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 770, 1633, 0, 0, -8, -6, 0, 0},/*DIGIT_B*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 852, 1633, 0, 0, -8, -6, 0, 0},/*DIGIT_C*/
	{ADDITIVE, 0, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0, 2, 941, 1633, 0, 0, -8, -6, 0, 0}, /*DIGIT_D*/

	//=========================================================================================
	// continous Modem/FAX tone play for RFC2833
	{ADDITIVE, 0, 1, 3000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2100, 0, 0, 0, -7, 0, 0, 0},		// ANS
	{ADDITIVE, 0, 1, 3000, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2100, 0, 0, 0, -7, 0, 0, 0},		// ANS_BAR
	{ADDITIVE, 0, 1, 3000, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2085, 2100, 2115, 0, -25, -3, -25, 0},// ANSam
	{ADDITIVE, 0, 1, 3000, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2085, 2100, 2115, 0, -25, -3, -25, 0},// ANSam_BAR
	{ADDITIVE, 0, 1, 500, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1100, 0, 0, 0, -7, 0, 0, 0},		// CNG
	{ADDITIVE, 0, 1, 400, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1375, 2002, 0, 0, -7, -7, 0, 0},	// V8bis Cre tone

	//=====================================================================
	// user define tone 
	{FOUR_FREQ, 0, 16, 1000, 1000, 900, 900, 700, 700, 500, 500, 0, 4, 888, 1555, 2222, 2777, -18, -10, -12, -15, 888, 1555, 2222, 0, -10, -15, -18, 0, 888, 1555, 0, 0, -20, -15, 0, 0, 888, 0, 0, 0, -10, 0, 0, 0, 500, 100, 400, 90, 300, 70, 200, 50, 888, 1555, 2222, 2777, -18, -10, -12, -15, 888, 1555, 2222, 0, -10, -15, -18, 0, 888, 1555, 0, 0, -20, -15, 0, 0, 888, 0, 0, 0, -10, 0, 0, 0, 500, 100, 400, 90, 300, 70, 200, 50, 888, 1555, 2222, 2777, -18, -10, -12, -15, 888, 1555, 2222, 0, -10, -15, -18, 0, 888, 1555, 0, 0, -20, -15, 0, 0, 888, 0, 0, 0, -10, 0, 0, 0, 500, 100, 400, 90, 300, 70, 200, 50, 888, 1555, 2222, 2777, -18, -10, -12, -15, 888, 1555, 2222, 0, -10, -15, -18, 0, 888, 1555, 0, 0, -20, -15, 0, 0, 888, 0, 0, 0, -10, 0, 0, 0},// user define tone #1
	{FOUR_FREQ, 2, 3, 1000, 1000, 900, 900, 700, 700, 0, 0, 0, 4, 888, 1555, 2222, 2777, -18, -10, -12, -15, 888, 1555, 2222, 0, -10, -15, -18, 0, 888, 1555, 0, 0, -20, -15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// user define tone #2
	{FOUR_FREQ, 4, 2, 1000, 1000, 900, 900, 0, 0, 0, 0, 0, 4, 888, 1555, 2222, 2777, -18, -10, -12, -15, 888, 1555, 2222, 0, -10, -15, -18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// user define tone #3
	{FOUR_FREQ, 6, 1, 1000, 1000, 0, 0, 0, 0, 0, 0, 0, 4, 888, 1555, 2222, 2777, -18, -10, -12, -15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// user define tone #4
	{FOUR_FREQ, 0, 32, 1000, 0, 900, 0, 700, 0, 500, 700, 0, 4, 888, 1555, 2222, 2777, -18, -10, -12, -15, 777, 1444, 2111, 2666, -10, -15, -18, -16, 666, 1333, 1999, 2555, -20, -15, -18, -17, 555, 1222, 1888, 2444, -25, -22, -26, -24, 10, 0, 15, 0, 70, 0, 50, 70, 888, 1555, 2222, 2777, -18, -10, -12, -15, 777, 1444, 2111, 2666, -10, -15, -18, -16, 666, 1333, 1999, 2555, -20, -15, -18, -17, 555, 1222, 1888, 2444, -25, -22, -26, -24, 10, 0, 15, 0, 70, 0, 50, 70, 888, 1555, 2222, 2777, -18, -10, -12, -15, 777, 1444, 2111, 2666, -10, -15, -18, -16, 666, 1333, 1999, 2555, -20, -15, -18, -17, 555, 1222, 1888, 2444, -25, -22, -26, -24, 10, 0, 15, 0, 70, 0, 50, 70, 888, 1555, 2222, 2777, -18, -10, -12, -15, 777, 1444, 2111, 2666, -10, -15, -18, -16, 666, 1333, 1999, 2555, -20, -15, -18, -17, 555, 1222, 1888, 2444, -25, -22, -26, -24, 10, 0, 15, 0, 70, 0, 50, 70, 888, 1555, 2222, 2777, -18, -10, -12, -15, 777, 1444, 2111, 2666, -10, -15, -18, -16, 666, 1333, 1999, 2555, -20, -15, -18, -17, 555, 1222, 1888, 2444, -25, -22, -26, -24, 10, 0, 15, 0, 70, 0, 50, 70, 888, 1555, 2222, 2777, -18, -10, -12, -15, 777, 1444, 2111, 2666, -10, -15, -18, -16, 666, 1333, 1999, 2555, -20, -15, -18, -17, 555, 1222, 1888, 2444, -25, -22, -26, -24, 10, 0, 15, 0, 70, 0, 50, 70, 888, 1555, 2222, 2777, -18, -10, -12, -15, 777, 1444, 2111, 2666, -10, -15, -18, -16, 666, 1333, 1999, 2555, -20, -15, -18, -17, 555, 1222, 1888, 2444, -25, -22, -26, -24, 10, 0, 15, 0, 70, 0, 50, 70, 888, 1555, 2222, 2777, -18, -10, -12, -15, 777, 1444, 2111, 2666, -10, -15, -18, -16, 666, 1333, 1999, 2555, -20, -15, -18, -17, 555, 1222, 1888, 2444, -25, -22, -26, -24},// user define tone #5

};
#ifndef PLAYTONE_UNITTEST
void update_cont_DTMF_tone_volume(unsigned int digit, unsigned int volume)
{
	if ((digit < 0) || (digit > 16))
	{
		if ((digit >= 32) && (digit <= 49)) // Fax/Modem event
			return;
		else
		{
			printk("\x1B[31m" "update_cont_DTMF_tone_volume error! wrong digit %d\n" "\x1B[0m", digit);
			return;
		}
	}
	
	if (volume > 36)
	{
		printk("\x1B[31m" "Don't accept -%d dBm0 (< -36 dBm0) volume for RFC2833 event" "\x1B[0m", volume);
		return;
	}
	
	ToneTable[DIGIT_0_CONT + digit].Gain0 = -4 - volume - 1;
	ToneTable[DIGIT_0_CONT + digit].Gain1 = -4 - volume + 1;
	
	//printk("update DMTF tone to -%d dBm0, digit=%d\n", volume, digit);
}
#endif /* PLAYTONE_UNITTEST */

#if 0
JB_CONTROL_PARAMETER CtrlParam =
{
	JB_ADPTSPEED_MEDIUM,
	JB_TRCKSENS_LOW,
	0,
	600,
	300
};
#endif

