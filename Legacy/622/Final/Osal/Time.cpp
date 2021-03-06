// Time.cpp: implementation of the Time namespace
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA  02111-1307, USA.

#define OSAL_BUILD
#include "Time.h"
#include "String.h"
#include "Algorithm.h"
#undef OSAL_BUILD

#include <stdio.h>

static const char* pszDay[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    "ERR"
};

static const char* pszDayOfWeek[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "ERROR"
};


static const char* pszMonth[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

static const char* pszRealMonth[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void Time::ZeroTime (UTCTime* ptUTCTime) {

    *ptUTCTime = 0;
}

void Time::GetTime (UTCTime* ptUTCTime) {

    *ptUTCTime = time (NULL);
}

void Time::GetTime (int iSec, int iMin, int iHour, int iDay, int iMonth, int iYear, UTCTime* ptUTCTime) {

    struct tm tmStruct;
    memset (&tmStruct, 0, sizeof (struct tm));
        
    tmStruct.tm_sec = iSec;
    tmStruct.tm_min = iMin;
    tmStruct.tm_hour = iHour;
    tmStruct.tm_mday = iDay;
    tmStruct.tm_mon = iMonth - 1;
    tmStruct.tm_year = iYear - 1900;

    *ptUTCTime = mktime (&tmStruct);
}

int64 Time::GetUnixTime() {
    return GetUnixTime (time (NULL));
}

int64 Time::GetUnixTime (const UTCTime& tTime) {
    return (int64) tTime;
}

void Time::FromUnixTime (int64 i64Time, UTCTime* ptUTCTime) {
    *ptUTCTime = (UTCTime) i64Time;
}

int Time::GetTimeZone (char pszTimeZone[OS::MaxTimeZoneLength], int* piBias) {

#ifdef __LINUX__
    return ERROR_FAILURE;
#else if defined __WIN32__

    TIME_ZONE_INFORMATION tzInfo;
    DWORD dwRetVal = ::GetTimeZoneInformation (&tzInfo);

    if (dwRetVal == TIME_ZONE_ID_INVALID) {
        return ERROR_FAILURE;
    }

    if (*(tzInfo.DaylightName) != L'\0') {
        wcstombs (pszTimeZone, tzInfo.DaylightName, OS::MaxTimeZoneLength);
    } else {
        wcstombs (pszTimeZone, tzInfo.StandardName, OS::MaxTimeZoneLength);
    }

    *piBias = - (tzInfo.Bias + tzInfo.DaylightBias);
    return OK;
#endif
}

int Time::GetTimeZoneBias (int* piBias) {

#ifdef __LINUX__
    return ERROR_FAILURE;
#else if defined __WIN32__

    TIME_ZONE_INFORMATION tzInfo;
    DWORD dwRetVal = ::GetTimeZoneInformation (&tzInfo);

    if (dwRetVal == TIME_ZONE_ID_INVALID) {
        return ERROR_FAILURE;
    }

    *piBias = - (tzInfo.Bias + tzInfo.DaylightBias);
    return OK;
#endif
}

void Time::GetDate (
    int* piSec, int* piMin, int* piHour, DayOfWeek* pdayOfWeek, int* piDay, int* piMonth, int* piYear) {

    GetDate (time (NULL), piSec, piMin, piHour, pdayOfWeek, piDay, piMonth, piYear);
}

void Time::GetDate (
    const UTCTime& tTime,
    int* piSec, int* piMin, int* piHour, DayOfWeek* pdayOfWeek, int* piDay, int* piMonth, int* piYear) {

    tm* ptmTime = localtime ((time_t*) &tTime);

    *piSec = ptmTime->tm_sec;
    *piMin = ptmTime->tm_min;
    *piHour = ptmTime->tm_hour;

    *pdayOfWeek = (DayOfWeek) ptmTime->tm_wday;

    *piDay = ptmTime->tm_mday;
    *piMonth = ptmTime->tm_mon + 1;
    *piYear = ptmTime->tm_year + 1900;
}
    
int Time::GetTimeString (char pszTimeString[OS::MaxTimeLength]) {

    UTCTime tTime;
    GetTime (&tTime);

    return GetTimeString (tTime, pszTimeString);
}


int Time::GetTimeString (const UTCTime& tTime, char pszTimeString[OS::MaxTimeLength]) {

    tm* ptmTime = localtime ((time_t*) &tTime);
    if (ptmTime == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    char pszHour[8], pszMin[8], pszSec[8];

    String::ItoA (ptmTime->tm_hour, pszHour, 10, 2);
    String::ItoA (ptmTime->tm_min, pszMin, 10, 2);
    String::ItoA (ptmTime->tm_sec, pszSec, 10, 2);

    sprintf (pszTimeString, "%s:%s:%s", pszHour, pszMin, pszSec);

    return OK;
}


int Time::GetDateString (char pszDateString[OS::MaxDateLength]) {
    
    UTCTime tTime;
    GetTime (&tTime);

    return GetDateString (tTime, pszDateString);
}


int Time::GetDateString (const UTCTime& tTime, char pszDateString[OS::MaxDateLength]) {

    tm* ptmTime = localtime ((const time_t*)&tTime);
    if (ptmTime == NULL || ptmTime->tm_wday > 6 || ptmTime->tm_mon > 11) {
        return ERROR_INVALID_ARGUMENT;
    }

    sprintf (
        pszDateString,
        "%.2d:%.2d:%.2d %s, %.2d %s %.4d",
        ptmTime->tm_hour,
        ptmTime->tm_min,
        ptmTime->tm_sec,
        pszDay [ptmTime->tm_wday],
        ptmTime->tm_mday,
        pszMonth[ptmTime->tm_mon],
        ptmTime->tm_year + 1900
        );

    return OK;
}


int Time::GetGMTDateString (char pszGMTDateString[OS::MaxGMTDateLength]) {

    UTCTime tTime;
    GetTime (&tTime);

    return GetGMTDateString (tTime, pszGMTDateString);
}

int Time::GetGMTDateString (const UTCTime& tTime, char pszGMTDateString[OS::MaxGMTDateLength]) {

    tm* ptmTime = gmtime ((const time_t*)&tTime);
    if (ptmTime == NULL || ptmTime->tm_wday > 6 || ptmTime->tm_mon > 11) {
        return ERROR_INVALID_ARGUMENT;
    }

    sprintf (
        pszGMTDateString,
        "%s, %.2d %s %.4d %.2d:%.2d:%.2d GMT",
        pszDay [ptmTime->tm_wday],
        ptmTime->tm_mday,
        pszMonth[ptmTime->tm_mon],
        ptmTime->tm_year + 1900,
        ptmTime->tm_hour,
        ptmTime->tm_min,
        ptmTime->tm_sec
        );

    return OK;
}

int Time::GetCookieDateString (char pszCookieDateString[OS::MaxCookieDateLength]) {

    UTCTime tTime;
    GetTime (&tTime);

    return GetCookieDateString (tTime, pszCookieDateString);
}

int Time::GetCookieDateString (const UTCTime& tTime, char pszCookieDateString[OS::MaxCookieDateLength]) {

    tm* ptmTime = gmtime((const time_t*)&tTime);
    if (ptmTime == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    char pszHour[8], pszMin[8], pszSec[8], pszDayM[8];
    
    String::ItoA (ptmTime->tm_hour, pszHour, 10, 2);
    String::ItoA (ptmTime->tm_min, pszMin, 10, 2);
    String::ItoA (ptmTime->tm_sec, pszSec, 10, 2);
    String::ItoA (ptmTime->tm_mday, pszDayM, 10, 2);
    
    sprintf (
        pszCookieDateString,
        "%s, %s-%s-%i %s:%s:%s GMT",
        pszDay [ptmTime->tm_wday],
        pszDayM,
        pszMonth[ptmTime->tm_mon],
        ptmTime->tm_year + 1900,
        pszHour,
        pszMin,
        pszSec
        );
    
    return OK;
}

int Time::GetSmtpDateString (char pszCookieDateString[OS::MaxSmtpDateLength]) {

    UTCTime tTime;
    GetTime(&tTime);

    return GetSmtpDateString (tTime, pszCookieDateString);
}

int Time::GetSmtpDateString (const UTCTime& tTime, char pszCookieDateString[OS::MaxSmtpDateLength]) {

    tm* ptmTime = localtime((const time_t*)&tTime);
    if (ptmTime == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    char pszHour[8], pszMin[8], pszSec[8], pszDayM[8];
    
    String::ItoA (ptmTime->tm_hour, pszHour, 10, 2);
    String::ItoA (ptmTime->tm_min, pszMin, 10, 2);
    String::ItoA (ptmTime->tm_sec, pszSec, 10, 2);
    String::ItoA (ptmTime->tm_mday, pszDayM, 10, 2);

    char pszOffset [32];
    int iBias;
    if (GetTimeZoneBias (&iBias) == OK) {

        int iBiasBase60 = 100 * (iBias / 60) + (iBias % 60);

        char pszBias[8];
        String::ItoA (iBiasBase60, pszBias, 10, 4);
        strcpy (pszOffset, pszBias);
                        
    } else {
        pszOffset[0] = '\0';
    }

    sprintf (
        pszCookieDateString,
        "%s, %s %s %i %s:%s:%s %s",
        pszDay [ptmTime->tm_wday],
        pszDayM,
        pszMonth[ptmTime->tm_mon],
        ptmTime->tm_year + 1900,
        pszHour,
        pszMin,
        pszSec,
        pszOffset
        );
    
    return OK;
}

int Time::GetDay() {

    UTCTime tTime;
    GetTime (&tTime);

    return GetDay (tTime);
}

int Time::GetDay (const UTCTime& tTime) {

    tm* ptmTime = localtime((const time_t*)&tTime);
    return ptmTime->tm_mday;
}

int Time::GetMonth() {

    UTCTime tTime;
    GetTime (&tTime);

    return GetMonth (tTime);
}

int Time::GetMonth (const UTCTime& tTime) {

    tm* ptmTime = localtime ((time_t*) &tTime);
    return ptmTime->tm_mon + 1;
}

int Time::GetYear() {

    UTCTime tTime;
    GetTime (&tTime);

    return GetYear (tTime);
}

int Time::GetYear (const UTCTime& tTime) {

    tm* ptmTime = localtime ((time_t*) &tTime);
    return ptmTime->tm_year + 1900;
}

DayOfWeek Time::GetDayOfWeek() {

    UTCTime tTime;
    GetTime (&tTime);

    return GetDayOfWeek (tTime);
}


DayOfWeek Time::GetDayOfWeek (const UTCTime& tTime) {

    tm* ptmTime = localtime((const time_t*)&tTime);
    return (DayOfWeek) ptmTime->tm_wday;
}

const char* Time::GetDayOfWeekName (DayOfWeek dayOfWeek) {

    return pszDayOfWeek [dayOfWeek];
}

const char* Time::GetAbbreviatedDayOfWeekName (DayOfWeek dayOfWeek) {

    return pszDay [dayOfWeek];
}

const char* Time::GetMonthName() {

    UTCTime tTime;
    GetTime (&tTime);

    return GetMonthName (tTime);
}

const char* Time::GetMonthName (int iMonth) {

    if (iMonth > 12 || iMonth < 1) {
        return "Invalid month";
    }
    return pszRealMonth [iMonth - 1];
}

const char* Time::GetMonthName (const UTCTime& tTime) {

    tm* ptmTime = localtime((const time_t*)&tTime);
    if (ptmTime == NULL) {
        return "Invalid month";
    }

    return GetMonthName (ptmTime->tm_mon + 1);
}

const char* Time::GetAbbreviatedMonthName() {

    UTCTime tTime;
    GetTime (&tTime);
    return GetAbbreviatedMonthName (tTime);
}

const char* Time::GetAbbreviatedMonthName (int iMonth) {

    if (iMonth > 12 || iMonth < 1) {
        return "Invalid month";
    }
    return pszMonth [iMonth - 1];
}

const char* Time::GetAbbreviatedMonthName (const UTCTime& tTime) {

    tm* ptmTime = localtime((const time_t*)&tTime);
    return GetAbbreviatedMonthName (ptmTime->tm_mon + 1);
}


bool Time::IsWeekendTime() {

    UTCTime tTime;
    GetTime (&tTime);
    return IsWeekendTime (tTime);
}

bool Time::IsWeekendTime (const UTCTime& tTime) {

    tm* ptmTime = localtime((const time_t*)&tTime);
    if (ptmTime == NULL) {
        return false;
    }

    return (ptmTime->tm_wday == 0 || ptmTime->tm_wday == 6);
}

Seconds Time::GetRemainingWeekendSeconds() {

    UTCTime tTime;
    GetTime (&tTime);
    return GetRemainingWeekendSeconds (tTime);
}

Seconds Time::GetRemainingWeekendSeconds (const UTCTime& tTime) {

    Seconds sSeconds;

    tm* ptmTime = localtime((const time_t*)&tTime);
    if (ptmTime == NULL) {
        return 0;
    }

    // Make sure today is saturday or sunday
    switch (ptmTime->tm_wday) {

    case 0:
        sSeconds = 0;
        break;

    case 6:
        sSeconds = 86400;
        break;

    default:
        return 0;
    }

    // Add hours remaining until 23h
    sSeconds += ((23 - ptmTime->tm_hour) * 3600);

    // Add minutes remaining until 59 min
    sSeconds += ((59 - ptmTime->tm_min) * 60);

    // Add secs remaining until 60 sec
    sSeconds += ((60 - ptmTime->tm_sec));

    return sSeconds;
}

Seconds Time::GetWeekendSecondsBetweenTimes (const UTCTime& tStart, const UTCTime& tEnd) {

    Assert (tEnd >= tStart);

    Seconds sDiff = (Seconds) (tEnd - tStart);
    Seconds sModDiff = sDiff % (7 * 24 * 60 * 60);

    // If the start is weeks before the end, mod it to inside the week before the end
    time_t tRealStart;
    tRealStart = sDiff == sModDiff ? tStart : tEnd - sModDiff;

    tm* ptmStart = localtime (&tRealStart);
    if (ptmStart == NULL) {
        Assert (false);
        return 0;
    }

    Seconds sWeekend = ((sDiff - sModDiff) * 2) / 7;

    if (ptmStart->tm_wday == 0 || ptmStart->tm_wday == 6) {

        // Start time is on a weekend - find the end of that weekend

        // Add a day if saturday
        Seconds sThisWeekend;
        if (ptmStart->tm_wday == 6) {
            sThisWeekend = 24 * 60 * 60;   
        } else {
            sThisWeekend = 0;
        }

        // Add hours remaining until 23h
        sThisWeekend += ((23 - ptmStart->tm_hour) * 3600);

        // Add minutes remaining until 59 min
        sThisWeekend += ((59 - ptmStart->tm_min) * 60);

        // Add secs remaining until 60 sec
        sThisWeekend += ((60 - ptmStart->tm_sec));

        time_t tEndOfWeekend = tRealStart + sThisWeekend;
        if (tEndOfWeekend > (time_t)tEnd) {

            // We overshot, but we're done.  Just add the seconds between the two times
            sWeekend += (Seconds) (tEnd - tRealStart);
            return sWeekend;
        }
        
        // Add the rest of the weekend
        sWeekend += sThisWeekend;
    
       time_t tNextWeekendStart = tEndOfWeekend + 5 * 24 * 60 * 60;

       if (tNextWeekendStart > (time_t)tEnd) {

           // We're done, since the end is before the next weekend
           return sWeekend;
       }

       // The end has to fall on a weekend, because otherwise it would be a difference
       // greater than a week

       sWeekend += (Seconds) (tEnd - tNextWeekendStart);
       return sWeekend;

    } else {

        // Start is not on a weekend.  Find the next weekend after start
        Seconds sUntilWeekend = (5 - ptmStart->tm_wday) * 24 * 60 * 60;

        // Add hours remaining until 23h
        sUntilWeekend += ((23 - ptmStart->tm_hour) * 3600);

        // Add minutes remaining until 59 min
        sUntilWeekend += ((59 - ptmStart->tm_min) * 60);

        // Add secs remaining until 60 sec
        sUntilWeekend += ((60 - ptmStart->tm_sec));

        time_t tNextWeekendStart = tRealStart + sUntilWeekend;

        if (tNextWeekendStart > (time_t)tEnd) {

            // The end is before the next weekend, so we're done
            return sWeekend;
        }

        time_t tNextWeekendEnd = tNextWeekendStart + 2 * 24 * 60 * 60;
        
        if (tNextWeekendEnd > (time_t)tEnd) {
 
            // The end falls inside the next weekend, so we're done
            sWeekend += (Seconds) (tEnd - tNextWeekendStart);
            return sWeekend;
        }

        // The end cannot fall on the next weekend, since that would be more than week's difference
        // Add the weekend we're processing and we're done
        sWeekend += 2 * 24 * 60 * 60;
        return sWeekend;
    }
}

void Time::StartTimer (Timer* ptmTimer) {
    _ftime (ptmTimer);
}

MilliSeconds Time::GetTimerCount (const Timer& tmTimer) {

    _timeb tbNow;
    _ftime (&tbNow);

    return (MilliSeconds) ((tbNow.time - tmTimer.time) * 1000 + tbNow.millitm - tmTimer.millitm);
}

Seconds Time::GetSecondDifference (const UTCTime& tFinish, const UTCTime& tStart) {
    return (Seconds) (tFinish - tStart);
}

void Time::AddSeconds (const UTCTime& tStart, Seconds sSeconds, UTCTime* ptUtcTime) {
    *ptUtcTime = tStart + sSeconds;
}

void Time::SubtractSeconds (const UTCTime& tStart, Seconds sSeconds, UTCTime* ptUtcTime) {
    *ptUtcTime = tStart - sSeconds;
}

void Time::AtoUTCTime (const char* pszString, UTCTime* ptUtcTime) {
    *ptUtcTime = atoi (pszString);
}

char* Time::UTCTimetoA (const UTCTime& tTime, char* pszString, int iRadix) {
#ifndef _USE_32_BIT_TIME_T
    return _i64toa ((__int64) tTime, pszString, iRadix);
#else
    return _itoa ((int) tTime, pszString, iRadix);
#endif
}

bool Time::OlderThan (const UTCTime& tFirstTime, const UTCTime& tSecondTime) {
    return tFirstTime < tSecondTime;
}

bool Time::YoungerThan (const UTCTime& tFirstTime, const UTCTime& tSecondTime) {
    return tFirstTime > tSecondTime;
}

bool Time::Equals (const UTCTime& tFirstTime, const UTCTime& tSecondTime) {
    return tFirstTime == tSecondTime;
}

unsigned int Time::GetHashValue (const UTCTime& tValue, unsigned int iNumBuckets) {
    return Algorithm::GetInt64HashValue ((int64) tValue, iNumBuckets);
}
