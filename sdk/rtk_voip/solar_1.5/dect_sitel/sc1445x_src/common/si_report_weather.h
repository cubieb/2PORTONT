/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:			si_report_weather.h
 * Purpose:		
 * Created:		Oct 2009
 * By:				YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_REPORT_WEATHER_H
#define SI_REPORT_WEATHER_H
 
typedef struct _WeatherReportForecast
{
	char day[12];
	char date[12];
	char low[4];
	char high[4];
	char description[32];
}WeatherReportForecast;

typedef struct _WeatherReport
{
	char title[64];
	char update[64];
	char city[16];
	char country[16];
	char Humidity[8];
	char Visibility[8];
	char Pressure[8];
	char Sunrise[8];
	char Sunset[8];
	char description[32];
	char temperature[8];
	char code[8];
	WeatherReportForecast forecast[4];
}WeatherReport; 

typedef struct _ReportNode
{
    char *Name;
    char *Value;
    long int ParentUI;
    struct ReportNode  *Prev;
    struct ReportNode  *Next;
}ReportNode;
 
char *weather_gettitle(ReportNode* pData);
char *weather_getcity(ReportNode* pData);
char *weather_getcountry(ReportNode* pData);
char *weather_gethumidity(ReportNode* pData);
char *weather_getpressure(ReportNode* pData);
char *weather_getsunrise(ReportNode* pData);
char *weather_getsunset(ReportNode* pData);
char *weather_gettemperature(ReportNode* pData);
char *weather_getvisibility(ReportNode* pData);
char *weather_getupdate(ReportNode* pData);
char *weather_getdescription(ReportNode* pData);
char *weather_getcode(ReportNode* pData);
char *weather_find(char *item,ReportNode* pData);
int rssFeed_get_weather_report(void *pRssData, WeatherReport *pReport);
 void copyitem(char *item, int max, char* data); 

#endif //SI_REPORT_WEATHER_H
