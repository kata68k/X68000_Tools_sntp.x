/*
 * socket.h
 *
 * Copyright (C) 1994 First Class Technology.
 */

#ifndef __socket_h__
#define __socket_h__

#define	AF_INET		0
#define	AF_LOCAL	1

#define	SOCK_STREAM	0
#define	SOCK_DGRAM	1
#define	SOCK_RAW	2

#define	SOCK_BINARY	0	/* socket in raw (binary) mode */
#define	SOCK_ASCII	1	/* socket in cooked (newline mapping) mode */
#define	SOCK_QUERY	2	/* Return setting without change */

#define ESOCKTNOSUPPORT	100
#define EAFNOSUPPORT	101
#define	EAFNOSUPP	102
#define	EISCONN		103
#define	EOPNOTSUPP	104
#define	ECONNREFUSED	105
#define	ENOTCONN	106
#define	EADDRINUSE	107
#define	EMSGSIZE	108
#define	EALARM		109
#define	EABORT		110

#define	NOTUSED			0
#define	TYPE_TCP		1
#define	TYPE_UDP		2
#define TYPE_RAW		3
#define	TYPE_LOCAL_STREAM	4
#define	TYPE_LOCAL_DGRAM	5

struct sockaddr
{
  short sa_family;
  char sa_data[14];
};

struct in_addr
{
  unsigned long s_addr;
};

struct sockaddr_in
{
  short sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct loc
{
  struct mbuf *q;
  int hiwat;			/* Flow control point */
  int flags;
#define	LOC_SHUTDOWN	1
};

#define	LOCDFLOW	5	/* dgram socket flow-control point, packets */
#define	LOCSFLOW	2048	/* stream socket flow control point, bytes */
#define	SOBUF		256	/* Size of buffer for usputc()/usprintf() */
#define	SOCKBASE	128	/* Start of socket indexes */

union cb
{
  struct tcb *tcb;
  struct udp_cb *udp;
  struct raw_ip *rip;
  struct loc *local;
  char *p;
};

union sp
{
  struct sockaddr *sa;
  struct sockaddr_in *in;
  char *p;
};

int accept (int, char *, int *);
int bind (int, char *, int);
int close_s (int);
int connect (int, char *, int);
int listen (int, int);
int socket (int, int, int);
int read_s (int, char *, int);
int recvfrom (int, char *, int, int, char *, int *);
int write_s (int, char *, int);
int sendto (int, char *, int, int, char *, int);
int socklen (int, int);

int getpeername (int, char *, int *);
int getsockname (int, char *, int *);
int setflush (int, int);
int shutdown (int, int);
int sockkick (int);
int usesock (int);
int recvchar (int);
int recvline (int, char *, unsigned int);
int rrecvchar (int);
int seteol (int, char *);
int sockmode (int, int);
void usflush (int s);
char *psocket (void *);
char *sockerr (int);
char *sockstate (int);

#endif
