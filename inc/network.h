/*
 * network.h
 *
 * Copyright (C) 1994 First Class Technology.
 */

#ifndef __network_h__
#define __network_h__

/* Interface section */
#define LOOPBACK_ADDR ((127 << 24) + 1)
#define MAX_HW_ADDR_LEN	(16)

#define IFACE_UP 	(1)
#define	IFACE_RUNNING	(2)
#define	IFACE_NOARP	(4)
#define	IFACE_NOTRAIL	(8)
#define	IFACE_BROAD	(16)
#define	IFACE_LOOPBACK	(32)
#define	IFACE_PTP	(64)

typedef struct iface {
  struct iface *next;
  char *name;

  int (*config) (struct iface *);
  int (*stop) (struct iface *);
  int (*update) (struct iface *);
  int (*send) (struct iface *, void *, long, int, int, int, int);
  int (*output) (struct iface *, char *, char *, long, void *);
  void (*input) (struct iface *, void *);
  int (*fprint) (FILE *, char *);
  int (*sprint) (char *, char *);
  int (*fscan) (FILE *, char *);
  int (*sscan) (char *, char *);

  int mtu;
  long my_ip_addr;
  long net_mask;
  long broad_cast;
  short arp_type;
  short arp_ip_type;
  short arp_hw_type;
  int hw_addr_len;
  char my_hw_addr[MAX_HW_ADDR_LEN];
  char my_hw_broad_addr[MAX_HW_ADDR_LEN];

  struct iface *forw;
  int flag;
  long ipsndcnt;
  long rawsndcnt;
  long snderrcnt;
  long iprcvcnt;
  long rawrcvcnt;
  long rcverrcnt;
  long collcnt;
  long lastsent;

  long data;
} iface;

/* domain name section */
struct dns
{
  struct dns *prev;
  struct dns *next;

  long address;
};

/* type values for resources and queries */
#define	T_A	1
#define	T_NS	2
#define	T_MD	3
#define	T_MF	4
#define	T_CNAME	5
#define	T_SOA	6
#define	T_MB	7
#define	T_MG	8
#define	T_MR	9
#define	T_NULL	10
#define	T_WKS	11
#define	T_PTR	12
#define	T_HINFO	13
#define	T_MINFO	14
#define	T_MX	15
#define	T_TXT	16
#define	T_ANY	255

/* Values for class */
#define	C_IN	1
#define C_ANY	255

/* Values for opcode */
#define QUERY	0
#define IQUERY	1

struct rrec {
  short r_zone;			/* zone number */
  short r_class;		/* class number */
  short r_type;			/* type number */
  unsigned long r_ttl;		/* time to live */
  int r_size;			/* size of data area */
  char *r_data;			/* pointer to data */
};

struct hostent
{
  char *h_name;
  char **h_aliases;
  int h_addrtype;
  int h_length;
  char **h_addr_list;
};
#define h_addr  h_addr_list[0]

struct netent
{
  char *n_name;
  char **n_aliases;
  int n_addrtype;
  unsigned long n_net;
};

struct protoent
{
  char *p_name;
  char **p_aliases;
  int p_proto;
};

struct servent {
  char *s_name;
  char **s_aliases;
  int s_port;
  char *s_proto;
};

/* mib section */
struct mib_entry
{
  char *name;
  union
    {
      long integer;
    } value;
};

struct mib_array {
  struct mib_entry* mib;
  int mib_size;
  char *name;
};

/* route section */
#define NROUTE 5

typedef struct route
{
  struct route *prev;
  struct route *next;
  long target;
  unsigned int bits;
  long gateway;
  long metric;
  struct iface *iface;
  int flags;
  int ttl;
  int _ttl;
  long uses;
} route;

typedef route *rtable[32][NROUTE];

/* functions */
long _get_version (void);

char *search_arp_table (long, int, char *);
void delete_arp_table (long);
void add_arp_table (long, char *);
long *get_arp_array (int *);

int isipaddr (char *);
char *n2a_ipaddr (long, char *);
long a2n_ipaddr (char *);
char *n2a_eaddr (char *, char *, int);
char *a2n_eaddr (char *, char *, int);

struct hostent *gethostbyname (char *);
struct hostent *gethostbyaddr (char *, int, int);
struct netent *getnetbyname (char *);
struct netent *getnetbyaddr (long, int);
struct servent *getservbyname (char *, char *);
struct servent *getservbyport (int, char *);
struct protoent *getprotobyname (char *);
struct protoent *getprotobynumber (int);

iface *get_new_iface (char *);
void link_new_iface (iface *);
long *get_my_ipaddr (int *);
iface *get_iface_list (void);
iface *iface_lookup (long);
iface *iface_lookupn (char *);
int ismyipaddr (long);

rtable *rt_top (route **);
route *rt_lookup (long);
route *rt_lookupb (long, unsigned int);
int rt_drop (long, unsigned int);
route *rt_add (long, unsigned int, long, iface *, long, long, char);

long *get_sock_array (int *);
char *ntoa_sock (int, int, char *, int);

struct mib_array * get_mib_list (void);

int dns_add (long);
int dns_drop (long);
struct dns *dns_get (void);
int set_domain_name (char *);
char *get_domain_name (void);
int res_query (char *, int, int, unsigned char *, int);
int res_search (char *, int, int, unsigned char *, int);
int res_mkquery (int, char *, int, int, char *, int, struct rrec *, char *, int);
int res_send (char *, int, char *, int);

unsigned long htonl (unsigned long);
unsigned short htons (unsigned short);
unsigned long ntohl (unsigned long);
unsigned short ntohs (unsigned short);

enum {
  RIP_ON = 0,
  RIP_OFF,
  RIP_STAT,
};

int rip_set (int);
#endif
