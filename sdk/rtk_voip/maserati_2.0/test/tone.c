#include "voip_manager.h"

/*
typedef struct
{
	unsigned long	toneType;	///< ADDITIVE, MODULATED, SUCC, SUCC_ADD
	unsigned short	cycle;		///< "<0": illegal value, "0": represent "continuous tone", ">0": cycle number

	unsigned short	cadNUM;		///< Cadence number (in SUCC and SUCC_ADD mode, it represent repeat number of sequence)

	unsigned long	CadOn0;		///< Cadence On0 time (ms)
	unsigned long	CadOff0;	///< Cadence Off0 time (ms)
	unsigned long	CadOn1;		///< Cadence On1 time (ms)
	unsigned long	CadOff1;	///< Cadence Off1 time (ms)
	unsigned long	CadOn2;		///< Cadence On2 time (ms)
	unsigned long	CadOff2;	///< Cadence Off2 time (ms)
	unsigned long	CadOn3;		///< Cadence On3 time (ms)
	unsigned long	CadOff3;	///< Cadence Off3 time (ms)

	unsigned long PatternOff;	///< pattern Off time (ms)
	unsigned long ToneNUM;		///< tone number (1..4)

	unsigned long	Freq0;		///< Freq0 (Hz)
	unsigned long	Freq1;		///< Freq1 (Hz)
	unsigned long	Freq2;		///< Freq2 (Hz)
	unsigned long	Freq3;		///< Freq3 (Hz)

	long Gain0;					///< Gain0 (db)
	long Gain1;					///< Gain1 (db)
	long Gain2;					///< Gain2 (db)
	long Gain3;					///< Gain3 (db)

	unsigned long	C1_Freq0;		///< C1_Freq0 (Hz)
	unsigned long	C1_Freq1;		///< C1_Freq1 (Hz)
	unsigned long	C1_Freq2;		///< C1_Freq2 (Hz)
	unsigned long	C1_Freq3;		///< C1_Freq3 (Hz)

	long C1_Gain0;			///< C1_Gain0 (db)
	long C1_Gain1;			///< C1_Gain1 (db)
	long C1_Gain2;			///< C1_Gain2 (db)
	long C1_Gain3;			///< C1_Gain3 (db)

	unsigned long	C2_Freq0;		///< C2_Freq0 (Hz)
	unsigned long	C2_Freq1;		///< C2_Freq1 (Hz)
	unsigned long	C2_Freq2;		///< C2_Freq2 (Hz)
	unsigned long	C2_Freq3;		///< C2_Freq3 (Hz)

	long C2_Gain0;			///< C2_Gain0 (db)
	long C2_Gain1;			///< C2_Gain1 (db)
	long C2_Gain2;			///< C2_Gain2 (db)
	long C2_Gain3;			///< C2_Gain3 (db)

	unsigned long	C3_Freq0;		///< C3_Freq0 (Hz)
	unsigned long	C3_Freq1;		///< C3_Freq1 (Hz)
	unsigned long	C3_Freq2;		///< C3_Freq2 (Hz)
	unsigned long	C3_Freq3;		///< C3_Freq3 (Hz)

	long C3_Gain0;			///< C3_Gain0 (db)
	long C3_Gain1;			///< C3_Gain1 (db)
	long C3_Gain2;			///< C3_Gain2 (db)
	long C3_Gain3;			///< C3_Gain3 (db)

	unsigned long	CadOn4;		///< Cadence On4 time (ms)
	unsigned long	CadOff4;		///< Cadence Off4 time (ms)
	unsigned long	CadOn5;		///< Cadence On5 time (ms)
	unsigned long	CadOff5;		///< Cadence Off5 time (ms)
	unsigned long	CadOn6;		///< Cadence On6 time (ms)
	unsigned long	CadOff6;		///< Cadence Off6 time (ms)
	unsigned long	CadOn7;		///< Cadence On7 time (ms)
	unsigned long	CadOff7;		///< Cadence Off7 time (ms)

	unsigned long	C4_Freq0;		///< C4_Freq0 (Hz)
	unsigned long	C4_Freq1;		///< C4_Freq1 (Hz)
	unsigned long	C4_Freq2;		///< C4_Freq2 (Hz)
	unsigned long	C4_Freq3;		///< C4_Freq3 (Hz)

	long C4_Gain0;			///< C4_Gain0 (db)
	long C4_Gain1;			///< C4_Gain1 (db)
	long C4_Gain2;			///< C4_Gain2 (db)
	long C4_Gain3;			///< C4_Gain3 (db)

	unsigned long	C5_Freq0;		///< C5_Freq0 (Hz)
	unsigned long	C5_Freq1;		///< C5_Freq1 (Hz)
	unsigned long	C5_Freq2;		///< C5_Freq2 (Hz)
	unsigned long	C5_Freq3;		///< C5_Freq3 (Hz)

	long C5_Gain0;			///< C5_Gain0 (db)
	long C5_Gain1;			///< C5_Gain1 (db)
	long C5_Gain2;			///< C5_Gain2 (db)
	long C5_Gain3;			///< C5_Gain3 (db)

	unsigned long	C6_Freq0;		///< C6_Freq0 (Hz)
	unsigned long	C6_Freq1;		///< C6_Freq1 (Hz)
	unsigned long	C6_Freq2;		///< C6_Freq2 (Hz)
	unsigned long	C6_Freq3;		///< C6_Freq3 (Hz)

	long C6_Gain0;			///< C6_Gain0 (db)
	long C6_Gain1;			///< C6_Gain1 (db)
	long C6_Gain2;			///< C6_Gain2 (db)
	long C6_Gain3;			///< C6_Gain3 (db)

	unsigned long	C7_Freq0;		///< C7_Freq0 (Hz)
	unsigned long	C7_Freq1;		///< C7_Freq1 (Hz)
	unsigned long	C7_Freq2;		///< C7_Freq2 (Hz)
	unsigned long	C7_Freq3;		///< C7_Freq3 (Hz)

	long C7_Gain0;			///< C7_Gain0 (db)
	long C7_Gain1;			///< C7_Gain1 (db)
	long C7_Gain2;			///< C7_Gain2 (db)
	long C7_Gain3;			///< C7_Gain3 (db)

	unsigned long	CadOn8;		///< Cadence On8 time (ms)
	unsigned long	CadOff8;		///< Cadence Off8 time (ms)
	unsigned long	CadOn9;		///< Cadence On9 time (ms)
	unsigned long	CadOff9;		///< Cadence Off9 time (ms)
	unsigned long	CadOn10;		///< Cadence On10 time (ms)
	unsigned long	CadOff10;		///< Cadence Off10 time (ms)
	unsigned long	CadOn11;		///< Cadence On11 time (ms)
	unsigned long	CadOff11;		///< Cadence Off11 time (ms)

	unsigned long	C8_Freq0;		///< C8_Freq0 (Hz)
	unsigned long	C8_Freq1;		///< C8_Freq1 (Hz)
	unsigned long	C8_Freq2;		///< C8_Freq2 (Hz)
	unsigned long	C8_Freq3;		///< C8_Freq3 (Hz)

	long C8_Gain0;			///< C8_Gain0 (db)
	long C8_Gain1;			///< C8_Gain1 (db)
	long C8_Gain2;			///< C8_Gain2 (db)
	long C8_Gain3;			///< C8_Gain3 (db)

	unsigned long	C9_Freq0;		///< C9_Freq0 (Hz)
	unsigned long	C9_Freq1;		///< C9_Freq1 (Hz)
	unsigned long	C9_Freq2;		///< C9_Freq2 (Hz)
	unsigned long	C9_Freq3;		///< C9_Freq3 (Hz)

	long C9_Gain0;			///< C9_Gain0 (db)
	long C9_Gain1;			///< C9_Gain1 (db)
	long C9_Gain2;			///< C9_Gain2 (db)
	long C9_Gain3;			///< C9_Gain3 (db)

	unsigned long	C10_Freq0;		///< C10_Freq0 (Hz)
	unsigned long	C10_Freq1;		///< C10_Freq1 (Hz)
	unsigned long	C10_Freq2;		///< C10_Freq2 (Hz)
	unsigned long	C10_Freq3;		///< C10_Freq3 (Hz)

	long C10_Gain0;			///< C10_Gain0 (db)
	long C10_Gain1;			///< C10_Gain1 (db)
	long C10_Gain2;			///< C10_Gain2 (db)
	long C10_Gain3;			///< C10_Gain3 (db)

	unsigned long	C11_Freq0;		///< C11_Freq0 (Hz)
	unsigned long	C11_Freq1;		///< C11_Freq1 (Hz)
	unsigned long	C11_Freq2;		///< C11_Freq2 (Hz)
	unsigned long	C11_Freq3;		///< C11_Freq3 (Hz)

	long C11_Gain0;			///< C11_Gain0 (db)
	long C11_Gain1;			///< C11_Gain1 (db)
	long C11_Gain2;			///< C11_Gain2 (db)
	long C11_Gain3;			///< C11_Gain3 (db)

	unsigned long	CadOn12;		///< Cadence On12 time (ms)
	unsigned long	CadOff12;		///< Cadence Off12 time (ms)
	unsigned long	CadOn13;		///< Cadence On13 time (ms)
	unsigned long	CadOff13;		///< Cadence Off13 time (ms)
	unsigned long	CadOn14;		///< Cadence On14 time (ms)
	unsigned long	CadOff14;		///< Cadence Off14 time (ms)
	unsigned long	CadOn15;		///< Cadence On15 time (ms)
	unsigned long	CadOff15;		///< Cadence Off15 time (ms)

	unsigned long	C12_Freq0;		///< C12_Freq0 (Hz)
	unsigned long	C12_Freq1;		///< C12_Freq1 (Hz)
	unsigned long	C12_Freq2;		///< C12_Freq2 (Hz)
	unsigned long	C12_Freq3;		///< C12_Freq3 (Hz)

	long C12_Gain0;			///< C12_Gain0 (db)
	long C12_Gain1;			///< C12_Gain1 (db)
	long C12_Gain2;			///< C12_Gain2 (db)
	long C12_Gain3;			///< C12_Gain3 (db)

	unsigned long	C13_Freq0;		///< C13_Freq0 (Hz)
	unsigned long	C13_Freq1;		///< C13_Freq1 (Hz)
	unsigned long	C13_Freq2;		///< C13_Freq2 (Hz)
	unsigned long	C13_Freq3;		///< C13_Freq3 (Hz)

	long C13_Gain0;			///< C13_Gain0 (db)
	long C13_Gain1;			///< C13_Gain1 (db)
	long C13_Gain2;			///< C13_Gain2 (db)
	long C13_Gain3;			///< C13_Gain3 (db)

	unsigned long	C14_Freq0;		///< C14_Freq0 (Hz)
	unsigned long	C14_Freq1;		///< C14_Freq1 (Hz)
	unsigned long	C14_Freq2;		///< C14_Freq2 (Hz)
	unsigned long	C14_Freq3;		///< C14_Freq3 (Hz)

	long C14_Gain0;			///< C14_Gain0 (db)
	long C14_Gain1;			///< C14_Gain1 (db)
	long C14_Gain2;			///< C14_Gain2 (db)
	long C14_Gain3;			///< C14_Gain3 (db)

	unsigned long	C15_Freq0;		///< C15_Freq0 (Hz)
	unsigned long	C15_Freq1;		///< C15_Freq1 (Hz)
	unsigned long	C15_Freq2;		///< C15_Freq2 (Hz)
	unsigned long	C15_Freq3;		///< C15_Freq3 (Hz)

	long C15_Gain0;			///< C15_Gain0 (db)
	long C15_Gain1;			///< C15_Gain1 (db)
	long C15_Gain2;			///< C15_Gain2 (db)
	long C15_Gain3;			///< C15_Gain3 (db)

	unsigned long	CadOn16;		///< Cadence On16 time (ms)
	unsigned long	CadOff16;		///< Cadence Off16 time (ms)
	unsigned long	CadOn17;		///< Cadence On17 time (ms)
	unsigned long	CadOff17;		///< Cadence Off17 time (ms)
	unsigned long	CadOn18;		///< Cadence On18 time (ms)
	unsigned long	CadOff18;		///< Cadence Off18 time (ms)
	unsigned long	CadOn19;		///< Cadence On19 time (ms)
	unsigned long	CadOff19;		///< Cadence Off19 time (ms)

	unsigned long	C16_Freq0;		///< C16_Freq0 (Hz)
	unsigned long	C16_Freq1;		///< C16_Freq1 (Hz)
	unsigned long	C16_Freq2;		///< C16_Freq2 (Hz)
	unsigned long	C16_Freq3;		///< C16_Freq3 (Hz)

	long C16_Gain0;			///< C16_Gain0 (db)
	long C16_Gain1;			///< C16_Gain1 (db)
	long C16_Gain2;			///< C16_Gain2 (db)
	long C16_Gain3;			///< C16_Gain3 (db)

	unsigned long	C17_Freq0;		///< C17_Freq0 (Hz)
	unsigned long	C17_Freq1;		///< C17_Freq1 (Hz)
	unsigned long	C17_Freq2;		///< C17_Freq2 (Hz)
	unsigned long	C17_Freq3;		///< C17_Freq3 (Hz)

	long C17_Gain0;			///< C17_Gain0 (db)
	long C17_Gain1;			///< C17_Gain1 (db)
	long C17_Gain2;			///< C17_Gain2 (db)
	long C17_Gain3;			///< C17_Gain3 (db)

	unsigned long	C18_Freq0;		///< C18_Freq0 (Hz)
	unsigned long	C18_Freq1;		///< C18_Freq1 (Hz)
	unsigned long	C18_Freq2;		///< C18_Freq2 (Hz)
	unsigned long	C18_Freq3;		///< C18_Freq3 (Hz)

	long C18_Gain0;			///< C18_Gain0 (db)
	long C18_Gain1;			///< C18_Gain1 (db)
	long C18_Gain2;			///< C18_Gain2 (db)
	long C18_Gain3;			///< C18_Gain3 (db)

	unsigned long	C19_Freq0;		///< C19_Freq0 (Hz)
	unsigned long	C19_Freq1;		///< C19_Freq1 (Hz)
	unsigned long	C19_Freq2;		///< C19_Freq2 (Hz)
	unsigned long	C19_Freq3;		///< C19_Freq3 (Hz)

	long C19_Gain0;			///< C19_Gain0 (db)
	long C19_Gain1;			///< C19_Gain1 (db)
	long C19_Gain2;			///< C19_Gain2 (db)
	long C19_Gain3;			///< C19_Gain3 (db)

	unsigned long	CadOn20;		///< Cadence On20 time (ms)
	unsigned long	CadOff20;		///< Cadence Off20 time (ms)
	unsigned long	CadOn21;		///< Cadence On21 time (ms)
	unsigned long	CadOff21;		///< Cadence Off21 time (ms)
	unsigned long	CadOn22;		///< Cadence On22 time (ms)
	unsigned long	CadOff22;		///< Cadence Off22 time (ms)
	unsigned long	CadOn23;		///< Cadence On23 time (ms)
	unsigned long	CadOff23;		///< Cadence Off23 time (ms)

	unsigned long	C20_Freq0;		///< C20_Freq0 (Hz)
	unsigned long	C20_Freq1;		///< C20_Freq1 (Hz)
	unsigned long	C20_Freq2;		///< C20_Freq2 (Hz)
	unsigned long	C20_Freq3;		///< C20_Freq3 (Hz)

	long C20_Gain0;			///< C20_Gain0 (db)
	long C20_Gain1;			///< C20_Gain1 (db)
	long C20_Gain2;			///< C20_Gain2 (db)
	long C20_Gain3;			///< C20_Gain3 (db)

	unsigned long	C21_Freq0;		///< C21_Freq0 (Hz)
	unsigned long	C21_Freq1;		///< C21_Freq1 (Hz)
	unsigned long	C21_Freq2;		///< C21_Freq2 (Hz)
	unsigned long	C21_Freq3;		///< C21_Freq3 (Hz)

	long C21_Gain0;			///< C21_Gain0 (db)
	long C21_Gain1;			///< C21_Gain1 (db)
	long C21_Gain2;			///< C21_Gain2 (db)
	long C21_Gain3;			///< C21_Gain3 (db)

	unsigned long	C22_Freq0;		///< C22_Freq0 (Hz)
	unsigned long	C22_Freq1;		///< C22_Freq1 (Hz)
	unsigned long	C22_Freq2;		///< C22_Freq2 (Hz)
	unsigned long	C22_Freq3;		///< C22_Freq3 (Hz)

	long C22_Gain0;			///< C22_Gain0 (db)
	long C22_Gain1;			///< C22_Gain1 (db)
	long C22_Gain2;			///< C22_Gain2 (db)
	long C22_Gain3;			///< C22_Gain3 (db)

	unsigned long	C23_Freq0;		///< C23_Freq0 (Hz)
	unsigned long	C23_Freq1;		///< C23_Freq1 (Hz)
	unsigned long	C23_Freq2;		///< C23_Freq2 (Hz)
	unsigned long	C23_Freq3;		///< C23_Freq3 (Hz)

	long C23_Gain0;			///< C23_Gain0 (db)
	long C23_Gain1;			///< C23_Gain1 (db)
	long C23_Gain2;			///< C23_Gain2 (db)
	long C23_Gain3;			///< C23_Gain3 (db)

	unsigned long	CadOn24;		///< Cadence On24 time (ms)
	unsigned long	CadOff24;		///< Cadence Off24 time (ms)
	unsigned long	CadOn25;		///< Cadence On25 time (ms)
	unsigned long	CadOff25;		///< Cadence Off25 time (ms)
	unsigned long	CadOn26;		///< Cadence On26 time (ms)
	unsigned long	CadOff26;		///< Cadence Off26 time (ms)
	unsigned long	CadOn27;		///< Cadence On27 time (ms)
	unsigned long	CadOff27;		///< Cadence Off27 time (ms)

	unsigned long	C24_Freq0;		///< C24_Freq0 (Hz)
	unsigned long	C24_Freq1;		///< C24_Freq1 (Hz)
	unsigned long	C24_Freq2;		///< C24_Freq2 (Hz)
	unsigned long	C24_Freq3;		///< C24_Freq3 (Hz)

	long C24_Gain0;			///< C24_Gain0 (db)
	long C24_Gain1;			///< C24_Gain1 (db)
	long C24_Gain2;			///< C24_Gain2 (db)
	long C24_Gain3;			///< C24_Gain3 (db)

	unsigned long	C25_Freq0;		///< C25_Freq0 (Hz)
	unsigned long	C25_Freq1;		///< C25_Freq1 (Hz)
	unsigned long	C25_Freq2;		///< C25_Freq2 (Hz)
	unsigned long	C25_Freq3;		///< C25_Freq3 (Hz)

	long C25_Gain0;			///< C25_Gain0 (db)
	long C25_Gain1;			///< C25_Gain1 (db)
	long C25_Gain2;			///< C25_Gain2 (db)
	long C25_Gain3;			///< C25_Gain3 (db)

	unsigned long	C26_Freq0;		///< C26_Freq0 (Hz)
	unsigned long	C26_Freq1;		///< C26_Freq1 (Hz)
	unsigned long	C26_Freq2;		///< C26_Freq2 (Hz)
	unsigned long	C26_Freq3;		///< C26_Freq3 (Hz)

	long C26_Gain0;			///< C26_Gain0 (db)
	long C26_Gain1;			///< C26_Gain1 (db)
	long C26_Gain2;			///< C26_Gain2 (db)
	long C26_Gain3;			///< C26_Gain3 (db)

	unsigned long	C27_Freq0;		///< C27_Freq0 (Hz)
	unsigned long	C27_Freq1;		///< C27_Freq1 (Hz)
	unsigned long	C27_Freq2;		///< C27_Freq2 (Hz)
	unsigned long	C27_Freq3;		///< C27_Freq3 (Hz)

	long C27_Gain0;			///< C27_Gain0 (db)
	long C27_Gain1;			///< C27_Gain1 (db)
	long C27_Gain2;			///< C27_Gain2 (db)
	long C27_Gain3;			///< C27_Gain3 (db)

	unsigned long	CadOn28;		///< Cadence On28 time (ms)
	unsigned long	CadOff28;		///< Cadence Off28 time (ms)
	unsigned long	CadOn29;		///< Cadence On29 time (ms)
	unsigned long	CadOff29;		///< Cadence Off29 time (ms)
	unsigned long	CadOn30;		///< Cadence On30 time (ms)
	unsigned long	CadOff30;		///< Cadence Off30 time (ms)
	unsigned long	CadOn31;		///< Cadence On31 time (ms)
	unsigned long	CadOff31;		///< Cadence Off31 time (ms)

	unsigned long	C28_Freq0;		///< C28_Freq0 (Hz)
	unsigned long	C28_Freq1;		///< C28_Freq1 (Hz)
	unsigned long	C28_Freq2;		///< C28_Freq2 (Hz)
	unsigned long	C28_Freq3;		///< C28_Freq3 (Hz)

	long C28_Gain0;			///< C28_Gain0 (db)
	long C28_Gain1;			///< C28_Gain1 (db)
	long C28_Gain2;			///< C28_Gain2 (db)
	long C28_Gain3;			///< C28_Gain3 (db)

	unsigned long	C29_Freq0;		///< C29_Freq0 (Hz)
	unsigned long	C29_Freq1;		///< C29_Freq1 (Hz)
	unsigned long	C29_Freq2;		///< C29_Freq2 (Hz)
	unsigned long	C29_Freq3;		///< C29_Freq3 (Hz)

	long C29_Gain0;			///< C29_Gain0 (db)
	long C29_Gain1;			///< C29_Gain1 (db)
	long C29_Gain2;			///< C29_Gain2 (db)
	long C29_Gain3;			///< C29_Gain3 (db)

	unsigned long	C30_Freq0;		///< C30_Freq0 (Hz)
	unsigned long	C30_Freq1;		///< C30_Freq1 (Hz)
	unsigned long	C30_Freq2;		///< C30_Freq2 (Hz)
	unsigned long	C30_Freq3;		///< C30_Freq3 (Hz)

	long C30_Gain0;			///< C30_Gain0 (db)
	long C30_Gain1;			///< C30_Gain1 (db)
	long C30_Gain2;			///< C30_Gain2 (db)
	long C30_Gain3;			///< C30_Gain3 (db)

	unsigned long	C31_Freq0;		///< C31_Freq0 (Hz)
	unsigned long	C31_Freq1;		///< C31_Freq1 (Hz)
	unsigned long	C31_Freq2;		///< C31_Freq2 (Hz)
	unsigned long	C31_Freq3;		///< C31_Freq3 (Hz)

	long C31_Gain0;			///< C31_Gain0 (db)
	long C31_Gain1;			///< C31_Gain1 (db)
	long C31_Gain2;			///< C31_Gain2 (db)
	long C31_Gain3;			///< C31_Gain3 (db)

	//int32	ret_val;
}
TstVoipToneCfg;
*/

TstVoipToneCfg customToneTable[] =
{
/* original
	{0, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, 13, 13, 0, 0},		// USA dial tone
	{0, 0, 1, 2000, 0, 0, 0, 4000, 0, 0, 0, 0, 2, 440, 480, 0, 0, 13, 13, 0, 0},		// USA ring back tone
	{2, 0, 3, 500, 500, 500, 0, 0, 0, 1000, 0, 0, 3, 950, 1400, 1750, 0, 7, 7, 7, 0},// USA sit-no circuit tone
	{2, 2, 3, 500, 500, 500, 0, 0, 0, 1000, 0, 0, 3, 950, 1400, 1750, 0, 7, 7, 7, 0},// USA sit-no circuit tone (2 cycle)
	{0, 0, 1, 500, 0, 0, 0, 500, 0, 0, 0, 0, 2, 480, 620, 0, 0, 7, 7, 0, 0},			// USA busy tone
	{0, 0, 2, 500, 1000, 0, 0, 500, 1000, 0, 0, 0, 2, 330, 880, 0, 0, 7, 7, 0, 0},	// tone with 2 cadence
	{0, 0, 1, 1000, 0, 0, 0, 1000, 0, 0, 0, 0, 2, 330, 660, 0, 0, 7, 7, 0, 0},		// tone with 1 cadence
	{1, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 425, 25, 0, 0, 7, 7, 0, 0}			// AUSTRALIA dial tone
*/
	{0, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, 13, 13, 0, 0},		// USA dial tone
	{0, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, 13, 13, 0, 0},		// USA ring back tone
	{2, 0, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, 7, 7, 7, 0},// USA sit-no circuit tone
	{2, 2, 3, 500, 0, 500, 0, 500, 1000, 0, 0, 0, 3, 950, 1400, 1750, 0, 7, 7, 7, 0},// USA sit-no circuit tone (2 cycle)
	{0, 0, 1, 500, 500, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, 7, 7, 0, 0},			// USA busy tone
	{0, 0, 2, 500, 500, 1000, 1000, 0, 0, 0, 0, 0, 2, 330, 880, 0, 0, 7, 7, 0, 0},	// tone with 2 cadence
	{0, 0, 1, 1000, 1000, 0, 0, 0, 0, 0, 0, 0, 2, 330, 660, 0, 0, 7, 7, 0, 0},		// tone with 1 cadence
	{1, 0, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 2, 425, 25, 0, 0, 7, 7, 0, 0},			// AUSTRALIA dial tone
	{4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, 10, 0, 0, 0},	// dial tone "DialT from C1"
	{4, 0, 4, 100, 100, 100, 100, 100, 100, 400, 400, 0, 1, 450, 0, 0, 0, 10, 0, 0, 0, 450, 0, 0, 0, 10, 0, 0, 0, 450, 0, 0, 0, 10, 0, 0, 0, 450, 0, 0, 0, 10, 0, 0, 0},		// roh tone "NUT from C1"
	{4, 0, 1, 400, 4000, 0, 0, 0, 0, 0, 0, 0, 1, 450, 0, 0, 0, 13, 0, 0, 0},				// call waiting #1 tone "CallWaitingT from C1"
	{4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 400, 400, 425, 425, 14, 47, 14, 47},			// extend #2 tone "RecallDialT from C1"
	{4, 0, 4, 100, 100, 100, 100, 100, 100, 0, 0, 0, 2, 350, 440, 0, 0, 13, 13, 0, 0, 350, 440, 0, 0, 13, 13, 0, 0, 350, 440, 0, 0, 13, 13, 0, 0, 350, 440, 0, 0, 13, 13, 0, 0},			// extend #3 tone usa second dial tone
	{4, 0, 17, 100, 100, 100, 100, 100, 100, 100, 100, 0, 1, 480, 0, 0, 0, 16, 0, 0, 0, 480, 0, 0, 0, 15, 0, 0, 0, 480, 0, 0, 0, 14, 0, 0, 0, 480, 0, 0, 0, 13, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, 12, 0, 0, 0, 480, 0, 0, 0, 11, 0, 0, 0, 480, 0, 0, 0, 10, 0, 0, 0, 480, 0, 0, 0, 9, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, 8, 0, 0, 0, 480, 0, 0, 0, 7, 0, 0, 0, 480, 0, 0, 0, 6, 0, 0, 0, 480, 0, 0, 0, 5, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, 4, 0, 0, 0, 480, 0, 0, 0, 3, 0, 0, 0, 480, 0, 0, 0, 2, 0, 0, 0, 480, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 480, 0, 0, 0, 0, 0, 0, 0},			// extend #4 tone uk howler tone
	{4, 0, 32, 100, 100, 100, 100, 100, 100, 100, 100, 0, 1, 480, 0, 0, 0, 16, 0, 0, 0, 480, 0, 0, 0, 16, 0, 0, 0, 480, 0, 0, 0, 15, 0, 0, 0, 480, 0, 0, 0, 15, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, 14, 0, 0, 0, 480, 0, 0, 0, 14, 0, 0, 0, 480, 0, 0, 0, 13, 0, 0, 0, 480, 0, 0, 0, 13, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, 12, 0, 0, 0, 480, 0, 0, 0, 12, 0, 0, 0, 480, 0, 0, 0, 11, 0, 0, 0, 480, 0, 0, 0, 11, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, 10, 0, 0, 0, 480, 0, 0, 0, 10, 0, 0, 0, 480, 0, 0, 0, 9, 0, 0, 0, 480, 0, 0, 0, 9, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, 8, 0, 0, 0, 480, 0, 0, 0, 8, 0, 0, 0, 480, 0, 0, 0, 7, 0, 0, 0, 480, 0, 0, 0, 7, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, 6, 0, 0, 0, 480, 0, 0, 0, 6, 0, 0, 0, 480, 0, 0, 0, 5, 0, 0, 0, 480, 0, 0, 0, 5, 0, 0, 0, 100, 100, 100, 100, 100, 100, 100, 100, 480, 0, 0, 0, 4, 0, 0, 0, 480, 0, 0, 0, 4, 0, 0, 0, 480, 0, 0, 0, 3, 0, 0, 0, 480, 0, 0, 0, 3, 0, 0, 0, 100, 100, 100, 100, 100, 100, 0, 0, 480, 0, 0, 0, 2, 0, 0, 0, 480, 0, 0, 0, 2, 0, 0, 0, 480, 0, 0, 0, 1, 0, 0, 0, 480, 0, 0, 0, 0, 0, 0, 0}			// extend #5 tone "32cad"

};

void SetCustomTone(int test)
{
	int cust_idx;
	
	st_ToneCfgParam custom_tone[TONE_CUSTOMER_MAX];
	
	voipCfgParam_t VoIPCfg;
	
	
	memcpy(custom_tone, customToneTable, TONE_CUSTOMER_MAX*sizeof(st_ToneCfgParam));
	
	for (cust_idx=0; cust_idx < TONE_CUSTOMER_MAX; cust_idx++)
	{
		rtk_SetCustomTone(cust_idx, &custom_tone[cust_idx]);
	}	
	
#if 0
	VoIPCfg.tone_of_country = 13; //Custom
	VoIPCfg.tone_of_custdial = 0;
	VoIPCfg.tone_of_custring = 1;
	VoIPCfg.tone_of_custbusy = 2;
	VoIPCfg.tone_of_custwaiting = 3;

	rtk_SetCountry(&VoIPCfg);
#else	
	rtk_SetCountryTone(COUNTRY_CUSTOMER);
	
	//int32 rtk_UseCustomTone(uint8 dial, uint8 ringback, uint8 busy, uint8 waiting)

	if (test == 0)
		rtk_UseCustomTone(0, 0, 0, 0);
	else if (test == 1)
		rtk_UseCustomTone(1, 1, 1, 1);
	else if (test == 2)
		rtk_UseCustomTone(2, 2, 2, 2);
	else if (test == 3)
		rtk_UseCustomTone(3, 3, 3, 3);
	else if (test == 4)
		rtk_UseCustomTone(4, 4, 4, 4);
	else if (test == 5)
		rtk_UseCustomTone(5, 5, 5, 5);
	else if (test == 6)
		rtk_UseCustomTone(6, 6, 6, 6);
	else if (test == 7)
		rtk_UseCustomTone(7, 7, 7, 7);
#endif
}

int tone_main(int argc, char *argv[])
{
	uint32 val;
	
	if (argc == 3)
	{
		switch (atoi(argv[1]))
		{
			case 0:
				// argv[2]: 0 ~ 12: country, 13: customize
				printf("rtk_SetCountryTone...\n");
				rtk_SetCountryTone(atoi(argv[2]));
				break;
				
			case 1:
				// argv[2]: 0 ~ 12: country, 13: customize
				printf("rtk_SetCountryImpedance...\n");
				rtk_SetCountryImpedance(atoi(argv[2]));
				break;
			
			case 2:
				printf("rtk_SetImpedance...\n");
				rtk_SetImpedance(atoi(argv[2]));
				/* 
					0: 600
					1: 900
					2: 250+(750||150nf)
					3: 320+(1150||230nf)
					4: 350+(1000||210nf)
				*/
				break;
				
			case 3:
				printf("SetCustomTone...\n");
				SetCustomTone(atoi(argv[2]));
				break;
			
			default:
				printf("Error input\n");
				break;
		}
	}
	else
	{
		printf("tone test error, please see the rtk_voip/test/tone.c for detail!\n");
	}

	return 0;
}

