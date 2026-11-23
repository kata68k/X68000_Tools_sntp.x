#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<signal.h>
#include<limits.h>
#ifdef __human68k__
#undef __DOS_INLINE__
#define __DOS_INLINE__
#include<sys/dos.h>
#endif
#include<sys/iocs.h>

#include "network.h"
#include "socket.h"
#include "ntp.h"

#define NTP_PACKET_SIZE 48

/* パケット内オフセット */
#define NTP_LI_VN_MODE      0
#define NTP_STRATUM         1

#define NTP_TX_SEC          40
#define NTP_TX_FRAC         44

static void wait_ms(int ms)
{
    clock_t start;
    clock_t ticks;

    start = clock();

    ticks = (CLOCKS_PER_SEC * (clock_t)ms) / 1000;

    while ((clock() - start) < ticks)
        ;
}

static unsigned long get_be32(const unsigned char *p)
{
    return ((unsigned long)p[0] << 24)
         | ((unsigned long)p[1] << 16)
         | ((unsigned long)p[2] <<  8)
         | ((unsigned long)p[3]);
}

static void put_be32(unsigned char *p,unsigned long v)
{
    p[0]=(unsigned char)(v>>24);
    p[1]=(unsigned char)(v>>16);
    p[2]=(unsigned char)(v>>8);
    p[3]=(unsigned char)(v);
}

static void make_request(unsigned char packet[48])
{
    memset(packet,0,48);

    /* LI=0  VN=4  MODE=3(Client) */

    packet[NTP_LI_VN_MODE]=0x23;
}

static int check_reply(unsigned char packet[48])
{
    int mode;

    mode = packet[NTP_LI_VN_MODE] & 7;

    if(mode != 4)
        return NTP_ERR_PACKET;

    if(packet[NTP_STRATUM] == 0)
        return NTP_ERR_PACKET;

    return NTP_SUCCESS;
}

static unsigned long packet_to_unix(unsigned char packet[48])
{
    unsigned long ntp;

    ntp = get_be32(&packet[NTP_TX_SEC]);

    return ntp - NTP_DELTA;
}

int rtc_set(unsigned long unix_time)
{
    unsigned long date = 0;
    unsigned long time = 0;
    int datebcd;
    int timebcd;

    NTP_DATETIME dt;

    /* UTC→JST */
    unix_time += 9UL * 3600UL;

    ntp_unix_to_datetime(unix_time, &dt);

    date =
        ((unsigned long)dt.year << 16) |
        ((unsigned long)dt.month << 8) |
        (unsigned long)dt.day;
    printf("date(%d/%d/%d)\n", dt.year, dt.month, dt.day);

    time =
        ((unsigned long)dt.hour << 16) |
        ((unsigned long)dt.minute << 8) |
        (unsigned long)dt.second;
    printf("time(%d:%d:%d)\n", dt.hour, dt.minute, dt.second);

    datebcd = _iocs_bindatebcd(date);
    if(datebcd < 0)
    {
        printf("date(0b%32b)\n", datebcd);
    }
    else
    {
        _iocs_bindateset(datebcd);
    }

    timebcd = _iocs_timebcd(time);
    if(timebcd < 0)
    {
        printf("time(0b%32b)\n", timebcd);
    }
    else
    {
        _iocs_timeset(timebcd);
    }

    return 0;
}

int ntp_query(
    const char *server,
    int timeout_ms,
    unsigned long *unix_time)
{
    int sock;
    int len;
    int addrlen;
#if 0
    int i;
#endif
    clock_t start;

    struct sockaddr_in addr;
    struct hostent *host;
    struct sockaddr_in local;

    unsigned char packet[48];

    if(unix_time==NULL)
        return NTP_ERR_PACKET;

    memset(&addr,0,sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(NTP_PORT);

    /*
     * IPアドレス文字列ならそのまま使う
     */
    addr.sin_addr.s_addr = a2n_ipaddr((char *)server);
    printf("server=%s(%d)\n", server, addr.sin_addr.s_addr);

    /*
     * DNS名なら名前解決
     */
    if(addr.sin_addr.s_addr==0)
    {
        host = gethostbyname((char *)server);

        if(host==NULL)
            return NTP_ERR_DNS;

//        printf("host=%s(%d)\n", host->h_name, host->h_length);
//        host->h_length = strlen(host->h_name);
//        printf("host=%s(%d)\n", host->h_name, host->h_length);

        memcpy(&addr.sin_addr,
               host->h_addr,
               4);
               //host->h_length);

        printf("IP=%lu.%lu.%lu.%lu\n",
            ((unsigned char *)host->h_addr)[0],
            ((unsigned char *)host->h_addr)[1],
            ((unsigned char *)host->h_addr)[2],
            ((unsigned char *)host->h_addr)[3]);
    }

    sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock<0)
        return NTP_ERR_SOCKET;

    wait_ms(1000);

    memset(&local, 0, sizeof(local));
    local.sin_family      = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port        = 0;      /* 任意のローカルポート */

    if (bind(sock, (char *)&local, sizeof(local)) < 0)
    {
        printf("bind error\n");
        close_s(sock);
        return NTP_ERR_SOCKET;
    }

    wait_ms(1000);

    make_request(packet);

    len = sendto(
            sock,
            (char *)packet,
            48,
            0,
            (char *)&addr,
            sizeof(addr));

#if 0
    printf("sendto(len) = %d\n", len);
    printf("sock=%d\n", sock);
    printf("socklen(0)=%d\n", socklen(sock,0));
    printf("socklen(1)=%d\n", socklen(sock,1));
    for (i = 0; i < 48; i++)
    {
        printf("%02X ", packet[i]);
        if ((i & 15) == 15)
            printf("\n");
    }
#endif

    start = clock();

    for (;;)
    {
        if (socklen(sock, 0) > 0)
            break;

        if ((clock() - start) >= timeout_ms)
        {
            close_s(sock);
            return NTP_ERR_TIMEOUT;
        }
    }

    addrlen = sizeof(addr);
#if 0
    printf("addrlen=%d\n", addrlen);
#endif

    len = recvfrom(
            sock,
            (char *)packet,
            sizeof(packet),
            0,
            (char *)&addr,
            &addrlen);
#if 0
    printf("recvfrom(len) = %d\n", len);
#endif
    if (len != 48)
    {
        close_s(sock);
        return NTP_ERR_RECV;
    }

    wait_ms(1000);

    if(check_reply(packet)!=NTP_SUCCESS)
    {
        close_s(sock);
        return NTP_ERR_PACKET;
    }

    *unix_time = packet_to_unix(packet);

    close_s(sock);

    return NTP_SUCCESS;
}

void
main (int argc, char **argv)
{
    int ret;
    unsigned long t;

    puts("sntp.x ver1.1 for X680x0 by kata68k 2026/11/23");

    ret = ntp_query("ntp.nict.jp",3000,&t);
    if(ret == NTP_SUCCESS)
    {
        printf("UNIX=%lu\n",t);
        rtc_set(t);
    }
    else
    {
        printf("ERROR: %d\n", ret);
    }
    return;
}

