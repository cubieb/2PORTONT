/*
 * msgq.h -- System V Message Queue Framework Header
 * --- By Kaohj
 */

#ifndef _h_MSGQ
#define _h_MSGQ 1

#include <sys/types.h>

#define MAX_SEND_SIZE	80
#define MQ_CREATE	0
#define MQ_GET		1
#define MSG_SUCC	0
#define MSG_FAIL	1

struct mymsgbuf {
        long mtype;			// Message type
        long request;			// Request ID/Status code
        char mtext[MAX_SEND_SIZE];	// Client/server information
};

extern int	open_queue( key_t keyval, int flag );
extern void	send_message(int qid, long type, long req, char *text);
extern void	read_message(int qid, struct mymsgbuf *qbuf, long type);
extern int	peek_message(int qid, long type);
extern void	remove_queue(int qid);

#endif /* _h_MSGQ */
