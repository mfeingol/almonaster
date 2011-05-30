// Time.cpp: implementation of the Time namespace
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
	"Sat"
};

static const char* pszDayOfWeek[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
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

int Time::GetTimeZone (char pszTimeZone[OS::MaxTimeZoneLength], int* piBias) {

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
}

int Time::GetTimeZoneBias (int* piBias) {

	TIME_ZONE_INFORMATION tzInfo;
	DWORD dwRetVal = ::GetTimeZoneInformation (&tzInfo);

	if (dwRetVal == TIME_ZONE_ID_INVALID) {
		return ERROR_FAILURE;
	}

	*piBias = - (tzInfo.Bias + tzInfo.DaylightBias);

	return OK;
}

int Time::GetDate (int* piSec, int* piMin, int* piHour, int* piDay, int* piMonth, int* piYear) {

	return GetDate (time (NULL), piSec, piMin, piHour, piDay, piMonth, piYear);
}

int Time::GetDate (const UTCTime& tTime, int* piSec, int* piMin, int* piHour, int* piDay, int* piMonth, 
				   int* piYear) {

	tm* ptmTime = localtime ((time_t*) &tTime);
	if (ptmTime == NULL) {
		return ERROR_INVALID_ARGUMENT;
	}

	*piSec = ptmTime->tm_sec;
	*piMin = ptmTime->tm_min;
	*piHour = ptmTime->tm_hour;

	*piDay = ptmTime->tm_mday;
	*piMonth = ptmTime->tm_mon + 1;
	*piYear = ptmTime->tm_year + 1900;

	return OK;
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

	tm* ptmTime = localtime (&tTime);
	if (ptmTime == NULL || ptmTime->tm_wday > 6 || ptmTime->tm_mon > 11) {
		return ERROR_INVALID_ARGUMENT;
	}

	char pszHour[8], pszMin[8], pszSec[8], pszDayM[8];
	
	String::ItoA (ptmTime->tm_hour, pszHour, 10, 2);
	String::ItoA (ptmTime->tm_min, pszMin, 10, 2);
	String::ItoA (ptmTime->tm_sec, pszSec, 10, 2);
	String::ItoA (ptmTime->tm_mday, pszDayM, 10, 2);

	sprintf (
		pszDateString,
		"%s:%s:%s %s, %s %s %i",
		pszHour,
		pszMin,
		pszSec,
		pszDay [ptmTime->tm_wday],
		pszDayM,
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

	tm* ptmTime = gmtime (&tTime);
	if (ptmTime == NULL || ptmTime->tm_wday > 6 || ptmTime->tm_mon > 11) {
		return ERROR_INVALID_ARGUMENT;
	}

	char pszHour[8], pszMin[8], pszSec[8], pszDayM[8];
	
	String::ItoA (ptmTime->tm_hour, pszHour, 10, 2);
	String::ItoA (ptmTime->tm_min, pszMin, 10, 2);
	String::ItoA (ptmTime->tm_sec, pszSec, 10, 2);
	String::ItoA (ptmTime->tm_mday, pszDayM, 10, 2);
	
	sprintf (
		pszGMTDateString,
		"%s:%s:%s %s, %s %s %i GMT",
		pszHour,
		pszMin,
		pszSec,
		pszDay [ptmTime->tm_wday],
		pszDayM,
		pszMonth[ptmTime->tm_mon],
		ptmTime->tm_year + 1900
		);

	return OK;
}

int Time::GetCookieDateString (char pszCookieDateString[OS::MaxCookieDateLength]) {

	UTCTime tTime;
	GetTime (&tTime);

	return GetCookieDateString (tTime, pszCookieDateString);
}

int Time::GetCookieDateString (const UTCTime& tTime, char pszCookieDateString[OS::MaxCookieDateLength]) {

	tm* ptmTime = gmtime (&tTime);
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

int Time::GetDay() {

	UTCTime tTime;
	GetTime (&tTime);

	return GetDay (tTime);
}

int Time::GetDay (const UTCTime& tTime) {

	tm* ptmTime = localtime ((time_t*) &tTime);
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

const char* Time::GetDayOfWeek() {

	UTCTime tTime;
	GetTime (&tTime);

	return GetDayOfWeek (tTime);
}


const char* Time::GetDayOfWeek (const UTCTime& tTime) {

	tm* ptmTime = localtime (&tTime);
	if (ptmTime == NULL || ptmTime->tm_wday > 6 || ptmTime->tm_wday < 0) {
		return "Invalid day of week";
	}

	return pszDayOfWeek [ptmTime->tm_wday];
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

	tm* ptmTime = localtime (&tTime);
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

	tm* ptmTime = localtime (&tTime);
	if (ptmTime == NULL) {
		return "ERR";
	}

	return GetAbbreviatedMonthName (ptmTime->tm_mon + 1);
}


bool Time::IsWeekendTime() {

	UTCTime tTime;
	GetTime (&tTime);
	return IsWeekendTime (tTime);
}

bool Time::IsWeekendTime (const UTCTime& tTime) {

	tm* ptmTime = localtime (&tTime);
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

	tm* ptmTime = localtime (&tTime);
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

void Time::StartTimer (Timer* ptmTimer) {
	_ftime (ptmTimer);
}

MilliSeconds Time::GetTimerCount (const Timer& tmTimer) {

	_timeb tbNow;
	_ftime (&tbNow);

	return (tbNow.time - tmTimer.time) * 1000 + tbNow.millitm - tmTimer.millitm;
}

Seconds Time::GetSecondDifference (const UTCTime& tFinish, const UTCTime& tStart) {
	return tFinish - tStart;
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
	return itoa (tTime, pszString, iRadix);
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

	// Not 64 bit clean
	return Algorithm::GetInt64HashValue ((int) tValue, iNumBuckets);
}
