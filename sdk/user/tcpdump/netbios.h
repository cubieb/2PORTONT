/*
 * NETBIOS protocol formats
 *
 * @(#) $Header: /usr/local/dslrepos/uClinux-dist/user/tcpdump/netbios.h,v 1.1.1.1 2003/08/18 05:40:16 kaohj Exp $
 */

struct p8022Hdr {
    u_char	dsap;
    u_char	ssap;
    u_char	flags;
};

#define	p8022Size	3		/* min 802.2 header size */

#define UI		0x03		/* 802.2 flags */

