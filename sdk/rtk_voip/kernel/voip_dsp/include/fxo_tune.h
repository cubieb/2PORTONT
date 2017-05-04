

/* FXO tune function prototype */

void test_tone_init(int chid);
void test_tone_gen(int chid, Word16* buffPtr, int sample_len);
void tone_analysis_init(int chid);
int tone_analysis(int chid, unsigned short* page_addr, unsigned int N_len);
int dBloss(int chid, int in1, int in2);
void FXO_tune_test_tone_cntr_add(int chid);
int FXO_tune_test_tone_cntr_get(int chid);
int FXO_tune_test_flag_get(int chid);
int FXO_tune_test_tone_pwr_get(int chid);
void FXO_tune_ERL_save(int chid, int erl);
void FXO_tune_test_tone_pwr_update(int chid, int pwr);


/* Start/Stop FXO tune timer */
void FXO_tune_timer_start(void);
void FXO_tune_timer_stop( void );

/* Init/Enable/Disable FXO tune */
void FXO_tune_init(int chid);
void FXO_tune_enable(int chid);
void FXO_tune_disable(int chid);

/* Check if FXO tune is done */
int FXO_tune_get_test_done(int chid);

/* Get the best ERL(dB), and best ERL impedance index */
int FXO_tune_get_best_ERL(int chid);
int FXO_tune_get_best_ERL_idx(int chid);

/* Get current impedance tuning index */
int FXO_tune_get_current_idx(int chid);

/* Get ERL by impedance tuning index */
int FXO_tune_get_ERL(int chid, int index);

/* Dump ERL of all impedance index */
void FXO_tune_dump_ERL(int chid);

/* Set busy tone time out, default is 10sec */
void FXO_tune_busy_timeout_set(int chid, int msec);

/* Set/Get test tone duration, default is 1sec */
void FXO_tune_test_duration_set(int chid, int msec);
int FXO_tune_test_duration_get(int chid);	// unit: msec
