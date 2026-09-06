/*
 * etherdrv.h
 *
 * Copyright (C) 1994 First Class Technology.
 */

#ifndef __etherdrv_h__
#define __etherdrv_h__

/*
** Ether Driver Command Set
*/
typedef enum
{
  etd_get_driver_version = 0,
  etd_get_mac_addr,
  etd_get_prom_addr,
  etd_set_mac_addr,
  etd_send_ether_packet,
  etd_set_int_addr,
  etd_get_int_addr,
  etd_del_int_addr,
  etd_set_multicast_addr,
  etd_get_statistics,
} etd_cmd;

#ifndef __EADDR_DEFINED__
#define __EADDR_DEFINED__
typedef struct
{
  unsigned char _eaddr[6];
} eaddr;
#endif


/*
** typedefs
*/
typedef long (*etd_func)(etd_cmd, void *);

/*
** Library functions
*/
long ETDGetDriverVersion (char *, int *ino);
eaddr *ETDGetMacAddr (eaddr *, int ino);
eaddr *ETDGetPromAddr (eaddr *, int ino);
int ETDSetMacAddr (eaddr *, int ino);
int ETDSendPacket (int, void *, int ino);
int ETDSetIntAddr (unsigned long, void*, int ino);
void* ETDGetIntAddr (unsigned long, int ino);
int ETDDelIntAddr (unsigned long, int ino);

int ETDSetMulticastAddr (char *, int ino);
int ETDGetStatistics (char *, int ino);

#endif
