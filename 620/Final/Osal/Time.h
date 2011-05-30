// Time.h: interface for the Time class.
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#if !defined(AFX_TIME_H__CB52E515_F346_11D1_9DAF_0060083E8062__INCLUDED_)
#define AFX_TIME_H__CB52E515_F346_11D1_9DAF_0060083E8062__INCLUDED_

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#include "OS.h"

typedef time_t UTCTime;
typedef _timeb Timer;

#define NULL_TIME ((time_t) 0)

class String;

enum DayOfWeek {
    Sunday,
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    DayOfWeekError
};

namespace Time {

    OSAL_EXPORT void ZeroTime (UTCTime* ptUTCTime);

    OSAL_EXPORT void GetTime (UTCTime* ptUTCTime);
    OSAL_EXPORT void GetTime (int iSec, int iMin, int iHour, int iDay, int iMonth, int iYear, UTCTime* ptUTCTime);

    OSAL_EXPORT int GetTimeZone (char pszTimeZone[OS::MaxTimeZoneLength], int* piBias);
    OSAL_EXPORT int GetTimeZoneBias (int* piBias);

    OSAL_EXPORT void GetDate (
        int* piSec, int* piMin, int* piHour, DayOfWeek* pdayOfWeek, int* piDay, int* piMonth, int* piYear);
    OSAL_EXPORT void GetDate (
        const UTCTime& tTime,
        int* piSec, int* piMin, int* piHour, DayOfWeek* pdayOfWeek, int* piDay, int* piMonth, int* piYear);

    OSAL_EXPORT int GetTimeString (char pszTimeString[OS::MaxTimeLength]);
    OSAL_EXPORT int GetTimeString (const UTCTime& tTime, char pszTimeString[OS::MaxTimeLength]);

    OSAL_EXPORT int GetDateString (char pszDateString[OS::MaxDateLength]);
    OSAL_EXPORT int GetDateString (const UTCTime& tTime, char pszDateString[OS::MaxDateLength]);

    OSAL_EXPORT int GetGMTDateString (char pszGMTDateString[OS::MaxGMTDateLength]);
    OSAL_EXPORT int GetGMTDateString (const UTCTime& tTime, char pszGMTDateString[OS::MaxGMTDateLength]);

    OSAL_EXPORT int GetCookieDateString (char pszCookieDateString[OS::MaxCookieDateLength]);
    OSAL_EXPORT int GetCookieDateString (const UTCTime& tTime, char pszCookieDateString[OS::MaxCookieDateLength]);

    OSAL_EXPORT int GetDay();
    OSAL_EXPORT int GetDay (const UTCTime& tTime);

    OSAL_EXPORT int GetMonth();
    OSAL_EXPORT int GetMonth (const UTCTime& tTime);

    OSAL_EXPORT int GetYear();
    OSAL_EXPORT int GetYear (const UTCTime& tTime);

    OSAL_EXPORT DayOfWeek GetDayOfWeek();
    OSAL_EXPORT DayOfWeek GetDayOfWeek (const UTCTime& tTime);

    OSAL_EXPORT const char* GetDayOfWeekName (DayOfWeek dayOfWeek);
    OSAL_EXPORT const char* GetAbbreviatedDayOfWeekName (DayOfWeek dayOfWeek);

    OSAL_EXPORT const char* GetMonthName();
    OSAL_EXPORT const char* GetMonthName (int iMonth);
    OSAL_EXPORT const char* GetMonthName (const UTCTime& tTime);

    OSAL_EXPORT const char* GetAbbreviatedMonthName();
    OSAL_EXPORT const char* GetAbbreviatedMonthName (int iMonth);
    OSAL_EXPORT const char* GetAbbreviatedMonthName (const UTCTime& tTime);

    OSAL_EXPORT bool IsWeekendTime();
    OSAL_EXPORT bool IsWeekendTime (const UTCTime& tTime);
    
    OSAL_EXPORT Seconds GetRemainingWeekendSeconds();
    OSAL_EXPORT Seconds GetRemainingWeekendSeconds (const UTCTime& tTime);

    OSAL_EXPORT void StartTimer (Timer* ptmTimer);
    OSAL_EXPORT MilliSeconds GetTimerCount (const Timer& tmTimer);

    OSAL_EXPORT Seconds GetSecondDifference (const UTCTime& tFinish, const UTCTime& tStart);
    OSAL_EXPORT void AddSeconds (const UTCTime& tStart, Seconds sSeconds, UTCTime* ptUtcTime);
    OSAL_EXPORT void SubtractSeconds (const UTCTime& tStart, Seconds sSeconds, UTCTime* ptUtcTime);

    OSAL_EXPORT void AtoUTCTime (const char* pszString, UTCTime* ptUtcTime);
    OSAL_EXPORT char* UTCTimetoA (const UTCTime& t64Time, char* pszString, int iRadix);

    OSAL_EXPORT bool OlderThan (const UTCTime& tFirstTime, const UTCTime& tSecondTime);
    OSAL_EXPORT bool YoungerThan (const UTCTime& tFirstTime, const UTCTime& tSecondTime);
    OSAL_EXPORT bool Equals (const UTCTime& tFirstTime, const UTCTime& tSecondTime);

    OSAL_EXPORT unsigned int GetHashValue (const UTCTime& tValue, unsigned int iNumBuckets);
};
/*
OSAL_EXPORT bool operator< (const UTCTime& tLhs, const UTCTime& tRhs);
OSAL_EXPORT bool operator<= (const UTCTime& tLhs, const UTCTime& tRhs);
OSAL_EXPORT bool operator> (const UTCTime& tLhs, const UTCTime& tRhs);
OSAL_EXPORT bool operator>= (const UTCTime& tLhs, const UTCTime& tRhs);
OSAL_EXPORT bool operator== (const UTCTime& tLhs, const UTCTime& tRhs);
*/

#endif // !defined(AFX_TIME_H__CB52E515_F346_11D1_9DAF_0060083E8062__INCLUDED_)