/****************************************************************************
 *
 * date.c
 *
 * UNIX Time -> DateTime
 *
 * C90
 * 32bit Integer Only
 *
 ****************************************************************************/

#include "ntp.h"

static const unsigned char month_days[12] =
{
    31,28,31,30,31,30,
    31,31,30,31,30,31
};

void ntp_unix_to_datetime(
    unsigned long unix_time,
    NTP_DATETIME *dt);

int ntp_is_leap_year(int year);

int ntp_days_in_month(int year,int month);

int ntp_weekday(unsigned long unix_time);

/*--------------------------------------------------------------------------*/

int ntp_is_leap_year(int year)
{
    if ((year % 400) == 0)
        return 1;

    if ((year % 100) == 0)
        return 0;

    if ((year % 4) == 0)
        return 1;

    return 0;
}


/*--------------------------------------------------------------------------*/

int ntp_days_in_month(int year,int month)
{
    if(month==2)
    {
        if(ntp_is_leap_year(year))
            return 29;
    }

    return month_days[month-1];
}

/****************************************************************************/
int ntp_weekday(unsigned long unix_time)
{
    unsigned long days;

    days = unix_time / 86400UL;

    return (int)((days + 4) % 7);

    /*
        0 Sunday
        1 Monday
        ...
        6 Saturday
    */
}

/****************************************************************************
 * UNIX Time -> DateTime
 ****************************************************************************/

void ntp_unix_to_datetime(
    unsigned long unix_time,
    NTP_DATETIME *dt)
{
    unsigned long days;
    unsigned long secs;

    int year;
    int month;

    int dim;

    /* •b */

    secs = unix_time;

    dt->second = (int)(secs % 60UL);
    secs /= 60UL;

    dt->minute = (int)(secs % 60UL);
    secs /= 60UL;

    dt->hour = (int)(secs % 24UL);

    /* “ú” */

    days = unix_time / 86400UL;

    /* —j“ú */

    dt->weekday = (int)((days + 4UL) % 7UL);

    /* ”N */

    year = 1970;

    for (;;)
    {
        unsigned long ydays;

        if (ntp_is_leap_year(year))
            ydays = 366UL;
        else
            ydays = 365UL;

        if (days < ydays)
            break;

        days -= ydays;
        year++;
    }

    dt->year = year;

    /* ŒŽ */

    month = 1;

    for (;;)
    {
        dim = ntp_days_in_month(year, month);

        if (days < (unsigned long)dim)
            break;

        days -= (unsigned long)dim;
        month++;
    }

    dt->month = month;
    dt->day   = (int)days + 1;
}