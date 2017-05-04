#include <stdio.h>
#include <unistd.h>
#include "voip_feature.h"
#include "voip_manager.h"

int main()
{
	rtk_CompleteDeferInitialzation();
	fprintf(stderr , "All DSP Software Ready. VoIP Defer Init done. \n");
	
	return 0;
}

