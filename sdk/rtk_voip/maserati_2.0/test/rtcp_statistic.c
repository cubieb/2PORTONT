#include "voip_manager.h"

int rtcp_main(int argc, char *argv[])
{
	TstRtpRtcpStatistics stRtpRtcpStatistics;
	TstVoipSessionStatistics stVoipSessionStatistics;
	
	if (argc == 4)
	{
		rtk_GetRtpRtcpStatistics(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), &stRtpRtcpStatistics);

		printf("Rx(byte) = %u\n", stRtpRtcpStatistics.nRxBytes);
		printf("Rx(pkt) = %u\n", stRtpRtcpStatistics.nRxPkts);
		printf("Tx(byte) = %u\n", stRtpRtcpStatistics.nTxBytes);
		printf("Tx(pkt) = %u\n", stRtpRtcpStatistics.nTxPkts);
		printf("\n");

		printf("Discard pkt = %d\n", stRtpRtcpStatistics.nDiscarded);
		printf("\n");

		printf("Lost pkt = %d\n", stRtpRtcpStatistics.nLost);
		printf("Max. Fraction lost = %d\n", stRtpRtcpStatistics.nMaxFractionLost);
		printf("Min. Fraction lost = %d\n", stRtpRtcpStatistics.nMinFractionLost);
		printf("Avg. Fraction lost = %d\n", stRtpRtcpStatistics.nAvgFractionLost);
		printf("Cur. Fraction lost = %d\n", stRtpRtcpStatistics.nCurFractionLost);
		printf("\n");
				
		printf("RTCP Tx pkt = %u\n", stRtpRtcpStatistics.nTxRtcpPkts);
		printf("RTCP Rx pkt = %u\n", stRtpRtcpStatistics.nRxRtcpPkts);
		printf("RTCP-XR Tx pkt = %u\n", stRtpRtcpStatistics.nTxRtcpXrPkts);
		printf("RTCP-XR Rx pkt = %u\n", stRtpRtcpStatistics.nRxRtcpXrPkts);
		printf("\n");
		
		printf("Max. Jitter(ms) = %d\n", stRtpRtcpStatistics.nMaxJitter);
		printf("Min. Jitter(ms) = %d\n", stRtpRtcpStatistics.nMinJitter);
		printf("Avg. Jitter(ms) = %d\n", stRtpRtcpStatistics.nAvgJitter);
		printf("Cur. Jitter(ms) = %d\n", stRtpRtcpStatistics.nCurJitter);
		printf("\n");
		
		printf("Jitter overrun = %d\n", stRtpRtcpStatistics.nOverRuns);
		printf("Jitter underrun = %d\n", stRtpRtcpStatistics.nUnderRuns);
		printf("\n");
		
		printf("Max. RTCP time(ms) = %d\n", stRtpRtcpStatistics.nMaxRtcpTime);
		printf("\n\n\n");
	}
	else if (argc == 5)
	{
		rtk_Get_Session_Statistics( atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), &stVoipSessionStatistics);

		printf("Rx Silence packet = %d\n", stVoipSessionStatistics.nRxSilencePacket);
		printf("Tx Silence packet = %d\n", stVoipSessionStatistics.nTxSilencePacket);
		printf("Average Playout Delay(unit:1ms) = %d\n", stVoipSessionStatistics.nAvgPlayoutDelay);
		printf("Current Jitter Buffer packet  = %d\n", stVoipSessionStatistics.nCurrentJitterBuf);
		printf("Early packet  = %d\n", stVoipSessionStatistics.nEarlyPacket);
		printf("Late packet  = %d\n", stVoipSessionStatistics.nLatePacket);
		printf("Silence speech  = %d\n", stVoipSessionStatistics.nSilenceSpeech);
	}
	else
	{
		printf("Usage error!\n");
		printf("To get RTP/RTCP statistics: rtcp_statistic  chid  sid(0 or 1) reset(0 or 1) \n");
		printf("To get Session statistics: rtcp_statistic  chid  sid(0 or 1)  reset(0 or 1) x\n");
		printf("Note: 'x' means any input.\n");

	}
	
	return 0;
}


