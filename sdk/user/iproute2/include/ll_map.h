#ifndef __LL_MAP_H__
#define __LL_MAP_H__ 1

#ifdef _LINUX_2_6_

#define IFLA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#define IFLA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ifinfomsg))

#define IFA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#define IFA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ifaddrmsg))

#define NDA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#define NDA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ndmsg))

#endif 


extern int ll_remember_index(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg);
extern int ll_init_map(struct rtnl_handle *rth);
extern int ll_name_to_index(char *name);
extern const char *ll_index_to_name(int idx);
extern const char *ll_idx_n2a(int idx, char *buf);
extern int ll_index_to_type(int idx);
extern unsigned ll_index_to_flags(int idx);

#endif /* __LL_MAP_H__ */
