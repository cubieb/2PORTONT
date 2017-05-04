#include <stdlib.h>
#include <stdio.h>
#include "voip_manager.h"

int main(int argc, char *argv[])
{
	TstRtpRtcpStatistics stRtpRtcpStatistics;
	
	uint32 chid  = 0;
	uint32 mid   = 0;
	uint32 reset = 0;
		
	if (argc == 4)
	{
		chid  = atoi(argv[1]);
		mid   = atoi(argv[2]);
		reset = atoi(argv[3]);
		
		rtk_GetRtpRtcpStatistics(chid , mid , reset , &stRtpRtcpStatistics);
		
		printf(
		"ch_id=%u \n"
		"m_id=%u \n"
		"bResetStatistics=%u \n"
		"nTxPkts=%lu \n"
		"nTxBytes=%lu \n"
		"nRxPkts=%lu \n"
		"nRxBytes=%lu \n"
		"nLost=%lu \n"
		"nMaxFractionLost=%lu \n"
		"nMinFractionLost=%lu \n"
		"nAvgFractionLost=%lu \n"
		"nCurFractionLost=%lu \n"
		"nDiscarded=%lu \n"
		"nTxRtcpPkts=%lu \n"
		"nRxRtcpPkts=%lu \n"
		"nTxRtcpXrPkts=%u \n"
		"nRxRtcpXrPkts=%u \n"
		"nOverRuns=%u \n"
		"nUnderRuns=%u \n"
		"nMaxJitter=%lu \n"
		"nMinJitter=%lu \n"
		"nAvgJitter=%lu \n"
		"nCurJitter=%lu \n"
		"nMaxRtcpTime=%lu \n"
		"\n"
		,stRtpRtcpStatistics.ch_id
		,stRtpRtcpStatistics.m_id
		,stRtpRtcpStatistics.bResetStatistics
		,stRtpRtcpStatistics.nTxPkts
		,stRtpRtcpStatistics.nTxBytes
		,stRtpRtcpStatistics.nRxPkts
		,stRtpRtcpStatistics.nRxBytes
		,stRtpRtcpStatistics.nLost
		,stRtpRtcpStatistics.nMaxFractionLost
		,stRtpRtcpStatistics.nMinFractionLost
		,stRtpRtcpStatistics.nAvgFractionLost
		,stRtpRtcpStatistics.nCurFractionLost
		,stRtpRtcpStatistics.nDiscarded
		,stRtpRtcpStatistics.nTxRtcpPkts
		,stRtpRtcpStatistics.nRxRtcpPkts
		,stRtpRtcpStatistics.nTxRtcpXrPkts
		,stRtpRtcpStatistics.nRxRtcpXrPkts
		,stRtpRtcpStatistics.nOverRuns
		,stRtpRtcpStatistics.nUnderRuns
		,stRtpRtcpStatistics.nMaxJitter
		,stRtpRtcpStatistics.nMinJitter
		,stRtpRtcpStatistics.nAvgJitter
		,stRtpRtcpStatistics.nCurJitter
		,stRtpRtcpStatistics.nMaxRtcpTime
		);
	}
	else
	{
		printf("Usage error!\n");
		printf("rtp_rtcp_stats chid mid reset \n");
	}
	
	return 0;
}


