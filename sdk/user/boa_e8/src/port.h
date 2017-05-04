#ifndef PORT_H_
#define PORT_H_

#define uint32 unsigned int
//typedef request* request *;
#define B_ARGS_DEC		char *file, int line

extern int getInfo(int eid, request* wp, int argc, char  **argv);
extern int portFwList(int eid, request* wp, int argc, char **argv);
extern int ipPortFilterList(int eid, request* wp, int argc, char **argv);
extern int macFilterList(int eid, request* wp, int argc, char **argv);
extern int atmVcList(int eid, request* wp, int argc, char **argv);
extern int atmVcList2(int eid, request* wp, int argc, char **argv);
extern int wanConfList(int eid, request* wp, int argc, char **argv);

/* Routines exported in fmtcpip.c */
extern void formTcpipLanSetup(request * wp, char *path, char *query);

/* Routines exported in fmfwall.c */
extern void formPortFw(request * wp, char *path, char *query);
extern void formFilter(request * wp, char *path, char *query);
extern void formDMZ(request * wp, char *path, char *query);
extern int portFwList(int eid, request * wp, int argc, char **argv);
extern int ipPortFilterList(int eid, request * wp, int argc, char **argv);
extern int macFilterList(int eid, request * wp, int argc, char **argv);

/* Routines exported in fmmgmt.c */
extern void formPasswordSetup(request * wp, char *path, char *query);
extern void formUpload(request * wp, char * path, char * query);
extern void formSaveConfig(request * wp, char *path, char *query);
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
extern void formSnmpConfig(request * wp, char *path, char *query);
#endif
extern void formAdslDrv(request * wp, char *path, char *query);
extern void formStats(request * wp, char *path, char *query);
extern void formRconfig(request * wp, char *path, char *query);
extern int adslDrvSnrTblGraph(int eid, request * wp, int argc, char **argv);
extern int adslDrvSnrTblList(int eid, request * wp, int argc, char **argv);
extern int adslDrvBitloadTblGraph(int eid, request * wp, int argc, char **argv);
extern int adslDrvBitloadTblList(int eid, request * wp, int argc, char **argv);
extern int pktStatsList(int eid, request * wp, int argc, char **argv);

#endif
