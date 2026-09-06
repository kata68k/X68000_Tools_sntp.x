/****************************************************************************
 *
 *  ntp.h
 *
 *  Simple SNTP Client for Human68k
 *
 *  C90 Compatible
 *  32bit Integer Only
 *
 ****************************************************************************/

#ifndef NTP_H
#define NTP_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
    Constants
****************************************************************************/

/* UDP Port */
#define NTP_PORT           123

/* NTP Epoch -> UNIX Epoch */
#define NTP_DELTA          2208988800UL

/* Default Timeout (ms) */
#define NTP_TIMEOUT        3000

/* Error Codes */
#define NTP_SUCCESS         0
#define NTP_ERR_SOCKET     -1
#define NTP_ERR_DNS        -2
#define NTP_ERR_SEND       -3
#define NTP_ERR_TIMEOUT    -4
#define NTP_ERR_RECV       -5
#define NTP_ERR_PACKET     -6

#define INADDR_ANY ((unsigned long)0)

/****************************************************************************
    NTP Packet
****************************************************************************/

typedef struct
{
    unsigned char  li_vn_mode;
    unsigned char  stratum;
    unsigned char  poll;
    signed char    precision;

    unsigned long  root_delay;
    unsigned long  root_dispersion;
    unsigned long  reference_id;

    unsigned long  ref_sec;
    unsigned long  ref_frac;

    unsigned long  org_sec;
    unsigned long  org_frac;

    unsigned long  recv_sec;
    unsigned long  recv_frac;

    unsigned long  tx_sec;
    unsigned long  tx_frac;

} NTP_PACKET;


/****************************************************************************
    Date Time
****************************************************************************/

typedef struct
{
    int year;
    int month;
    int day;

    int hour;
    int minute;
    int second;

    int weekday;

} NTP_DATETIME;


/****************************************************************************
    API
****************************************************************************/

/*
    NTP Server Query

    server      : Hostname or IP Address
    timeout_ms  : Receive timeout
    unix_time   : Result UNIX Time

    return
        NTP_SUCCESS
        error code
*/
int ntp_query(
        const char *server,
        int timeout_ms,
        unsigned long *unix_time);


/*
    UNIX -> DateTime
*/
void ntp_unix_to_datetime(
        unsigned long unix_time,
        NTP_DATETIME *dt);


/*
    Set Human68k System Clock

    (User implementation or rtc.c)
*/
int ntp_set_system_time(
        const NTP_DATETIME *dt);


/****************************************************************************
    Utility
****************************************************************************/

/*
    Leap Year
*/
int ntp_is_leap_year(int year);


/*
    Days in Month
*/
int ntp_days_in_month(
        int year,
        int month);


/*
    Weekday

    0 = Sunday
*/
int ntp_weekday(
        unsigned long unix_time);


#ifdef __cplusplus
}
#endif

#endif