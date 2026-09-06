/*
 * tcpipdrv.h
 *
 * Copyright (C) 1994 First Class Technology.
 */

#ifndef __tcpipdrv_h__
#define __tcpipdrv_h__

typedef enum
{
  _TI_get_version,

  _TI_add_arp_table,
  _TI_del_arp_table,
  _TI_search_arp_table,
  _TI_get_arp_table_top,
  _TI_arp_request,

  _TI_get_iface_list,
  _TI_get_new_iface,
  _TI_link_new_iface,

  _TI_rt_top,
  _TI_rt_lookup,
  _TI_rt_lookupb,
  _TI_rt_drop,
  _TI_rt_add,

  _TI_dns_add,
  _TI_dns_drop,
  _TI_dns_get,
  _TI_set_domain_name,
  _TI_get_domain_name,
  _TI_res_query,
  _TI_res_search,
  _TI_res_mkquery,
  _TI_res_sendquery,

  _TI_get_MIB,

  _TI_socket,
  _TI_bind,
  _TI_listen,
  _TI_accept,
  _TI_connect,
  _TI_read_s,
  _TI_write_s,
  _TI_recvfrom,
  _TI_sendto,
  _TI_close_s,
  _TI_socklen,
  _TI_getsockname,
  _TI_getpeername,
  _TI_sockkick,
  _TI_shutdown,
  _TI_usesock,
  _TI_recvline,
  _TI_sendline,
  _TI_rrecvchar,
  _TI_recvchar,
  _TI_usflush,
  _TI_seteol,
  _TI_sockmode,
  _TI_setflush,
  _TI_psocket,
  _TI_sockerr,
  _TI_sockstate,

  _TI_sock_top,
  _TI_ntoa_sock,

  _TI_gethostbyname,
  _TI_gethostbyaddr,
  _TI_getnetbyname,
  _TI_getnetbyaddr,
  _TI_getservbyname,
  _TI_getservbyport,
  _TI_getprotobyname,
  _TI_getprotobynumber,

  _TI_rip,
} cmd;

typedef long (*_ti_func) (long, void *);

#endif
