#include <stdio.h>

typedef struct {
	char name[20];
	int (*func)(int, char **);
} TVoIPBox;

extern int dbg_main(int argc, char *argv[]);
extern int fskgen_main(int argc, char *argv[]);
extern int pulse_dial_main(int argc, char *argv[]);
extern int cp3_measure_main(int argc, char *argv[]);
extern int led_main(int argc, char *argv[]);
extern int phonerecord_main(int argc, char *argv[]);
extern int reg_main(int argc, char *argv[]);
extern int ram_main(int argc, char *argv[]);
extern int slic_reset_main(int argc, char *argv[]);
extern int switchmii_main(int argc, char *argv[]);
extern int test_main(int argc, char *argv[]);
extern int vmwigen_main(int argc, char *argv[]);
extern int voicerecord_main(int argc, char *argv[]);
extern int voiceplay_main(int argc, char *argv[]);
extern int ec_test_main(int argc, char *argv[]);
extern int rtcp_main(int argc, char *argv[]);
extern int rtcp_logger_main(int argc, char *argv[]);
extern int iphone_test_main(int argc, char *argv[]);
extern int ring_test_main(int argc, char *argv[]);
extern int clone_mac_main(int argc, char *arg[]);
extern int gpio_main(int argc, char *argv[]);
extern int bandwidth_mgr_main(int argc, char *argv[]);
extern int send_2833_main(int argc, char *argv[]);
extern int power_main(int argc, char *argv[]);
extern int netmask_main(int argc, char *argv[]);
extern int voip_cli_main(int argc, char *argv[]);
extern int tone_main(int argc, char *argv[]);
extern int lb_test_main(int argc, char *argv[]);

TVoIPBox voip_box[] = {
#ifdef VOIPBOX_DBG
	{"dbg", dbg_main},
#endif
#ifdef VOIPBOX_FSKGEN
	{"fskgen", fskgen_main},
#endif
#ifdef VOIPBOX_PULSE_DIAL
	{"pulse_dial", pulse_dial_main},
#endif
#ifdef VOIPBOX_CP3_MEASURE
	{"cp3_measure", cp3_measure_main},
#endif
#ifdef VOIPBOX_LED
	{"led", led_main},
#endif
#ifdef VOIPBOX_PHONERECORD
	{"phonerecord", phonerecord_main},
#endif
#ifdef VOIPBOX_VOICEPLAY
	{"voiceplay", voiceplay_main},
#endif
#ifdef VOIPBOX_REG
	{"reg", reg_main},
#ifdef VOIPBOX_RAM
	{"ram", ram_main},
#endif
#endif
#ifdef VOIPBOX_SLIC_RESET
	{"slic_reset", slic_reset_main},
#endif
#ifdef VOIPBOX_SWITCHMII
	{"switchmii", switchmii_main},
#endif
#ifdef VOIPBOX_TEST
	{"crash", test_main},
#endif
#ifdef VOIPBOX_VMWIGEN
	{"vmwigen", vmwigen_main},
#endif
#ifdef VOIPBOX_VOICERECORD
	{"voicerecord", voicerecord_main},
#endif
#ifdef VOIPBOX_EC_TEST
	{"ec_test", ec_test_main},
#endif
#ifdef VOIPBOX_RTCP_STATISTIC
	{"rtcp_statistic", rtcp_main},
#endif
#ifdef VOIPBOX_RTCP_LOGGER
	{"rtcp_logger", rtcp_logger_main},
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
	{"iphone_test", iphone_test_main},
#endif
#ifdef VOIPBOX_RING_TEST
	{"ring_test", ring_test_main},
#endif
#ifdef VOIPBOX_CLONE_MAC
	{"clone_mac", clone_mac_main},
#endif
#ifdef VOIPBOX_GPIO
	{"gpio_init", gpio_main},
	{"gpio_read", gpio_main},
	{"gpio_write", gpio_main},
#endif
#ifdef VOIPBOX_BANDWIDTH_MGR
	{"bandwidth_mgr", bandwidth_mgr_main},
#endif
#ifdef VOIPBOX_SEND_2833
	{"send_2833", send_2833_main},
#endif
#ifdef VOIPBOX_POWER
	{"power", power_main},
#endif
#ifdef VOIPBOX_NETMASK
	{"netmask", netmask_main},
#endif
#ifdef VOIPBOX_CLI
	{"voipcli", voip_cli_main},
#endif
#ifdef VOIPBOX_TONE
	{"tone", tone_main},
#endif
#ifdef VOIPBOX_LBTEST
	{"lb_test", lb_test_main},
#endif
	{"", NULL}
};

int main(int argc, char *argv[])
{
	int i;

	for (i=0; voip_box[i].func; i++)
	{
		if (strcmp(argv[0], voip_box[i].name) == 0)
		{
			return voip_box[i].func(argc, argv);
		}
	}

	printf("voip box: cmd %s is not support\n", argv[0]);
	return 0;
}

