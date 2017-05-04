/*
    rcp.c - remote file copy program
    Copyright (C) 2003  Guus Sliepen <guus@sliepen.eu.org>,
                        Wessel Dankers <wsl@fruit.eu.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as published
	by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

char *argv0;

void usage(void) {
	fprintf(stderr, "Usage: %s [-p] [-r] source... destination\n", argv0);
}

/* Make sure everything gets written */

ssize_t safewrite(int fd, const void *buf, size_t count) {
	int written = 0, result;
	
	while(count) {
		result = write(fd, buf, count);
		if(result == -1) {
			if(errno == EINTR)
				continue;
			else
				return result;
		}
		written += result;
		buf += result;
		count -= result;
	}
	
	return written;
}

int safewritev(int fd, struct iovec *v, size_t count) {
	size_t r, e;

	while(count > 0) {
		r = writev(fd, v, count);
		if(r == -1) {
			if(errno == EINTR)
				continue;
			else
				return r;
		}

		for(;;) {
			e = v->iov_len;
			if(r < e)
				break;
			r -= e;
			v++;
			if(!--count)
				return 0;
		}

		v->iov_len = e - r;
		v->iov_base += r;
	}
	return 0;
}

ssize_t saferead(int fd, void *buf, size_t count) {
	int written = 0, result;
	
	while(count) {
		result = read(fd, buf, count);
		if(result == -1) {
			if(errno == EINTR)
				continue;
			else
				return result;
		}
		written += result;
		buf += result;
		count -= result;
	}
	
	return written;
}

/* Rules of a ring:
 *   off < len
 *  fill <= len
 */

typedef struct ring_t {
	char *buf;
	size_t len;
	size_t fill;
	off_t off;
} ring_t;

/* Read data from fd to ring. Block if no data is available yet. */

ssize_t ringread(int fd, ring_t *ring) {
	ssize_t r, c, o;
	fd_set rfds;
	struct iovec io[2];

	c = ring->len - ring->fill;
	r = ring->off + ring->fill;
	o = r - ring->len;
	io[0].iov_base = ring->buf + r;
	io[0].iov_len = r = ring->len - r;
	io[1].iov_base = ring->buf;
	io[1].iov_len = ring->fill - r;

	for(;;) {
		if(ring->off && ring->off + ring->fill < ring->len)
			r = readv(fd, io, c);
		else if(o >= 0)
			r = read(fd, ring->buf + o, c);
		else
			r = read(fd, ring->buf + ring->off, c);

		if(r >= 0)
			break;
		if(errno == EINTR)
			continue;
		if(errno != EAGAIN)
			return r;

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		select(fd + 1, &rfds, NULL, NULL, NULL);
		/* any errors will be caught in the next read() */
	}

	ring->fill += r;
	return r;
}

/* Write max count bytes from ring to fd */

ssize_t ringwrite(int fd, ring_t *ring, size_t count) {
	struct iovec io[2];
	size_t part;

	if(ring->off + ring->fill < ring->len)
		return safewrite(fd, ring->buf + ring->off, ring->fill);

	io[0].iov_base = ring->buf + ring->off;
	io[0].iov_len = part = ring->len - ring->off;
	io[1].iov_base = ring->buf;
	io[1].iov_len = ring->fill - part;

	return safewritev(fd, io, 2);
}

bool ringgetchar(char *c, ring_t *ring) {
	while(!ring->fill) {
		if(ringread(fd, ring) < 0)
			return false;
	}

	*c = ring->buf[ring->off++];
	
	ring->off %= ring->len;
	ring->fill--;

	return true;
}

bool ringgetint(int *i, ring_t *ring) {
	char c;
	int temp = 0, len = 0;

	while(ringgetchar(&c, ring)) {
		if(c == ' ' || c == '\n') {
			if(!len)
				return false;
			*i = temp;
			return true;
		}
		if(c < '0' || c > '9')
			return false;
		temp *= 10;
		temp += c - '0';
		len++;
	}

	return false;
}

bool ringgetmode(int *mode, ring_t *ring) {
	char c;
	int temp = 0, len = 0;

	while(ringgetchar(&c, ring)) {
		if(c == ' ' || c == '\n') {
			if(!len || len > 4)
				return false;
			*i = temp;
			return true;
		}
		if(c < '0' || c > '7')
			return false;
		temp <<= 3;
		temp |= c - '0';
	}

	return false;
}
	

char *ringgets(char *buf, size_t count, ring_t *ring) {
	char *begin, *p, *end = NULL;
	size_t toscan;
	
	p = begin = ring->buf + ring->off;
	toscan = ring->fill;

	while(!end) {
		while(toscan--) {
			if(!*p++) {
		
	
	while(!(end = ringchr(ring, '\0', 0))) {
		if(ringread(fd, ring) < 0)
			return EOF;
	}

	do {
		if(!ring->fill)
			if(ringread(fd, ring) <= 0)
				return NULL;
	} while(!(end = ringchr(ring, '\0', 0)))

	ring->off
	return begin;
}
	
char *ringchr(ring_t *ring, char c, size_t extra) {
	off_t off = ring->off;
	size_t fill = ring->fill;

	fill -= extra;
	if(fill <= 0)
		return NULL;

	off += extra;
	if(off > ring->len)
		off -= ring->len;

	while(fill) {
		if(ring->buf[off] == c)
			return ring->buf + off;
		fill--;
		off++;
		if(off > ring->len)
			off = 0;
	}
	return NULL;
}

ssize_t ringdist(ring_t *ring, char *s) {
	ssize_t r;
	r = s - (ring->buf + ring->off);
	if(r < 0)
		r += ring->len;
	return r;
}

#define MAXMMAP (1<<22)

ssize_t mmapcopy(int fd, ssize_t written, ssize_t size) {
	ssize_t length, chunk;
	off_t start, skip;
	void *m;
	static int pagesize = 0;

	if(!pagesize)
		pagesize = getpagesize();

	while(written < size) {
		start = written / pagesize * pagesize;
		length = size - start;
		if(length > MAXMMAP)
			length = MAXMMAP;
		skip = written - start;
		chunk = length - skip;
		m = mmap(NULL, length, PROT_WRITE, MAP_SHARED, fd, start);
		if(m == MAP_FAILED)
			return -1;
		saferead(fd, m + skip, chunk);
		if(munmap(m, length) == -1)
			return -1;
		written += chunk;
	}

	return written;
}

#if 0
{
	char *user = NULL;
	char *luser = NULL;
	char *host = NULL;
	char *port = "shell";
	char lport[5];
	
	struct passwd *pw;
	
	struct addrinfo hint, *ai, *aip, *lai;
	struct sockaddr raddr;
	int raddrlen;
	int err, sock = -1, lsock = -1, esock, i;
	
	char hostaddr[NI_MAXHOST];
	char portnr[NI_MAXSERV];

	char buf[4096];
	int len;
	
	struct pollfd pfd[3];

	/* Lookup local username */
	
	if (!(pw = getpwuid(getuid()))) {
		fprintf(stderr, "%s: Could not lookup username: %s\n", argv0, strerror(errno));
		return 1;
	}
	user = luser = pw->pw_name;
	
}

ssize_t safesend(int dst, int src, ssize_t len) {
	off_t offset = 0;
	char *mbuf = NULL;
	ssize_t x;
	
	while(len) {
		x = sendfile(dst, src, &offset, len);
		if(x <= 0) {
			if(offset)
				return -1;
			else
				goto mmap;
		}
		len -= x;
	}

	return offset;

mmap:
	if(ftruncate(src, len))
		goto mmap2;
		
	mbuf = mmap(NULL, len, PROT_READ, MAP_PRIVATE, 0, src, 0);

	if(!mbuf)
		goto mmap2;
	
	if(safewrite(dst, mbuf, len) == -1)
		return -1;
	
	munmap(mbuf, len);
	
	return len;

mmap2:
	if(ftruncate(dst, len))
		goto oldway;
		
	mbuf = mmap(NULL, len, PROT_READ, MAP_PRIVATE, 0, dst, 0);

	if(!mbuf)
		goto oldway;
	
	if(saferead(src, mbuf, len) == -1)
		return -1;
	
	munmap(mbuf, len);
	
	return len;
		
oldway:
	while(len) {
		x = read(src, buf, sizeof(buf));
		if(x <= 0)
			return -1;
		if(safewrite(dst, buf, x) == -1)
			return -1;
		count -= x;
	}
	
	return len;
}

ssize_t send_file(int out, int file, char *name, struct stat stat, int preserve) {
	size_t size = stat.st_size;
	size_t len, offset = 0;
	
	if(preserve) {
		snprintf(buf, sizeof(buf), "T%li 0 %li 0\n", stat.st_mtime, stat.st_atime);
		if(safewrite(out, buf, strlen(buf)) == -1)
			return -1;
	}
	
	snprintf(buf, sizeof(buf), "C%04o %li %s\n", stat.st_mode&07777, size, safebasename(name));
	if(safewrite(out, buf, strlen(buf)) == -1)
		return -1;
	
	if(recvresponse())
		return -1;
	
	if(safesend(out, file, size) == -1)
		return -1;
	
	return recvresponse();
}

int send_dir(int out, char *name, struct stat stat, int preserve) {
	DIR *dir;
	struct dirent *ent;
	char buf[1024];
		
	if(preserve) {
		snprintf(buf, sizeof(buf), "T%li 0 %li 0\n", stat.st_mtime, stat.at_mtime);
		if(safewrite(out, buf, strlen(buf)) == -1)
			return -1;
	}
	
	snprintf(buf, sizeof(buf), "D%04o %li %s\n", stat.st_mode&07777, 0, safebasename(name));
	if(safewrite(out, buf, strlen(buf)) == -1)
		return -1;
	
	dir = opendir(name);
	
	if(!dir)
		return -1;
	
	while((ent = readdir(dir))) {
		if(!ent>d_ino)
			continue;
		if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;
		snprintf(buf, sizeof(buf), "%s/%s", name, ent->d_name);
		from(1, &buf, 1, preserve);
	}
	
	closedir(dir);

	snprintf(buf, sizeof(buf), "E\n");
	if(safewrite(out, buf, strlen(buf)) == -1)
		return -1;
	
	return recvresponse();
}
#endif

int from(int fd, int argc, char **argv, int recursive, int preserve) {
	int i;
	struct stat stat;
	int file;
	
	for(i = 0; i < argc; i++) {
		file = open(argv[i], O_RDONLY);
		
		if(file == -1) {
			senderror("%s: %s: %s\n", argv0, argv[i], strerror(errno));
			continue;
		}
		
		if(fstat(file, &stat)) {
			close(file);
			senderror("%s: %s: %s\n", argv0, argv[i], strerror(errno));
			continue;
		}
		
		switch(stat.st_modes & S_IFMT) {
			case S_IFREG:
				send_file(fd, file, stat, recurse, preserve);
				break;
			case SI_IFDIR:
				if(!recursive)
					send_dir(fd, argv[i], stat, preserve);
			default:
				senderror("%s: %s: not a regular file\n", argv0, argv[i]);
				continue;
		}
		
		close(file);
	}
	
	return recvresponse();
}

int to(char *dname, int preserve, int dir) {
	int i;
	struct stat stat;
	int file;
	int mode, size;
	struct timeval tvp[2];
	char name[1024];
	
	for(;;) {
		if(preserve) {
			if(ringg(0, buf, sizeof(buf), '\n') <= 0)
				return -1;
			if(sscanf(buf, "T%li 0 %li 0", &time.modtime, &time.actime) != 2)
				return -1;
		}
		
		if(ringgets(buf, sizeof(buf), ring) <= 0)
			return -1;
		
		switch(*buf) {
			case 'T':
				if(sscanf(buf, "T%li %li %li %li",  
			case 'E':
				if(!dir || buf[1])
					return -1;
				safewrite(0, "", 1);
				return 0;
			case 'D':
				if(sscanf(buf, "D%04o %li %1024s", &type, &mode, &size, name) != 4)
					return -1;

				if(!name)
					return -1;
				
				if(mkdir(name, mode) || chdir(name)) {
					sendresponse(strerror(errno));
					continue;
				}

				safewrite(0, "", 1);

				from(NULL, preserve, 1);
				
				if(chdir(".."))
					return -1;
				
				if(preserve && utime(name, &time))
					return -1;
				
				free(fname);
				free(name);
				
				continue;
			case 'C':
				if(sscanf(buf, "C%04o %li %1024s", &type, &mode, &size, name) != 4)
					return -1;

				if(!name)
					return -1;
				
				file = open(name, O_WRONLY | O_CREAT, mode);

				if(!file) {
					sendresponse(strerror(errno));
					continue;
				}

				if(safewrite(0, "", 1) == -1)
					return -1;

				if(safesend(file, 0, size) == -1)
					return -1;

				if(preserve && utime(name, &time))
					return -1;
				
				close(file);
				
				if(recvresponse())
					return -1;

				continue;
			default:
				return -1;
		}
	}
}

int to(char *dname, int preserve, int dir) {
	char buf[65536];
	char path[PATH_MAX];
	ssize_t size, r, offs, part, slash[128];
	int flags, mode, i, level, fd;
	char cmd, c, *s;
	struct stat st;
	struct ring_t ring;

	ring.buf = buf;
	ring.len = sizeof buf;
	ring.off = 0;
	ring.fill = 0;

	slash[level = 0] = strlen(dname);
	if(slash[level] + 1 >= sizeof path)
		return errno = EINVAL, -1;
	memcpy(path, dname, slash[level] + 1);

	flags = fcntl(STDIN_FILENO, F_GETFL);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

	for(;;) {	
		if(!ring.fill) {
			ring.off = 0;
			r = ringread(&ring, STDIN_FILENO);
			if(r <= 0)
				return -1;
		}
		switch(cmd = ring.buf[ring.off]) {
			case 'E':
				if(ring.len < 2) {
					r = ringread(&ring, STDIN_FILENO);
					if(!r)
						errno = ENODATA;
					if(r <= 0)
						return -1;
				}
				if(ring.buf[ring.off])
					return errno = EPROTO, -1;
				safewrite(STDOUT_FILENO, "", 1);
				if(!level)
					return 0;
				path[slash[--level]] = '\0';
			break;
			case 'C':
			case 'D':
				offs = 0;
				for(;;) {
					s = ringchr(&ring, '\0', offs);
					if(s)
						break;
					offs = ring.fill;
					if(offs == ring.len)
						return errno = ENOBUFS, -1;
					r = ringread(&ring, STDIN_FILENO);
					if(r <= 0)
						return -1;
				}
				r = ringdist(&ring, s) + 1;

				if(r < 9)
					return errno = EPROTO, -1;

				mode = 0;
				for(i = 0; i < 4; i++) {
					if(++ring.off > ring.len)
						ring.off = 0;
					c = ring.buf[ring.off];
					if(c < '0' || c > '7')
						return errno = EPROTO, -1;
					mode = (mode << 3) | (c - '0');
				}

				if(++ring.off > ring.len)
					ring.off = 0;
				if(ring.buf[ring.off] != ' ')
					return errno = EPROTO, -1;
				if(++ring.off > ring.len)
					ring.off = 0;
				ring.fill -= 6;

				size = 0;
				offs = 0;
				for(;;) {
					if(++offs > 19)
						return errno = EOVERFLOW, -1;
					c = ring.buf[ring.off];
					if(c == ' ')
						break;
					if(c < '0' || c > '9')
						return errno = EPROTO, -1;
					size = size * 10 + c - '0';
					if(++ring.off > ring.len)
						ring.off = 0;
				}
				if(offs == 1) /* just a space */
					return errno = EPROTO, -1;
				if(++ring.off > ring.len)
					ring.off = 0;
				ring.fill -= offs;

				r = ringdist(&ring, s) + 1;
				slash[level + 1] = slash[level] + r;
				if(slash[level + 1] + 1 >= sizeof path)
					return errno = ENAMETOOLONG, -1;
				path[slash[level]] = '/';

				offs = ring.off + r - ring.len;
				s = path + slash[level] + 1;
				if(offs > 0) {
					part = ring.len - ring.off;
					memcpy(s, ring.buf + ring.off, part);
					memcpy(s + part, ring.buf, offs);
				} else {
					memcpy(s, ring.buf + ring.off, r);
				}
				ring.off += r;
				ring.fill -= r;

				if(cmd == 'D') {
					if(stat(path, &st)) {
						if(errno == ENOENT && mkdir(path, mode))
							return -1;
						else if((st.st_mode & 07777) != mode)
							chmod(path, mode);
					} else {
						if(!S_ISDIR(st.st_mode))
							return errno = ENOTDIR, -1;
					}
					level++;
				} else {
					/* write file */
					fd = open(path, O_WRONLY|O_CREAT, mode);
					if(fd == -1 || ftruncate(fd, size) == -1)
						return -1;
					r = ringwrite(&ring, fd, size);
					if(r == -1)
						return -1;
					if(r < size && mmapcopy(fd, r, size) == -1)
						return -1;
					if(close(fd) == -1)
						return -1;
					path[slash[level]] = '\0';
				}
				break;
			default:
				return errno = EPROTO, -1;
		}
	}
}

int split(char *name, char **user, char **host, char **file) {
	char *colon, *slash, *at;
	
	colon = strrchr(name, ':');
	slash = strrchr(name, '/');
	
	if(!colon || (slash && slash < colon))
		return 0;
	
	at = strrchr(name, '@');
	
	if(at && at > colon)
		at = NULL;
	
	*colon++ = '\0';
	*file = colon;
	
	if(at) {
		*at++ = '\0';
		*user = name;
		*host = at;
	} else
		*host = name;
	
	return 1;
}
	
#if 0
int remote(char *user, char *host) {
	/* Resolve hostname and try to make a connection */
	
	memset(&hint, '\0', sizeof(hint));
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	
	err = getaddrinfo(host, port, &hint, &ai);
	
	if(err) {
		fprintf(stderr, "%s: Error looking up host: %s\n", argv0, gai_strerror(err));
		return -1;
	}
	
	hint.ai_flags = AI_PASSIVE;
	
	for(aip = ai; aip; aip = aip->ai_next) {
		if(getnameinfo(aip->ai_addr, aip->ai_addrlen, hostaddr, sizeof(hostaddr), portnr, sizeof(portnr), NI_NUMERICHOST | NI_NUMERICSERV)) {
			fprintf(stderr, "%s: Error resolving address: %s\n", argv0, strerror(errno));
			return -1;
		}
		fprintf(stderr, "Trying %s port %s...",	hostaddr, portnr);
		
		if((sock = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol)) == -1) {
			fprintf(stderr, " Could not open socket: %s\n", strerror(errno));
			continue;
		}

		hint.ai_family = aip->ai_family;

		/* Bind to a privileged port */
				
		for(i = 1023; i >= 512; i--) {
			snprintf(lport, sizeof(lport), "%d", i);
			err = getaddrinfo(NULL, lport, &hint, &lai);
			if(err) {
				fprintf(stderr, " Error looking up localhost: %s\n", gai_strerror(err));
				return -1;
			}
			
			err = bind(sock, lai->ai_addr, lai->ai_addrlen);
			
			freeaddrinfo(lai);
			
			if(err)
				continue;
			else
				break;
		}
		
		if(err) {
			fprintf(stderr, " Could not bind to privileged port: %s\n", strerror(errno));
			continue;
		}
		
		if(connect(sock, aip->ai_addr, aip->ai_addrlen) == -1) {
			fprintf(stderr, " Connection failed: %s\n", strerror(errno));
			continue;
		}
		fprintf(stderr, " Connected.\n");
		break;
	}
	
	if(!aip) {
		fprintf(stderr, "%s: Could not make a connection.\n", argv0);
		return -1;
	}
	
	/* Send required information to the server */
	
	if(safewrite(sock, "0", 2) == -1 || 
	   safewrite(sock, luser, strlen(luser) + 1) == -1 ||
	   safewrite(sock, user, strlen(user) + 1) == -1 ||
	   safewrite(sock, command, strlen(user) + 1) == -1) {
		fprintf(stderr, "%s: Unable to send required information: %s\n", argv0, strerror(errno));
		return -1;
	}

	/* Wait for acknowledgement from server */
	
	errno = 0;
	
	if(read(sock, buf, 1) != 1 || *buf) {
		fprintf(stderr, "%s: Didn't receive NULL byte from server: %s\n", argv0, strerror(errno));
		return -1;
	}
	
	return sock;
}
#endif

int from(int argc, char **argv, bool recurse, bool dir) {
	if(!argc) {
		send_error("Not enough arguments.");
		return 1;
	}
}

int to(int argc, char **argv, bool recurse, bool dir) {
	char buf[BUFSIZE];
	char path[PATH_MAX];
	struct ring_t ring = {buf, sizeof buf, 0, 0};

	if(argc != 1) {
		send_error("One argument expected.");
		return 1;
	}

	for(;;) {
		if(!ring.fill) {
			r = ringread(STDIN_FILENO, &ring);
			if(r <= 0)
				return -1;
		}
		
		cmd = ring.buf[ring.off];
		switch(cmd) {
			case '\01':
			case '\02':
			case 'T':
			case 'C':
			case 'D':
			case 'E':
			default:
				send_error("Unknown command '%c'", cmd);
				return -1;
		}
	}

	return 0;
}

int local(int argc, char **argv, bool recurse, bool dir) {
	char *dest;

	if(argc < 2) {
		fprintf(stderr, "%s: Not enough arguments!\n", argv0);
		usage();
		return 1;
	}
	
	dest = argv[--argc];
}

int main(int argc, char **argv) {
	int opt;

	bool preserve = false, recurse = false, f = false, t = false, dir = false;
	
	argv0 = argv[0];
	
	/* Process options */
			
	while((opt = getopt(argc, argv, "+rpdft")) != -1) {
		switch(opt) {
			case 'p':
				preserve = true;
				break;
			case 'r':
				recurse = true;
				break;
			case 'd':
				dir = true;
				break;
			case 'f':
				f = 1;
				break;
			case 't':
				t = 1;
				break;
			default:
				fprintf(stderr, "%s: Unknown option!\n", argv0);
				usage();
				return 1;
		}
	}
	
	argc -= optind;
	argv += optind;
	
	if(f && t) {
		fprintf(stderr, "%s: specify only one of -f and -t!\n", argv0);
		return 1;
	}

	if(f)
		return from(argc, argv, recurse, dir);

	if(t)
		return to(argc, argv, recurse, dir);

	return local(argc, argv, recurse, dir);
}

