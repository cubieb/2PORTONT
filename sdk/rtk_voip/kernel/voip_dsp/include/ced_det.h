#ifndef CED_DET_H
#define CED_DET_H
/* ced.c function prototype */

char CED_routine(int chid); /* return value: 1: fax, 2: local-modem, 3: remote-modem */
void ced_time_power_ths_set(int power);	/* set the power threshold for ced tone detection */
void modem_time_power_local_ths_set(int power); /* set the power threshold for local modem tone detection */
void modem_time_power_remote_ths_set(int power); /* set the power threshold for remote modeom tone detection */

void Init_CED_Det(unsigned char CH);

/* answer tone   */
typedef struct
{
	unsigned cng_det_local_on:1  /*__attribute__((aligned(16)))*/;
	unsigned ans_det_local_on:1;
	unsigned ansam_det_local_on:1;
	unsigned ansbar_det_local_on:1;
	unsigned ansambar_det_local_on:1;
	unsigned bellans_2225_det_local_on:1;
	unsigned vdot22_2250_det_local_on:1;
	unsigned v8bis_cre_det_local_on:1;
	unsigned v21flag_det_local_on:1;
	unsigned v21dis_det_local_on:1;
	unsigned v21dcn_det_local_on:1;
	unsigned v21ecm_ppsmps_det_local_on:1;
	unsigned v21ecm_ppseop_det_local_on:1;
	unsigned v21ecm_rnr_det_local_on:1;
	unsigned v21channel_2_det_local_on:1;
	unsigned v21channel_1_det_local_on:1;
	unsigned v23_det_local_on:1;
	unsigned bell202ans_det_local_on:1;
	unsigned bell202cp_det_local_on:1;

	unsigned cng_det_remote_on:1;
	unsigned ans_det_remote_on:1;
	unsigned ansam_det_remote_on:1;
	unsigned ansbar_det_remote_on:1;
	unsigned ansambar_det_remote_on:1;
	unsigned bellans_2225_det_remote_on:1;
	unsigned vdot22_2250_det_remote_on:1;
	unsigned v8bis_cre_det_remote_on:1;
	unsigned v21flag_det_remote_on:1;
	unsigned v21dis_det_remote_on:1;
	unsigned v21dcn_det_remote_on:1;
	unsigned v21ecm_ppsmps_det_remote_on:1;
	unsigned v21ecm_ppseop_det_remote_on:1;
	unsigned v21ecm_rnr_det_remote_on:1;
	unsigned v21channel_2_det_remote_on:1;
	unsigned v21channel_1_det_remote_on:1;
	unsigned v23_det_remote_on:1;
	unsigned bell202ans_det_remote_on:1;
	unsigned bell202cp_det_remote_on:1;

	//TstVoIPAnsDetStat stVoIPAnsDetStat[2];
}TstVoipAnsDetCtrl;//answer tone detection control

extern TstVoipAnsDetCtrl stVoipAnsDetCtrl[];

#define ANSWER_TONE_LOCAL_CNG 1
#define ANSWER_TONE_LOCAL_ANS 2
#define ANSWER_TONE_LOCAL_ANSAM 4
#define ANSWER_TONE_LOCAL_ANSBAR 8
#define ANSWER_TONE_LOCAL_ANSAMBAR 16
#define ANSWER_TONE_LOCAL_BELLANS 32
#define ANSWER_TONE_LOCAL_V22 64
#define ANSWER_TONE_LOCAL_V8BIS 128


#define ANSWER_TONE_REMOTE_CNG 256
#define ANSWER_TONE_REMOTE_ANS 512
#define ANSWER_TONE_REMOTE_ANSAM 1024
#define ANSWER_TONE_REMOTE_ANSBAR 2048
#define ANSWER_TONE_REMOTE_ANSAMBAR 4096
#define ANSWER_TONE_REMOTE_BELLANS 8192
#define ANSWER_TONE_REMOTE_V22 16384
#define ANSWER_TONE_REMOTE_V8BIS 32768

#define ANSWER_TONE_LOCAL_V21FLAG 0x10000
#define ANSWER_TONE_REMOTE_V21FLAG 0x20000

#define ANSWER_TONE_LOCAL_V21DIS 0x40000
#define ANSWER_TONE_REMOTE_V21DIS 0x80000

#define ANSWER_TONE_LOCAL_V21DCN 0x100000
#define ANSWER_TONE_REMOTE_V21DCN 0x200000

#define ANSWER_TONE_LOCAL_V21CHANNEL_2 0x400000
#define ANSWER_TONE_REMOTE_V21CHANNEL_2 0x800000

#define ANSWER_TONE_LOCAL_V21CHANNEL_1 0x1000000
#define ANSWER_TONE_REMOTE_V21CHANNEL_1 0x2000000

#define ANSWER_TONE_LOCAL_V23 0x4000000
#define ANSWER_TONE_REMOTE_V23 0x8000000

#define ANSWER_TONE_LOCAL_BELL202ANS 0x10000000
#define ANSWER_TONE_REMOTE_BELL202ANS 0x20000000

#define ANSWER_TONE_LOCAL_BELL202CP 0x40000000
#define ANSWER_TONE_REMOTE_BELL202CP 0x80000000
//====================================================
// config 2 for ECM and others
//====================================================
#define ANSWER_TONE_LOCAL_ECM_PPSMPS 	0x1
#define ANSWER_TONE_REMOTE_ECM_PPSMPS 	0x2
#define ANSWER_TONE_LOCAL_ECM_PPSEOP 	0x4
#define ANSWER_TONE_REMOTE_ECM_PPSEOP 	0x8
#define ANSWER_TONE_LOCAL_ECM_RNR 	0x10
#define ANSWER_TONE_REMOTE_ECM_RNR 	0x20






int set_answer_tone_det(unsigned int chid, unsigned int config1, unsigned int config2);	// duncan
void answer_tone_det(unsigned int chid, unsigned short* page_addr, int dir);
int reinit_answer_tone_det(unsigned int chid);

#endif

