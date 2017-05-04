#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// Global vars
char g_extInterfaceName[10]; // The name of the external interface, picked up from the
                             // command line
char g_intInterfaceName[10]; // The name of the internal interface, picked from command line

// All vars below are read from /etc/upnpd.conf in main.c
int g_debug;  // 1 - print debug messages to syslog
              // 0 - no debug messages
char g_iptables[50];  // The full name and path of the iptables executable, used in pmlist.c
char g_upstreamBitrate[10];  // The upstream bitrate reported by the daemon
char g_downstreamBitrate[10]; // The downstream bitrate reported by the daemon
char g_forwardChainName[20];  // The name of the iptables chain to put FORWARD rules in
char g_preroutingChainName[20]; // The name of the chain to put PREROUTING rules in
int g_forwardRules;            // 1 - forward rules are inserted
                               // 0 - no forward rules inserted
#endif // _GLOBALS_H_
