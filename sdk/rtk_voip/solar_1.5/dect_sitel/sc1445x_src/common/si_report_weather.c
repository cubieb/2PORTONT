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
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		si_report_weather.c
 * Purpose:		 		 
 * Created:		 	Oct 2009
 * By:		 		 	YP
 * Country:		 	Greece
 *
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <si_print_api.h>
#include "si_report_weather.h"
   
int rssFeed_get_weather_report(void *pRssData, WeatherReport *pReport)
{
	ReportNode* pData=(ReportNode*)pRssData;

	if (pData==NULL || pData->Next==NULL) return -1;
  
 	copyitem(pReport->title,sizeof(pReport->title) ,weather_gettitle(pData));
  copyitem(pReport->city,sizeof(pReport->city) ,weather_getcity(pData));
 	copyitem(pReport->country,sizeof(pReport->country) ,weather_getcountry(pData));
	copyitem(pReport->Humidity,sizeof(pReport->Humidity) ,weather_gethumidity(pData));
	copyitem(pReport->Pressure,sizeof(pReport->Pressure) ,weather_getpressure(pData));
	copyitem(pReport->Sunrise,sizeof(pReport->Sunrise) ,weather_getsunrise(pData));
	copyitem(pReport->Sunset,sizeof(pReport->Sunset) ,weather_getsunset(pData));
	copyitem(pReport->temperature,sizeof(pReport->temperature) ,weather_gettemperature(pData));
	copyitem(pReport->Visibility,sizeof(pReport->Visibility) ,weather_getvisibility(pData));
	copyitem(pReport->update,sizeof(pReport->update) ,weather_getupdate(pData));
	copyitem(pReport->description,sizeof(pReport->description) ,weather_getdescription(pData));
 	copyitem(pReport->code,sizeof(pReport->code) ,weather_getcode(pData));

	si_print(PRINT_LEVEL_DEBUG, "Title : %s  \n",pReport->title );
	si_print(PRINT_LEVEL_DEBUG, "City: %s  \n",pReport->city );
	si_print(PRINT_LEVEL_DEBUG, "Country: %s  \n",pReport->country  );
	si_print(PRINT_LEVEL_DEBUG, "Hum: %s  \n",pReport->Humidity  );
	si_print(PRINT_LEVEL_DEBUG, "Pres: %s  \n",pReport->Pressure  );
	si_print(PRINT_LEVEL_DEBUG, "Sun:%s  \n",pReport->Sunrise  );
	si_print(PRINT_LEVEL_DEBUG, "Set: %s  \n",pReport->Sunset  );
	si_print(PRINT_LEVEL_DEBUG, "Temp: %s  \n",pReport->temperature  );
	si_print(PRINT_LEVEL_DEBUG, "Vis: %s  \n",pReport->Visibility  );
	si_print(PRINT_LEVEL_DEBUG, "Up: %s  \n",pReport->update  );
	si_print(PRINT_LEVEL_DEBUG, "Des:%s  \n",pReport->description  );
	si_print(PRINT_LEVEL_DEBUG, "Code:%s  \n",pReport->code  );
  
	return 0;
}

char *weather_gettitle(ReportNode* pData)
{
	return weather_find("Overall Title",pData);	
}
char *weather_getcity(ReportNode* pData)
{
	return weather_find("City",pData);	
}
char *weather_getcountry(ReportNode* pData)
{
	return weather_find("Country",pData);	
}
char *weather_gethumidity(ReportNode* pData)
{
	return weather_find("Humidity",pData);	
}
char *weather_getpressure(ReportNode* pData)
{
	return weather_find("Pressure",pData);	
}
char *weather_getsunrise(ReportNode* pData)
{
	return weather_find("Sunrise",pData);	
}
char *weather_getsunset(ReportNode* pData)
{
	return weather_find("Sunset",pData);	
}
char *weather_getvisibility(ReportNode* pData)
{
	return weather_find("Visibility",pData);	
}
char *weather_getupdate(ReportNode* pData)
{
	return weather_find("Last Build",pData);	
}
char *weather_getdescription(ReportNode* pData)
{
	return weather_find("CurrentWeatherDescription",pData);	
} 
char *weather_gettemperature(ReportNode* pData)
{
	return weather_find("CurrentTemperature",pData);	
}
char *weather_getcode(ReportNode* pData)
{
	return weather_find("CurrentCode",pData);	
}

char *weather_find(char *item, ReportNode* pReport)
{
	ReportNode* pData = pReport;

 	if (pData==NULL) return NULL;

	if (pData->Next)
		pData = pData->Next;
   
	while(pData)
	{
  		if(!strcmp(item, (pData->Name)))
		{
  			return (char *)pData->Value;
		}
		pData = pData->Next;
	}
	return NULL;
}
void copyitem(char *item, int max, char* data) 
{
	int len = strlen(data);
	if (data==NULL) {si_print(PRINT_LEVEL_ERR, "Error on copyitem %s\n ", item); return ;}
	if (len> max) len=max;
 
	strncpy(item, data, len);

} 