/*****************************************************************************
 * Copyright (C) 2004,2005,2006,2007,2008 Katalix Systems Ltd
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>
#include <netdb.h>
#include <errno.h>
#include <syscall.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/utsname.h>

#include <wait.h>

#include "if_sppp.h"

#include "usl.h"
#include "l2tp_main.h"
#include "l2tp_rpc.h"
#include "l2tp_private.h"


char *l2tp_state[L2TP_ST_NUM] = {
	"dead",
	"tunnel_init",
	"tunnel_establish",
	"session_init",
	"session_establish",
};

char l2tp_host_name[]="L2TPLAC";
#if 0
extern int l2tp_config_parse(FILE **fp);

#define L2TP_PID_FILENAME	"/var/run/openl2tpd.pid"
#define L2TP_CONFIG_FILENAME	"/etc/openl2tpd.conf"

/* Patch information.
 * Each patch adds its version number to this array.
 */
static int l2tp_installed_patches[] = {
	0,			/* end of list */
};

/* Private variables */

static char			*l2tp_opt_pid_filename = L2TP_PID_FILENAME;
#ifdef L2TP_FEATURE_LOCAL_CONF_FILE
static char			*l2tp_opt_config_filename = NULL;
#endif
#ifdef L2TP_FEATURE_LOCAL_STAT_FILE
static int			l2tp_opt_write_status_files;
#endif
static pid_t			l2tp_my_pid;
#endif
static int			l2tp_rand_fd = -1;
#ifdef L2TP_DMALLOC
static unsigned long 		l2tp_dmalloc_mark;
#endif

int				l2tp_opt_nodaemon = 0;
uint32_t			l2tp_opt_trace_flags;
int				l2tp_opt_debug;
int				l2tp_opt_udp_port;
int				l2tp_opt_flags;
int				l2tp_opt_remote_rpc;
int				l2tp_opt_log_facility = LOG_DAEMON;
int				l2tp_opt_throttle_ppp_setup_rate;
void		 		*l2tp_sig_notifier;
uint16_t			l2tp_firmware_revision;
char				*l2tp_kernel_version;
char				*l2tp_cpu_name;

struct l2tp_stats		l2tp_stats;

/* allow plugins to inhibit loading of the default ppp_unix plugin */
int				l2tp_inhibit_default_plugin;


/* profile management hooks */
int (*l2tp_profile_created_hook)(enum l2tp_profile_type type, const char *name) = NULL;
int (*l2tp_profile_deleted_hook)(enum l2tp_profile_type type, const char *name) = NULL;
int (*l2tp_profile_modified_hook)(enum l2tp_profile_type type, const char *name) = NULL;

void l2tp_init(void);
void l2tp_cleanup(void);

static struct in_addr get_ip_address(char *name);
static int get_l2tp_tunnel_profile_msg_data(struct l2tp_api_tunnel_profile_msg_data *msg, struct sppp *sppp);
static int get_l2tp_tunnel_msg_data(struct l2tp_api_tunnel_msg_data *msg, struct sppp *sppp);
static int get_l2tp_session_profile_msg_data(struct l2tp_api_session_profile_msg_data *msg, struct sppp *sppp);
static int get_l2tp_session_msg_data(struct l2tp_api_session_msg_data *msg, struct sppp *sppp);

#if 0
static void l2tp_init(void);
static void l2tp_cleanup(void);
static int l2tp_parse_args(int argc, char **argv);

static void usage(char **argv, int exit_code)
{
	fprintf(stderr, "Usage: %s [-f] [-D] [-d debugmask] [-u udpport] [-p plugin]\n\t"
#ifdef L2TP_FEATURE_RPC_MANAGEMENT
		"[-R] "
#endif
		"[-L facility] [-h]\n", argv[0]);
	fprintf(stderr, 
		"\t-h			This message\n"
		"\t-f			Run in foreground\n"
		"\t-u <port>		UDP port\n"
		"\t-p <plugin>		Load plugin\n"
#ifdef L2TP_FEATURE_LOCAL_CONF_FILE
		"\t-c <config-file>	Use specified file instead of " L2TP_CONFIG_FILENAME "\n"
#endif
#ifdef L2TP_FEATURE_LOCAL_STAT_FILE
		"\t-S                   Write peer/tunnel/session/profile status files\n"
#endif
#ifdef L2TP_FEATURE_RPC_MANAGEMENT
		"\t-R			Enable remote management (SUN RPC)\n"
#endif
		"\t-P <pid-filename>	Use specified file instead of " L2TP_PID_FILENAME "\n"
		"\t-L <facility>\t	Send syslog messages to the specified facility\n"
 		"\t			local0..local7\n"
		"\t-D			Enable debug messages in new objects created\n"
		"\t                     unless overridden by trace_flags\n"
		"\t-d <flags>		Enable debug trace messages\n"
		"\t-y <num>		Throttle PPP setup rate by limiting the number of PPP"
		"\t\t\t\tprocesses trying to establish simultaneously. Default 0 (unlimited)\n"
);
	exit(exit_code);
}

int main(int argc, char **argv)
{
	int result;
	//int log_flags = LOG_PID;
	struct utsname name;
	int fd;
	char pidstr[10];

	/* Parse arguments */
	result = l2tp_parse_args(argc, argv);
	if (result < 0) {
		fprintf(stderr, "Invalid argument\n");
		return result;
	}

	/* Create a pid file, error if already exists */
	fd = open(l2tp_opt_pid_filename, O_WRONLY | O_CREAT | O_EXCL, 0660);
	if (fd < 0) {
		if (errno == EEXIST) {
			fprintf(stderr, "File %s already exists. Is %s already running?\n",
				l2tp_opt_pid_filename, argv[0]);
		} else {
			fprintf(stderr, "File %s: %m", l2tp_opt_pid_filename);
		}
		exit(1);
	}

	/* Get system kernel info, which is used to build our vendor name */
	result = uname(&name);
	if (result < 0) {
		fprintf(stderr, "Failed to get system version info: %m");
		return result;
	}
	l2tp_kernel_version = strdup(name.release);
	l2tp_cpu_name = strdup(name.machine);
	if ((l2tp_kernel_version == NULL) || (l2tp_cpu_name == NULL)) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	l2tp_firmware_revision = (((L2TP_APP_MAJOR_VERSION & 0xff) << 8) |
				  ((L2TP_APP_MINOR_VERSION & 0xff) << 0));

	/* Become a daemon */
	if (!l2tp_opt_nodaemon) {
		usl_daemonize();
	}

	/* We write the PID file AFTER the double-fork */
	sprintf(&pidstr[0], "%d", getpid());
	if (write(fd, &pidstr[0], strlen(pidstr)) < 0)
		syslog(LOG_WARNING, "Failed to write pid file %s", l2tp_opt_pid_filename);
	close(fd);

#if 0
	/* Open the syslog */
	if (l2tp_opt_debug) {
		log_flags |= LOG_NDELAY;
	}
	openlog("openl2tpd", log_flags, l2tp_opt_log_facility);
	if (l2tp_opt_debug) {
		setlogmask(LOG_UPTO(LOG_DEBUG));
	} else {
		setlogmask(LOG_UPTO(LOG_INFO));
	}
	l2tp_log(LOG_INFO, "Start, trace_flags=%08x%s", l2tp_opt_trace_flags,
		 l2tp_opt_debug ? " (debug enabled)" : "");
#endif
	/* Init the app */
	l2tp_init();

	/* Main loop - USL takes care of it */
	usl_main_loop();

	return 0;
}
#endif

void l2tp_mem_dump(int level, void *data, int data_len, int hex_only)
{
	int x, y;
	unsigned char *bytep;
	char cbuf[80];
	char nbuf[80];
	char *p;

	bytep = data;
	for (y = 0; y < data_len; y += 16) {
		memset(&cbuf[0], 0, sizeof(cbuf));
		memset(&nbuf[0], 0, sizeof(nbuf));
		p = &nbuf[0];

		for (x = 0; x < 16; x++, bytep++) {
			if ((x + y) >= data_len) {
				break;
			}
			cbuf[x] = isprint(*bytep) ? *bytep : '.';
			p += sprintf(p, "%02x ", *bytep);
		}
		l2tp_log(level, "%8d: %-48s  %s", y, nbuf, hex_only ? "" : cbuf);
	}
}

char *l2tp_buffer_hexify(void *buf, int buf_len)
{
	static char string_buf[80];
	int max_len = (buf_len < ((sizeof(string_buf) - 1) / 3)) ? buf_len : (sizeof(string_buf) - 1) / 3;
	int index;
	char *bufp = &string_buf[0];
	unsigned char *datap = (unsigned char *) buf;

	/* We use 3 chars in the output buffer per octet */
	for (index = 0; index < max_len; index++) {
		bufp += sprintf(bufp, "%02x ", *datap);
		datap++;
	}
	*bufp = '\0';
	
	return &string_buf[0];
}
 
#if 0
static int l2tp_parse_log_facility(char *arg)
{
	static const struct {
		char *name;
		int facility;
	} codes[] = {
		{ "local0", LOG_LOCAL0 },
		{ "local1", LOG_LOCAL1 },
		{ "local2", LOG_LOCAL2 },
		{ "local3", LOG_LOCAL3 },
		{ "local4", LOG_LOCAL4 },
		{ "local5", LOG_LOCAL5 },
		{ "local6", LOG_LOCAL6 },
		{ "local7", LOG_LOCAL7 },
		{ NULL, 0 }
	};
	int index;

	if (arg == NULL) {
		return -EINVAL;
	}

	for (index = 0; codes[index].name != NULL; index++) {
		if (strcasecmp(arg, codes[index].name) == 0) {
			return codes[index].facility;
		}
	}

	fprintf(stderr, "Expecting local[0-7]\n");
	return -EINVAL;
}

static int l2tp_parse_args(int argc, char **argv)
{
	int opt;
	int result = 0;

	while((opt = getopt(argc, argv, "p:d:u:L:c:fRDSy:P:h")) != -1) {
		switch(opt) {
		case 'h':
			usage(argv, 0);
			break;
		case 'f':
			l2tp_opt_nodaemon = 1;
			break;
		case 'D':
			l2tp_opt_debug = 1;
			break;
		case 'd':
			if (l2tp_parse_debug_mask(&l2tp_opt_trace_flags, optarg, 1) < 0) {
				exit(1);
			}
			break;
		case 'p':
			if (l2tp_plugin_add(optarg) < 0) {
				exit(1);
			}
			break;
#ifdef L2TP_FEATURE_LOCAL_CONF_FILE
		case 'c':
			l2tp_opt_config_filename = optarg;
			break;
#endif
#ifdef L2TP_FEATURE_LOCAL_STAT_FILE
		case 'S':
			l2tp_opt_write_status_files = 1;
			break;
#endif
		case 'R':
			l2tp_opt_remote_rpc = 1;
			break;
		case 'L':
			l2tp_opt_log_facility = l2tp_parse_log_facility(optarg);
			if (l2tp_opt_log_facility < 0) {
				result = -EINVAL;
			}
			break;
		case 'u':
			sscanf(optarg, "%d", &l2tp_opt_udp_port);
			L2TP_DEBUG(L2TP_API, "Using port %d", l2tp_opt_udp_port);
			break;
		case 'y':
			sscanf(optarg, "%d", &l2tp_opt_throttle_ppp_setup_rate);
			break;
		case 'P':
			l2tp_opt_pid_filename = optarg;
			break;
		default:
			usage(argv, 1);
		}
	}

	return result;
}

static void l2tp_toggle_debug(void)
{
#ifdef USL_DMALLOC
	dmalloc_log_unfreed();
	dmalloc_log_stats();
	dmalloc_log_heap_map();
#endif

	l2tp_opt_debug = !l2tp_opt_debug;
}

static void l2tp_dmalloc_dump(void)
{
#ifdef L2TP_DMALLOC
	dmalloc_log_changed(l2tp_dmalloc_mark, 1, 0, 1);
	l2tp_dmalloc_mark = dmalloc_mark();
	dmalloc_message("DMALLOC MARK set to %lu\n", l2tp_dmalloc_mark);
#endif
}
#endif

/* Warn about features not yet supported from one place so they're
 * easier to find...
 */
void l2tp_warn_not_yet_supported(const char *what)
{
	l2tp_log(LOG_WARNING, "WARNING: %s not yet supported", what);
}

#if 0
/* die - clean up state and exit with the specified status.
 */
static void l2tp_die(void)
{
	static int exiting = 0;

	if (!exiting) {
		exiting = 1;
		l2tp_log(LOG_INFO, "Exiting");
		exit(1);
	} else {
		_exit(1);
	}
}

/* hangup - handle SIGHUP signal. Called from main loop.
 */
static void l2tp_hangup(void)
{
	l2tp_load_config_file();
}

/* This function is registered as a signal notifier with USL.
 */
static void l2tp_signal_handler(void *arg, int sig)
{
	switch (sig) {
	case SIGUSR1:
		l2tp_toggle_debug();
		break;
	case SIGUSR2:
		l2tp_dmalloc_dump();
		break;
	case SIGTERM:
		/* This is handled in the main loop */
		break;
	case SIGHUP:
		/* This is handled in the main loop */
		break;
	default:
		break;
	}
}
#endif

void l2tp_make_random_vector(void *buf, int buf_len)
{
	size_t count;

	count = usl_fd_read(l2tp_rand_fd, buf, buf_len);
	if ((count < 0) && (errno != EAGAIN)) {
		l2tp_log(LOG_ERR, "ERROR: problem reading /dev/urandom: %s", strerror(errno));
		exit(1);
	}
}

uint16_t l2tp_make_random_id(void)
{
	return (uint16_t) random();
}

int l2tp_random(int min, int max)
{
	float scale = (float) (max - min);

	return min + (int) (scale * rand() / (RAND_MAX + scale));
}

void l2tp_vlog(int level, const char *fmt, va_list ap)
{
	if (l2tp_opt_nodaemon) {
		vprintf(fmt, ap);
		printf("\n");
	} else {
		//vsyslog(level, fmt, ap);
	}
	DMALLOC_VMESSAGE(fmt, ap);
}

void l2tp_log(int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

static void l2tp_system_log(int level, const char *fmt, ...)
{
	if (l2tp_opt_trace_flags & L2TP_DEBUG_SYSTEM) {
		va_list ap;

		va_start(ap, fmt);
		l2tp_vlog(level, fmt, ap);
		va_end(ap);
	}
}

char *l2tp_system_time(void)
{
	time_t now;
	char *tstr;

	now = time(NULL);
	if (now == -1) {
		return NULL;
	}

	tstr = ctime(&now);
	if (tstr != NULL) {
		return strdup(tstr);
	}
	return NULL;
}

#if 0
/*****************************************************************************
 * Management interface
 *****************************************************************************/

bool_t l2tp_app_info_get_1_svc(struct l2tp_api_app_msg_data *msg, struct svc_req *req)
{
	int patches_len;

	msg->build_date = strdup(__DATE__);
	msg->build_time = strdup(__TIME__);
	msg->major = L2TP_APP_MAJOR_VERSION;
	msg->minor = L2TP_APP_MINOR_VERSION;
	msg->cookie = L2TP_APP_COOKIE;

	msg->features = 0;
#ifdef L2TP_FEATURE_LOCAL_CONF_FILE
	msg->features |= L2TP_API_APP_FEATURE_LOCAL_CONF_FILE;
#endif
#ifdef L2TP_FEATURE_LOCAL_STAT_FILE
	msg->features |= L2TP_API_APP_FEATURE_LOCAL_STAT_FILE;
#endif
#ifdef L2TP_FEATURE_LAC_SUPPORT
	msg->features |= L2TP_API_APP_FEATURE_LAC_SUPPORT;
#endif
#ifdef L2TP_FEATURE_LNS_SUPPORT
	msg->features |= L2TP_API_APP_FEATURE_LNS_SUPPORT;
#endif
#ifdef L2TP_FEATURE_RPC_MANAGEMENT
	msg->features |= L2TP_API_APP_FEATURE_RPC_MANAGEMENT;
#endif
#ifdef L2TP_FEATURE_LAIC_SUPPORT
	msg->features |= L2TP_API_APP_FEATURE_LAIC_SUPPORT;
#endif
#ifdef L2TP_FEATURE_LAOC_SUPPORT
	msg->features |= L2TP_API_APP_FEATURE_LAOC_SUPPORT;
#endif
#ifdef L2TP_FEATURE_LNIC_SUPPORT
	msg->features |= L2TP_API_APP_FEATURE_LNIC_SUPPORT;
#endif
#ifdef L2TP_FEATURE_LNOC_SUPPORT
	msg->features |= L2TP_API_APP_FEATURE_LNOC_SUPPORT;
#endif

	patches_len = sizeof(l2tp_installed_patches) - sizeof(l2tp_installed_patches[0]);
	msg->patches.patches_len = patches_len / sizeof(l2tp_installed_patches[0]);
	if (msg->patches.patches_len > 0) {
		msg->patches.patches_val = malloc(patches_len);
		if (msg->patches.patches_val == NULL) {
			return FALSE;
		}
		memcpy(msg->patches.patches_val, &l2tp_installed_patches[0], patches_len);
	}

	if ((msg->build_date == NULL) || (msg->build_time == NULL)) {
		return FALSE;
	}

	return TRUE;
}

bool_t l2tp_system_get_1_svc(struct l2tp_api_system_msg_data *msg, struct svc_req *req)
{
	int type;
	struct l2tp_api_system_msg_stats *stat;

	memset(msg, 0, sizeof(*msg));

	msg->config.trace_flags = l2tp_opt_trace_flags;
	msg->config.udp_port = l2tp_opt_udp_port;
	msg->config.flags = l2tp_opt_flags;

	/* catch uncopied data errors */
	USL_POISON_MEMORY(&msg->status.stats, 0xaa, sizeof(msg->status.stats));

	msg->status.stats.total_sent_control_frames = l2tp_stats.total_sent_control_frames;
	msg->status.stats.total_rcvd_control_frames = l2tp_stats.total_rcvd_control_frames;
	msg->status.stats.total_control_frame_send_fails = l2tp_stats.total_control_frame_send_fails;
	msg->status.stats.total_retransmitted_control_frames = l2tp_stats.total_retransmitted_control_frames;
	msg->status.stats.event_queue_full_errors = l2tp_stats.event_queue_full_errors;
	msg->status.stats.short_frames = l2tp_stats.short_frames;
	msg->status.stats.wrong_version_frames = l2tp_stats.wrong_version_frames;
	msg->status.stats.unexpected_data_frames = l2tp_stats.unexpected_data_frames;
	msg->status.stats.bad_rcvd_frames = l2tp_stats.bad_rcvd_frames;
	msg->status.stats.no_control_frame_resources = l2tp_stats.no_control_frame_resources;
	msg->status.stats.no_peer_resources = l2tp_stats.no_peer_resources;
	msg->status.stats.no_tunnel_resources = l2tp_stats.no_tunnel_resources;
	msg->status.stats.no_session_resources = l2tp_stats.no_session_resources;
	msg->status.stats.no_ppp_resources = l2tp_stats.no_ppp_resources;
	msg->status.stats.auth_fails = l2tp_stats.auth_fails;
	msg->status.stats.no_matching_tunnel_id_discards = l2tp_stats.no_matching_tunnel_id_discards;
	msg->status.stats.no_matching_session_id_discards = l2tp_stats.no_matching_session_id_discards;
	msg->status.stats.mismatched_tunnel_ids = l2tp_stats.mismatched_tunnel_ids;
	msg->status.stats.mismatched_session_ids = l2tp_stats.mismatched_session_ids;
	msg->status.stats.encode_message_fails = l2tp_stats.encode_message_fails;
	msg->status.stats.ignored_avps = l2tp_stats.ignored_avps;
	msg->status.stats.vendor_avps = l2tp_stats.vendor_avps;
	msg->status.stats.illegal_messages = l2tp_stats.illegal_messages;
	msg->status.stats.unsupported_messages = l2tp_stats.unsupported_messages;
	msg->status.stats.messages.messages_len = L2TP_API_MSG_TYPE_COUNT;
	msg->status.stats.messages.messages_val = calloc(L2TP_API_MSG_TYPE_COUNT, sizeof(*msg->status.stats.messages.messages_val));
	if (msg->status.stats.messages.messages_val == NULL) {
		msg->status.stats.messages.messages_len = 0;
	}
	
	stat = msg->status.stats.messages.messages_val;
	for (type = 0; type < msg->status.stats.messages.messages_len; type++) {
		stat->rx = l2tp_stats.messages[type].rx;
		stat->tx = l2tp_stats.messages[type].tx;
		stat->rx_bad = l2tp_stats.messages[type].rx_bad;
		stat++;
	}
	
	msg->status.stats.too_many_tunnels = l2tp_stats.too_many_tunnels;
	msg->status.stats.too_many_sessions = l2tp_stats.too_many_sessions;
	msg->status.stats.tunnel_setup_failures = l2tp_stats.tunnel_setup_failures;
	msg->status.stats.session_setup_failures = l2tp_stats.session_setup_failures;

	l2tp_tunnel_globals_get(msg);
	l2tp_session_globals_get(msg);

	return TRUE;
}

bool_t l2tp_system_modify_1_svc(struct l2tp_api_system_msg_data msg, int *result, struct svc_req *req)
{
	if (msg.config.flags & L2TP_API_CONFIG_FLAG_TRACE_FLAGS) {
		if (msg.config.trace_flags_mask == 0) {
			msg.config.trace_flags_mask = 0xffffffff;
		}
		l2tp_opt_trace_flags &= ~(msg.config.trace_flags_mask);
		l2tp_opt_trace_flags |= (msg.config.trace_flags & msg.config.trace_flags_mask);
	}
	if (msg.config.flags & L2TP_API_CONFIG_FLAG_RESET_STATISTICS) {
		memset(&l2tp_stats, 0, sizeof(l2tp_stats));
		msg.config.flags &= ~L2TP_API_CONFIG_FLAG_RESET_STATISTICS;
	}
	l2tp_tunnel_globals_modify(&msg, result);
	l2tp_session_globals_modify(&msg, result);

	*result = 0;

	/* Remember all non-default parameters */
	l2tp_opt_flags |= msg.config.flags;
	
	return TRUE;
}
#endif

/*****************************************************************************
 * Init and cleanup
 *****************************************************************************/

/* Removes any profiles and resets the default profiles.
 */
void l2tp_restore_default_config(void)
{
	l2tp_opt_flags = 0;

	//l2tp_ppp_reinit();
	l2tp_session_reinit();
	l2tp_tunnel_reinit();
	l2tp_peer_reinit();
}

#if 0
/* Read local config file.
 */
void l2tp_load_config_file(void)
{
#ifdef L2TP_FEATURE_LOCAL_CONF_FILE
	FILE *f;

	if (l2tp_opt_config_filename != NULL) {
		f = fopen(l2tp_opt_config_filename, "r");
		if (f == NULL) {
			l2tp_log(LOG_ERR, "Unable to open %s", l2tp_opt_config_filename);
			exit(1);
		}
	} else {
		f = fopen(L2TP_CONFIG_FILENAME, "r");
	}
	if (f != NULL) {
		l2tp_log(LOG_INFO, "Using config file: %s", 
			 l2tp_opt_config_filename ? l2tp_opt_config_filename : L2TP_CONFIG_FILENAME);
		l2tp_restore_default_config();
		if (l2tp_config_parse(&f) < 0) {
			exit(1);
		}
		fclose(f);
	}
#endif
}
#endif

/*static */void l2tp_init(void)
{
	unsigned int seed;

	l2tp_log(LOG_INFO, "OpenL2TP V%d.%d, %s %s\n",
		 L2TP_APP_MAJOR_VERSION, L2TP_APP_MINOR_VERSION,
		 L2TP_APP_COPYRIGHT_INFO, L2TP_APP_VENDOR_INFO);

	atexit(l2tp_cleanup);
	L2TP_DEBUG(L2TP_FUNC, "%s (%s %s): trace flags = %08x", __func__, __DATE__, __TIME__, l2tp_opt_trace_flags);

	//usl_signal_terminate_hook = l2tp_die;
	//usl_signal_hangup_hook = l2tp_hangup;
	//usl_signal_init();
	usl_fd_init();
	usl_timer_init();
	//usl_pid_init();
	//l2tp_plugin_init();
	l2tp_net_init();

	l2tp_rand_fd = open("/dev/urandom", O_RDONLY);
	if (l2tp_rand_fd < 0) {
		fprintf(stderr, "No /dev/urandom device found. Exiting.\n");
		exit(1);
	}

	/* Set the random() seed. We use random() rather than
	 * l2tp_make_random_vector() to generate tunnel/session ids
	 * because random() is faster and we potentially allocate
	 * tunnels/sessions very often. But for better randomness, set
	 * the seed using a good random value.
	 */
	l2tp_make_random_vector(&seed, sizeof(seed));
	srandom(seed);
	
	//usl_signal_notifier_add(l2tp_signal_handler, NULL);

	l2tp_avp_init();
	l2tp_peer_init();
#ifdef L2TP_FEATURE_RPC_MANAGEMENT
	//l2tp_api_init();
#endif
	//l2tp_event_init();
	l2tp_xprt_init();
	l2tp_tunnel_init();
	l2tp_session_init();
	//l2tp_ppp_init();

#ifdef L2TP_FEATURE_LOCAL_STAT_FILE
	if (l2tp_opt_write_status_files) {
		l2tp_statusfile_init();
	}
#endif

	//l2tp_load_config_file();
	openl2tp_plugin_init();

	//printf("%s success.\n", __func__);
}

/*static */
void l2tp_cleanup(void)
{
#if 0
	pid_t pid;

	pid = getpid();
	if (pid != l2tp_my_pid) {
		L2TP_DEBUG(L2TP_FUNC, "%s: not main pid so returning now", __func__);
		return;
	}
#endif
	l2tp_log(LOG_INFO, "Cleaning up before exiting");

	//usl_signal_notifier_remove(l2tp_signal_handler, NULL);

	/* Cleanup all resources */
#ifdef L2TP_FEATURE_RPC_MANAGEMENT
	l2tp_api_cleanup();
#endif
	//l2tp_event_cleanup();
	l2tp_net_cleanup();
	l2tp_avp_cleanup();
	//l2tp_ppp_cleanup();
	l2tp_session_cleanup();
	l2tp_xprt_cleanup();
	l2tp_tunnel_cleanup();
	l2tp_peer_cleanup();
	//l2tp_plugin_cleanup();

	usl_timer_cleanup();
	usl_fd_cleanup();
	//usl_signal_cleanup();
	//usl_pid_cleanup();

	if (l2tp_rand_fd != 0) {
		close(l2tp_rand_fd);
	}

#ifdef L2TP_FEATURE_LOCAL_STAT_FILE
	if (l2tp_opt_write_status_files) {
		l2tp_statusfile_cleanup();
	}
#endif

	L2TP_DEBUG(L2TP_FUNC, "%s: done", __func__);

#ifdef L2TP_DMALLOC
	dmalloc_log_changed(l2tp_dmalloc_mark, 1, 0, 1);
	// dmalloc_log_unfreed();
	// dmalloc_log_stats();
	// dmalloc_log_heap_map();
#endif

	/* Remove pid file */
	//unlink(l2tp_opt_pid_filename);
}

/*
 * search through a possible list of ',' seperated ip addresses, try
 * each one,  if it works then use that one
 */
#if 1
static struct in_addr get_ip_address(char *name)
{
	struct in_addr retval;
	struct sockaddr_in dest;
	int s;
	char *cp, *np;

	retval.s_addr = 0;
	for (cp = name; cp && *cp; cp = np) {
		if ((np = strchr(cp, ',')) != 0) {
			*np++ = '\0';
		}

		if (inet_aton(cp, &retval) == 0) {
			struct hostent *host = gethostbyname(cp);
			if (host==NULL) {
				if (h_errno == HOST_NOT_FOUND)
					printf("gethostbyname: HOST NOT FOUND");
				else if (h_errno == NO_ADDRESS)
					printf("gethostbyname: NO IP ADDRESS");
				else
					printf("gethostbyname: name server error");
				continue;
			}
			if (host->h_addrtype != AF_INET) {
				printf("Host has non-internet address");
				continue;
			}
			memcpy(&retval.s_addr, host->h_addr, sizeof(retval.s_addr));
		}

		if (np)
			*(np - 1) = ','; /* put string back how we found it */

		return (retval);
	}
	
	retval.s_addr = 0;
	
	return retval;
}
#else
static struct in_addr get_ip_address(char *name)
{
	struct in_addr retval;
	struct sockaddr_in dest;
	int s;
	char *cp, *np;
	char buf[128];
	struct in_addr ipv4addr;
	
	retval.s_addr = 0;
	for (cp = name; cp && *cp; cp = np) {
		if ((np = strchr(cp, ',')) != 0) {
			*np++ = '\0';
		}

		if (inet_aton(cp, &retval) == 0) {
			struct hostent *host = gethostbyname(cp);
			if (host==NULL) {
				if (h_errno == HOST_NOT_FOUND)
					printf("gethostbyname: HOST NOT FOUND");
				else if (h_errno == NO_ADDRESS)
					printf("gethostbyname: NO IP ADDRESS");
				else
					printf("gethostbyname: name server error");
				continue;
			}
			if (host->h_addrtype != AF_INET) {
				printf("Host has non-internet address");
				continue;
			}
			memcpy(&retval.s_addr, host->h_addr, sizeof(retval.s_addr));
		}
		// Mason Yu. If cp is not host name, use gethostbyaddr() to find host
		else {
			inet_pton(AF_INET, cp, &ipv4addr);
			struct hostent *host = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
			if (host==NULL) {
				printf("gethostbyaddr: HOST NOT FOUND\n");
				continue;
			}
			memcpy(&retval.s_addr, host->h_addr, sizeof(retval.s_addr));
		}
		
		if (np)
			*(np - 1) = ','; /* put string back how we found it */
		
		return (retval);
		
		/*
		bzero(&dest, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port   = htons(1701);
		dest.sin_addr   = retval;

		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			printf("Cannot get socket: %s", strerror(errno));
			continue;
		}
		
		sprintf(buf, "This is a test packet!\n");		
		if (sendto(s, buf, sizeof(buf), 0, (struct sockaddr *)&dest, sizeof(dest)) != -1) {
			L2TP_DEBUG("connect %x(%d) success.", dest.sin_addr.s_addr, dest.sin_port);
			close(s);
			return(retval);
		}		
		close(s);
		*/
	}
	
	retval.s_addr = 0;
	
	return retval;
}
#endif

/*QL 20120514: support IPv6*/
#ifdef CONFIG_IPV6_VPN
static struct in6_addr get_ip6_address(char *name)
{
	struct in6_addr retval;
	char *cp, *np;
	char buf[100];

	memset(&retval, 0, sizeof(retval));
	
	if (!strchr(name, ':'))
	{
		printf("name is not IPv6 address.\n");
		return retval;
	}
	
	for (cp = name; cp && *cp; cp = np) {
		if ((np = strchr(cp, ',')) != 0) {
			*np++ = '\0';
		}

		if (inet_pton(AF_INET6, cp, &retval) != 1) {
			printf("%s %d fail(%s)\n", __func__, __LINE__, cp);
			continue;
		}

		if (np)
			*(np - 1) = ','; /* put string back how we found it */

		return (retval);
	}
	
	memset(&retval, 0, sizeof(retval));
	
	return retval;
}
#endif // end of CONFIG_IPV6_VPN

static int get_l2tp_tunnel_profile_msg_data(struct l2tp_api_tunnel_profile_msg_data *msg, struct sppp *sppp)
{
	struct vpn_param_s *vpnr;
	L2TP_DRV_CTRL *p;

	vpnr = (struct vpn_param_s *)sppp->dev;
	if (NULL == vpnr)
		return 0;
	
	p = sppp->pp_lowerp;

	memset(msg, 0, sizeof(struct l2tp_api_tunnel_profile_msg_data));
	
	msg->profile_name = vpnr->tunnel_profile_nam;
	
	msg->flags |= L2TP_API_TUNNEL_PROFILE_FLAG_AUTH_MODE;
	if (vpnr->tunnel_auth) {
		msg->auth_mode = L2TP_API_TUNNEL_AUTH_MODE_CHALLENGE;

		msg->flags |= L2TP_API_TUNNEL_PROFILE_FLAG_SECRET;
		msg->secret.valid = vpnr->tunnel_auth?1:0;
		OPTSTRING(msg->secret) = vpnr->secret;
	}
	else
		msg->auth_mode = L2TP_API_TUNNEL_AUTH_MODE_NONE;

	msg->flags |= L2TP_API_TUNNEL_PROFILE_FLAG_PEER_ADDR;
	if (0 == vpnr->ipType) {
		msg->peer_addr.s_addr.in = p->remote.sin.sin_addr.s_addr;
		msg->peer_addr.flag = 0;
	}
#ifdef CONFIG_IPV6_VPN
	else {
		memcpy((void *)msg->peer_addr.s_addr.in6, (const void *)p->remote.sin6.sin6_addr.s6_addr, 16);
		msg->peer_addr.flag = 1;
	}
#endif

	if ((p->local.sa.sa_family == AF_INET) || (p->local.sa.sa_family == AF_INET6)) {
		msg->flags |= L2TP_API_TUNNEL_PROFILE_FLAG_OUR_ADDR;
		if (0 == vpnr->ipType) {
			msg->our_addr.s_addr.in = p->local.sin.sin_addr.s_addr;
			msg->our_addr.flag = 0;
		}
#ifdef CONFIG_IPV6_VPN
		else {
			memcpy((void *)msg->our_addr.s_addr.in6, (const void *)p->local.sin6.sin6_addr.s6_addr, 16);
			msg->our_addr.flag = 1;
		}
#endif
	}

	msg->flags |= L2TP_API_TUNNEL_PROFILE_FLAG_PEER_UDP_PORT;
	msg->peer_udp_port = 1701;

	msg->flags |= L2TP_API_TUNNEL_PROFILE_FLAG_OUR_UDP_PORT;
	msg->our_udp_port = 1701;

	msg->flags |= L2TP_API_TUNNEL_PROFILE_FLAG_MTU;
	msg->mtu = sppp->lcp.mru;

	msg->flags |= L2TP_API_TUNNEL_PROFILE_FLAG_SESSION_PROFILE_NAME;
	msg->session_profile_name.optstring_u.value = vpnr->session_profile_nam;
	msg->session_profile_name.valid = 1;

	msg->flags |= L2TP_API_TUNNEL_PROFILE_FLAG_HELLO_TIMEOUT;
	msg->hello_timeout = 2;

	msg->flags |= L2TP_API_TUNNEL_PROFILE_FLAG_HOST_NAME;
	msg->host_name.valid = 1;
	OPTSTRING(msg->host_name) = l2tp_host_name;

	return 1;
}

static int get_l2tp_tunnel_msg_data(struct l2tp_api_tunnel_msg_data *msg, struct sppp *sppp)
{
	struct vpn_param_s *vpnr;
	L2TP_DRV_CTRL *p;

	vpnr = (struct vpn_param_s *)sppp->dev;
	if (NULL == vpnr)
		return 0;

	p = sppp->pp_lowerp;

	memset(msg, 0, sizeof(struct l2tp_api_tunnel_msg_data));
	
	//msg->flags |= L2TP_API_TUNNEL_FLAG_CONFIG_ID;
	//msg->config_id = vpnr->index+1;//config_id increments from 1

	msg->flags |= L2TP_API_TUNNEL_FLAG_PEER_ADDR;
	if (0 == vpnr->ipType) {
		msg->peer_addr.s_addr.in = p->remote.sin.sin_addr.s_addr;
		msg->peer_addr.flag = 0;
	}
#ifdef CONFIG_IPV6_VPN
	else {
		memcpy((void *)msg->peer_addr.s_addr.in6, (const void *)p->remote.sin6.sin6_addr.s6_addr, 16);
		msg->peer_addr.flag = 1;
	}
#endif

	if ((p->local.sa.sa_family == AF_INET) || (p->local.sa.sa_family == AF_INET6)) {
		msg->flags |= L2TP_API_TUNNEL_FLAG_OUR_ADDR;
		if (0 == vpnr->ipType) {
			msg->our_addr.s_addr.in = p->local.sin.sin_addr.s_addr;
			msg->our_addr.flag = 0;
		}
#ifdef CONFIG_IPV6_VPN
		else {
			memcpy((void *)msg->our_addr.s_addr.in6, (const void *)p->local.sin6.sin6_addr.s6_addr, 16);
			msg->our_addr.flag = 1;
		}
#endif
	}
	
	msg->flags |= L2TP_API_TUNNEL_FLAG_PEER_UDP_PORT;
	msg->peer_udp_port = 1701;

	msg->flags |= L2TP_API_TUNNEL_FLAG_OUR_UDP_PORT;
	msg->our_udp_port = 1701;

	msg->flags |= L2TP_API_TUNNEL_FLAG_AUTH_MODE;
	if (vpnr->tunnel_auth) {
		msg->auth_mode = L2TP_API_TUNNEL_AUTH_MODE_CHALLENGE;
		
		msg->flags |= L2TP_API_TUNNEL_FLAG_SECRET;
		msg->secret.valid = vpnr->tunnel_auth?1:0;
		OPTSTRING(msg->secret) = vpnr->secret;
	}
	else
		msg->auth_mode = L2TP_API_TUNNEL_AUTH_MODE_NONE;

	//msg->flags |= L2TP_API_TUNNEL_FLAG_TUNNEL_ID;
	//msg->tunnel_id = vpnr->index+1;//tunnel_id=0 indicates undefined.

	msg->flags |= L2TP_API_TUNNEL_FLAG_PROFILE_NAME;
	msg->tunnel_profile_name.valid = 1;
	OPTSTRING(msg->tunnel_profile_name) = vpnr->tunnel_profile_nam;

	msg->flags |= L2TP_API_TUNNEL_FLAG_TUNNEL_NAME;
	OPTSTRING(msg->tunnel_name) = vpnr->tunnel_nam;
	msg->tunnel_name.valid = 1;

	msg->flags |= L2TP_API_TUNNEL_FLAG_MTU;
	msg->mtu = sppp->lcp.mru;

	msg->flags |= L2TP_API_TUNNEL_FLAG_SESSION_PROFILE_NAME;
	OPTSTRING(msg->session_profile_name) = vpnr->session_profile_nam;
	msg->session_profile_name.valid = 1;

	msg->flags |= L2TP_API_TUNNEL_FLAG_HOST_NAME;
	OPTSTRING(msg->host_name) = l2tp_host_name;
	msg->host_name.valid = 1;

	return 1;
}

static int get_l2tp_session_profile_msg_data(struct l2tp_api_session_profile_msg_data *msg, struct sppp *sppp)
{
	struct vpn_param_s *vpnr;
	L2TP_DRV_CTRL *p;

	vpnr = (struct vpn_param_s *)sppp->dev;
	if (NULL == vpnr)
		return 0;
	
	p = sppp->pp_lowerp;

	memset(msg, 0, sizeof(struct l2tp_api_session_profile_msg_data));
	
	msg->profile_name = vpnr->session_profile_nam;

	msg->flags |= L2TP_API_SESSION_PROFILE_FLAG_SESSION_TYPE;
	msg->flags |= L2TP_API_SESSION_FLAG_SESSION_TYPE;
	msg->session_type = L2TP_API_SESSION_TYPE_LAIC;
//	msg->session_type = L2TP_API_SESSION_TYPE_LNOC;

	return 1;
}

static int get_l2tp_session_msg_data(struct l2tp_api_session_msg_data *msg, struct sppp *sppp)
{
	struct vpn_param_s *vpnr;
	L2TP_DRV_CTRL *p;

	vpnr = (struct vpn_param_s *)sppp->dev;
	if (NULL == vpnr)
		return 0;

	p = sppp->pp_lowerp;

	memset(msg, 0, sizeof(struct l2tp_api_session_msg_data));
	
	msg->flags |= L2TP_API_SESSION_FLAG_SESSION_TYPE;
	msg->session_type = L2TP_API_SESSION_TYPE_LAIC;
//	msg->session_type = L2TP_API_SESSION_TYPE_LNOC;

	msg->flags |= L2TP_API_SESSION_FLAG_USER_NAME;
	OPTSTRING(msg->user_name) = sppp->myauth.name;
	msg->user_name.valid = 1;

	msg->flags |= L2TP_API_SESSION_FLAG_USER_PASSWORD;
	OPTSTRING(msg->user_password) = sppp->myauth.secret;
	msg->user_password.valid = 1;

	msg->flags |= L2TP_API_SESSION_FLAG_TUNNEL_NAME;
	OPTSTRING(msg->tunnel_name) = vpnr->tunnel_nam;
	msg->tunnel_name.valid = 1;

	msg->flags |= L2TP_API_SESSION_FLAG_SESSION_NAME;
	OPTSTRING(msg->session_name) = vpnr->session_nam;
	msg->session_name.valid = 1;

#if 0
	msg->flags |= L2TP_API_SESSION_FLAG_MINIMUM_BPS;
	msg->minimum_bps = 0;

	msg->flags |= L2TP_API_SESSION_FLAG_MAXIMUM_BPS;
	msg->maximum_bps = 1000000000;//1GigaBits

	msg->flags |= L2TP_API_SESSION_FLAG_INTERFACE_NAME;
	OPTSTRING(msg->interface_name) = "nas0_0";
	msg->interface_name.valid = 1;
#endif

	msg->flags |= L2TP_API_SESSION_FLAG_PROFILE_NAME;
	OPTSTRING(msg->profile_name) = vpnr->session_profile_nam;
	msg->profile_name.valid = 1;

	msg->tunnel_id = p->tunnel_id;
	
	return 1;
}

static void get_sys_info(void)
{
	int result;
	//int log_flags = LOG_PID;
	struct utsname name;

	/* Get system kernel info, which is used to build our vendor name */
	result = uname(&name);
	if (result < 0) {
		fprintf(stderr, "Failed to get system version info: %m");
		return;
	}
	l2tp_kernel_version = strdup(name.release);
	l2tp_cpu_name = strdup(name.machine);
	if ((l2tp_kernel_version == NULL) || (l2tp_cpu_name == NULL)) {
		fprintf(stderr, "Out of memory\n");
		return;
	}
	l2tp_firmware_revision = (((L2TP_APP_MAJOR_VERSION & 0xff) << 8) |
				  ((L2TP_APP_MINOR_VERSION & 0xff) << 0));
}

static unsigned int l2tp_init_flag=0;

int l2tp_client_init(struct sppp *sppp)
{
	struct l2tp_api_tunnel_profile_msg_data tunnel_profile;
	struct l2tp_api_session_profile_msg_data session_profile;
	struct l2tp_api_tunnel_msg_data tunnel_msg;
	struct l2tp_api_session_msg_data session_msg;
	//struct l2tp_tunnel *tunnel;
	struct vpn_param_s *vpnr;
	L2TP_DRV_CTRL *p;
	struct in_addr inetaddr;
#ifdef CONFIG_IPV6_VPN
	struct in6_addr inet6addr;
#endif
	int flags, result;

	L2TP_DEBUG(L2TP_FUNC, "%s %d Enter.\n", __func__, __LINE__);

	if (0 == l2tp_init_flag) {
		get_sys_info();
		
		l2tp_init();
		l2tp_init_flag = 1;
	}
	
	p = (L2TP_DRV_CTRL *)malloc(sizeof(L2TP_DRV_CTRL));
	if (!p)
		return -1;
	memset (p, 0, sizeof(L2TP_DRV_CTRL));
	p->fd = -1;
	p->sppp = (void *)sppp;

	vpnr = sppp->dev;
	if (NULL == vpnr)
		return -1;

	if (0 == sppp->ipType)
		vpnr->ipType = 0;
#ifdef CONFIG_IPV6_VPN
	else
		vpnr->ipType = 1;
#endif

	strncpy(p->name, vpnr->devname, IFNAMSIZ);
	memcpy(p->server, vpnr->server, MAX_DOMAIN_LENGTH);
	//p->out_ctrl_timeout = 1;		// Mason Yu. Set up_retry
	
	sppp->pp_lowerp = (void *)p;

	if (0 == sppp->ipType) {//ipv4
		inetaddr = get_ip_address(vpnr->server);
		if (0 == inetaddr.s_addr) {
			printf("Server %s is unreachable.\n", vpnr->server);
			return -1;
		}

		vpnr->inetaddr.in = inetaddr;
		
		p->remote.sa.sa_family = AF_INET;
		p->remote.sin.sin_addr.s_addr = inetaddr.s_addr;
		p->remote.sin.sin_port = htons(1701);
		memset(&p->local.sin, 0, sizeof(struct sockaddr_in));
	}
#ifdef CONFIG_IPV6_VPN
	else {//ipv6
		inet6addr = get_ip6_address(vpnr->server);
		if (ipv6_addr_any(&inet6addr)) {
			printf("Server %s is unreachable.\n", vpnr->server);
			return -1;
		}

		vpnr->inetaddr.in6 = inet6addr;
		
		p->remote.sin6.sin6_family = AF_INET6;
		p->remote.sin6.sin6_addr = inet6addr;
		p->remote.sin6.sin6_port = htons(1701);
		memset(&p->local.sin6, 0, sizeof(struct sockaddr_in6));
	}
#endif

	if (0 == sppp->ipType) {
		result = l2tp_net_get_local_address_for_peer(&p->remote.sin, &p->local.sin.sin_addr);
		if (result < 0) {
			printf("l2tp_net_get_local_address_for_peer fail.\n");
			return -1;
		}
	}
#ifdef CONFIG_IPV6_VPN
	else {
		result = l2tp_net6_get_local_address_for_peer(&p->remote.sin6, &p->local.sin6.sin6_addr);
		if (result < 0) {
			printf("l2tp_net6_get_local_address_for_peer fail.\n");
			return -1;
		}
	}
#endif

	if (0 == sppp->ipType) {//ipv4
		L2TP_DEBUG(L2TP_FUNC, "src is %x dst is %x.\n", p->local.sin.sin_addr.s_addr, p->remote.sin.sin_addr.s_addr);
	}
#ifdef CONFIG_IPV6_VPN
	else {//ipv6
		char src[48], dst[48];
		L2TP_DEBUG(L2TP_FUNC, "src is %s dst is %s.\n", 
			inet_ntop(AF_INET6, &p->local.sin6.sin6_addr, src, 48), 
			inet_ntop(AF_INET6, &p->remote.sin6.sin6_addr, dst, 48));
	}
#endif

	snprintf(vpnr->tunnel_nam, MAX_NAME_LEN, "tunnel_%d", vpnr->index);
	snprintf(vpnr->tunnel_profile_nam, MAX_NAME_LEN, "tunnel_profile_%d", vpnr->index);
	snprintf(vpnr->session_nam, MAX_NAME_LEN, "session_%d", vpnr->index);
	snprintf(vpnr->session_profile_nam, MAX_NAME_LEN, "session_profile_%d", vpnr->index);

	if (l2tp_tunnel_get_instance(0, vpnr->tunnel_nam) != NULL) {
		//L2TP_DEBUG(L2TP_FUNC, "waiting for tunnel to be deleted...\n");
		return -1;
	}
	
	memset(&tunnel_profile, 0, sizeof(tunnel_profile));
	if (1 != get_l2tp_tunnel_profile_msg_data(&tunnel_profile, sppp)) {
		printf("get_l2tp_tunnel_profile_msg_data fail.\n");
		return -1;
	}
	
	l2tp_tunnel_profile_create_1_svc(tunnel_profile, &result, NULL);
	if ((result < 0) && (result != -L2TP_ERR_PROFILE_ALREADY_EXISTS)) {
		printf("l2tp_tunnel_profile_create_1_svc fail. errno=%d\n", result);
		return -1;
	}

	if (1 != get_l2tp_tunnel_msg_data(&tunnel_msg, sppp)) {
		printf("get_l2tp_tunnel_msg_data fail.\n");
		return -1;
	}
	
	l2tp_tunnel_create_1_svc(tunnel_msg, &result, NULL);
	if ((result < 0) && (result != -L2TP_ERR_TUNNEL_ALREADY_EXISTS)) {
		printf("l2tp_tunnel_create_1_svc fail. errno=%d\n", result);
		return -1;
	}

	if (1 != get_l2tp_session_profile_msg_data(&session_profile, sppp)) {
		printf("get_l2tp_session_profile_msg_data fail.\n");
		return -1;
	}
	
	l2tp_session_profile_create_1_svc(session_profile, &result, NULL);
	if ((result < 0) && (result != -L2TP_ERR_PROFILE_ALREADY_EXISTS)) {
		printf("l2tp_session_profile_create_1_svc fail. errno=%d\n", result);
		return -1;
	}

	p->in_ctrl_conn_mode = 1;
	p->call_state = L2TP_ST_TUNNEL_INIT;

	//tunnel = l2tp_tunnel_find_by_name(vpnr->tunnel_nam);
	//pppol2tp socket has not been created, so return -1
	
	return -1;
}

extern void l2tp_tunnel_update_state(struct sppp *sp);
extern void l2tp_session_update_state(struct sppp *sp);

static void l2tp_handle_state(struct sppp *sp)
{
	struct vpn_param_s *vpnr = sp?sp->dev:NULL;
	struct l2tp_tunnel *tunnel = NULL;
	L2TP_DRV_CTRL *p = sp?sp->pp_lowerp:NULL;

	if ((sp==NULL) || (sp->over!=SPPP_L2TP) || (vpnr==NULL) || (p==NULL))
		return;

	//check if tunnel is closing.
	l2tp_tunnel_update_state(sp);
	
	switch (p->call_state)
	{
	case L2TP_ST_DEAD:
		p->in_ctrl_conn_mode = 0;
		break;
		
	case L2TP_ST_TUNNEL_INIT:
		L2TP_DEBUG(L2TP_FUNC,"%s in L2TP_ST_TUNNEL_INIT state(%d).\n", __func__, L2TP_ST_TUNNEL_INIT);
		//l2tp_tunnel_update_state(sp);
		break;
		
	case L2TP_ST_TUNNEL_ESTABLISH:
		L2TP_DEBUG(L2TP_FUNC,"%s in L2TP_ST_TUNNEL_ESTABLISH state(%d).\n", __func__, L2TP_ST_TUNNEL_ESTABLISH);
		{//start OCRQ
			struct l2tp_api_session_msg_data session;
			int result;
			
			get_l2tp_session_msg_data(&session, sp);
			l2tp_session_create_1_svc(session, &result, NULL);
			if ((result < 0) && (result != -L2TP_ERR_SESSION_ALREADY_EXISTS)) {
				L2TP_DEBUG(L2TP_FUNC,"create session fail. errno=%d\n", result);
			}
			else
				p->call_state = L2TP_ST_SESSION_INIT;
		}
		break;
		
	case L2TP_ST_SESSION_INIT:
		L2TP_DEBUG(L2TP_FUNC,"%s in L2TP_ST_SESSION_INIT state(%d).\n", __func__, L2TP_ST_SESSION_INIT);
		l2tp_session_update_state(sp);
		break;
		
	case L2TP_ST_SESSION_ESTABLISH:
		L2TP_DEBUG(L2TP_FUNC,"%s in L2TP_ST_SESSION_ESTABLISH state(%d).\n", __func__, L2TP_ST_SESSION_ESTABLISH);
		tunnel = l2tp_tunnel_get_instance(p->tunnel_id, NULL);
		if (NULL == tunnel) {
			L2TP_DEBUG(L2TP_FUNC,"%s %d fatal error, tunnel is NULL.\n", __func__, __LINE__);
		}
		else
		{
			struct l2tp_session *session = NULL;
			
			session = l2tp_session_get_instance(tunnel, 0, vpnr->session_nam);
			if (NULL == session) {
				L2TP_DEBUG(L2TP_FUNC,"%s %d session %s has not been created.\n", __func__, __LINE__, vpnr->session_nam);
			}
			else {
				sp->fd = p->fd = l2tp_session_get_fd(session);
				if (sp->fd != -1) {
					L2TP_DEBUG(L2TP_FUNC,"start ppp phase(tunnel_id=%d).\n", p->tunnel_id);
					p->in_ctrl_conn_mode = 0;
					
					start_ppp_real(sp);
				}
				else
					L2TP_DEBUG(L2TP_FUNC,"l2tp socket hasn't been created.\n");
			}
		}
		break;
	}
}

int l2tp_up(L2TP_DRV_CTRL *p)
{
	struct sppp *sp;
	struct vpn_param_s *vpnr;
	struct l2tp_api_tunnel_profile_msg_data tunnel_profile;
	struct l2tp_api_session_profile_msg_data session_profile;
	struct l2tp_api_tunnel_msg_data tunnel_msg;
	struct l2tp_api_session_msg_data session_msg;
	struct in_addr inetaddr;
#ifdef CONFIG_IPV6_VPN
	struct in6_addr inet6addr;
	unsigned char sbuf[48], dbuf[48];
#endif
	int result;

	L2TP_DEBUG(L2TP_FUNC, "l2tp_up enter.\n");
	
	sp = (struct sppp*)p->sppp;
	vpnr = sp->dev;

	inetaddr = p->remote.sin.sin_addr;
	if (0 == vpnr->ipType) {
		inetaddr = get_ip_address(p->server);
		if (0 == inetaddr.s_addr) {
			printf("Server %s is unreachable.\n", p->server);
			return -1;
		}

		vpnr->inetaddr.in = inetaddr;
		
		p->remote.sin.sin_family = AF_INET;
		p->remote.sin.sin_addr.s_addr = inetaddr.s_addr;
		p->remote.sin.sin_port = htons(1701);
	}
#ifdef CONFIG_IPV6_VPN
	else {
		inet6addr = get_ip6_address(vpnr->server);
		if (ipv6_addr_any(&inet6addr)) {
			printf("Server %s is unreachable.\n", p->server);
			return -1;
		}
		
		vpnr->inetaddr.in6 = inet6addr;
		
		p->remote.sin6.sin6_family = AF_INET6;
		p->remote.sin6.sin6_addr = inet6addr;
		p->remote.sin6.sin6_port = htons(1701);
	}
#endif // end of CONFIG_IPV6_VPN

	if (0 == vpnr->ipType) {//IPv4
		if (0 == p->local.sin.sin_addr.s_addr) {
	
			result = l2tp_net_get_local_address_for_peer(&p->remote.sin, &p->local.sin.sin_addr);
			if (result < 0) {
				L2TP_DEBUG(L2TP_FUNC,"l2tp_net_get_local_address_for_peer fail.\n");
				return -1;
			}
			
			L2TP_DEBUG(L2TP_FUNC, "src is %x dst is %x.\n", p->local.sin.sin_addr.s_addr, p->remote.sin.sin_addr.s_addr);
		}
	}
#ifdef CONFIG_IPV6_VPN
	else {
		if (ipv6_addr_any(&p->local.sin6.sin6_addr)) {
			result = l2tp_net6_get_local_address_for_peer(&p->remote.sin6, &p->local.sin6.sin6_addr);
			if (result < 0) {
				L2TP_DEBUG(L2TP_FUNC,"l2tp_net6_get_local_address_for_peer fail.\n");
				return -1;
			}
			
			L2TP_DEBUG(L2TP_FUNC, "src is %s dst is %s.\n", 
				inet_ntop(AF_INET6, &p->local.sin6.sin6_addr, sbuf, 48), 
				inet_ntop(AF_INET6, &p->remote.sin6.sin6_addr, dbuf, 48));
		}
	}
#endif

	snprintf(vpnr->tunnel_nam, MAX_NAME_LEN, "tunnel_%d", vpnr->index);
	snprintf(vpnr->tunnel_profile_nam, MAX_NAME_LEN, "tunnel_profile_%d", vpnr->index);
	snprintf(vpnr->session_nam, MAX_NAME_LEN, "session_%d", vpnr->index);
	snprintf(vpnr->session_profile_nam, MAX_NAME_LEN, "session_profile_%d", vpnr->index);

	if (l2tp_tunnel_get_instance(0, vpnr->tunnel_nam) != NULL) {
		//L2TP_DEBUG(L2TP_FUNC, "waiting for tunnel to be deleted...\n");
		return -1;
	}

	L2TP_DEBUG(L2TP_FUNC, "to create tunnel %s\n", vpnr->tunnel_nam);
	
	memset(&tunnel_profile, 0, sizeof(tunnel_profile));
	if (1 != get_l2tp_tunnel_profile_msg_data(&tunnel_profile, sp)) {
		L2TP_DEBUG(L2TP_FUNC,"get_l2tp_tunnel_profile_msg_data fail.\n");
		return -1;
	}
	
	l2tp_tunnel_profile_create_1_svc(tunnel_profile, &result, NULL);
	if ((result < 0) && (result != -L2TP_ERR_PROFILE_ALREADY_EXISTS)) {
		L2TP_DEBUG(L2TP_FUNC,"l2tp_tunnel_profile_create_1_svc fail. errno=%d\n", result);
		return -1;
	}

	if (1 != get_l2tp_tunnel_msg_data(&tunnel_msg, sp)) {
		L2TP_DEBUG(L2TP_FUNC,"get_l2tp_tunnel_msg_data fail.\n");
		return -1;
	}
	
	l2tp_tunnel_create_1_svc(tunnel_msg, &result, NULL);
	if ((result < 0) && (result != -L2TP_ERR_TUNNEL_ALREADY_EXISTS)) {
		L2TP_DEBUG(L2TP_FUNC,"l2tp_tunnel_create_1_svc fail. errno=%d\n", result);
		return -1;
	}

	if (1 != get_l2tp_session_profile_msg_data(&session_profile, sp)) {
		L2TP_DEBUG(L2TP_FUNC,"get_l2tp_session_profile_msg_data fail.\n");
		return -1;
	}
	
	l2tp_session_profile_create_1_svc(session_profile, &result, NULL);
	if ((result < 0) && (result != -L2TP_ERR_PROFILE_ALREADY_EXISTS)) {
		L2TP_DEBUG(L2TP_FUNC,"l2tp_session_profile_create_1_svc fail. errno=%d\n", result);
		return -1;
	}

	p->in_ctrl_conn_mode = 1;
	p->call_state = L2TP_ST_TUNNEL_INIT;

	return 1;
}

int l2tp_close(struct sppp * sp)
{
	L2TP_DRV_CTRL *p = sp->pp_lowerp;
	struct vpn_param_s *vpnr = sp->dev;
	optstring tunnel_name;
	optstring session_name;
	optstring reason={.valid = 0,};
	int result;

	printf("%s %d enter\n", __func__, __LINE__);
	if ((p != NULL) && (vpnr != NULL)) {
		if (p->fd >= 0)
			close(p->fd);
		p->fd = -1;

		tunnel_name.valid = 1;
		OPTSTRING(tunnel_name) = vpnr->tunnel_nam;
		session_name.valid = 1;
		OPTSTRING(session_name) = vpnr->session_nam;
		
		l2tp_session_delete_1_svc(p->tunnel_id, tunnel_name, p->session_id, session_name, reason, &result, NULL);
		if (result < 0)
			printf("delete session %d fail. errno=%d\n", p->session_id, result);

		l2tp_tunnel_delete_1_svc(p->tunnel_id, tunnel_name, reason, &result, NULL);
		if (result < 0)
			printf("delete tunnel %d fail. errno=%d\n", p->tunnel_id, result);

		if (p->previous_local_addr)
		{
			char cmd_str[256];
			sprintf(cmd_str, "iptables -t nat -D POSTROUTING -o ppp%d -j SNAT --to-source %s", sp->if_unit, 
				inet_ntoa(*(struct in_addr *)&p->previous_local_addr));
			system(cmd_str);
		}
		
		p->previous_local_addr = sp->ipcp.myipaddr;

		free(p);
		sp->pp_lowerp = NULL;
	}
	printf("%s %d exit\n", __func__, __LINE__);
	
	return (0);
}

extern struct sppp *spppq;
// Mason Yu. Set up_retry
/*
void l2tp_out_ctrl(L2TP_DRV_CTRL *p)
{	
	// 0: do up_retry, 1: not do up_retry
	p->up_retry=0;
}
*/

void process_l2tp()
{
	struct sppp *sp;
	L2TP_DRV_CTRL *p;

	sp = spppq;
	while(sp) {
		if (sp->over != SPPP_L2TP) {
			sp = sp->pp_next;
			continue;
		}
		if (!(p = sp->pp_lowerp)) {
			sp = sp->pp_next;
			continue;
		}

		if (L2TP_ST_DEAD != p->call_state)
			usl_fd_poll();
		
		if (p->in_ctrl_conn_mode) {
			l2tp_handle_state(sp);
		}
		else {
			/*
			if (L2TP_ST_DEAD == p->call_state) {
				// Mason Yu. Set up_retry
				if ( !p->up_retry) {
					if (l2tp_up(p) == -1) {					
						p->up_retry = 1;
						TIMEOUT(l2tp_out_ctrl, p, p->out_ctrl_timeout, p->out_ctrl_ch);
						
						p->out_ctrl_timeout *= 2;
						if (p->out_ctrl_timeout >= 16)
							p->out_ctrl_timeout = 1;						
					}					
				}
			}			
			*/
			
			if (L2TP_ST_DEAD == p->call_state) {
				l2tp_up(p);
			}			
		}

		sp = sp->pp_next;
	}
   	return;

}

#ifdef CONFIG_USER_L2TPD_LNS
int l2tp_session_created_pppd(uint16_t tunnel_id, uint16_t session_id){
	struct sppp *sp;
	L2TPD_DRV_CTRL *p;
	
	if ((sp= add_pppd(SPPP_L2TPD)) != NULL){
		p = (L2TPD_DRV_CTRL *)malloc(sizeof(L2TPD_DRV_CTRL));
		if (!p)
			return -1;
		memset (p, 0, sizeof(L2TPD_DRV_CTRL));

		sp->pp_lowerp = (void *)p;
		
		p->sppp = (void *)sp;
		p->in_ctrl_conn_mode = 1;
		p->account_idx = -1;
		p->call_state = L2TP_ST_SESSION_INIT;
		p->tunnel_id = tunnel_id;
		p->session_id = session_id;
	}
	return 0;
}

int l2tp_server_init(int first)
{
	if (0 == l2tp_init_flag) {
		get_sys_info();
		l2tp_init();
		l2tp_init_flag = 1;
	}
	if(first){
		createServerSocket();
		l2tp_session_created_pppd_hook = l2tp_session_created_pppd;
	}
	return 0;
}

int l2tp_server_close( int first)
{	
	closeServerSocket();
}

static void l2tpd_handle_state(struct sppp *sp)
{
	struct l2tp_tunnel *tunnel = NULL;
	L2TPD_DRV_CTRL *p = sp?sp->pp_lowerp:NULL;

	if ((sp==NULL) || (sp->over!=SPPP_L2TPD) || (p==NULL))
		return;

	l2tpd_session_update_state(sp);

	switch (p->call_state)
	{
	case L2TP_ST_DEAD:
		del_pppd(sp->unit);
		break;
		
	case L2TP_ST_SESSION_ESTABLISH:
		tunnel = l2tp_tunnel_find_by_id(p->tunnel_id);
		if (NULL == tunnel) {
			L2TP_DEBUG(L2TP_FUNC,"%s %d tunnel %d has  been deleted.\n", __func__, __LINE__, p->tunnel_id);
		}
		else
		{
			struct l2tp_session *session = NULL;
			session = l2tp_session_get_instance(tunnel, p->session_id, "");
			if (NULL == session) {
				L2TP_DEBUG(L2TP_FUNC,"%s %d session %d has  been deleted.\n", __func__, __LINE__, p->session_id);
			}
			else {
				if(p->in_ctrl_conn_mode == 1){
					sp->fd = p->fd = l2tp_session_get_fd(session);
					if (sp->fd != -1) {
						p->in_ctrl_conn_mode = 0;
						start_pppd_real(sp);
					}
				}				
			}
		}
		break;
	}
}

void process_l2tpd()
{
	struct sppp *sq, *sp;

	if(getServerSocketStatus())
		usl_fd_poll();

	sq = spppq;
	while(sp = sq) {
		sq = sp->pp_next;
		if (sp->over != SPPP_L2TPD) {
			continue;
		}

		//todo:
		l2tpd_handle_state(sp);

	}
	return;
}

#endif



