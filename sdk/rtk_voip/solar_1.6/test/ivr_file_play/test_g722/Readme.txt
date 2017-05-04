Files: 
	722_raw: Recorded G.722 of the sound file
	Makefile: To make test_g722.c --> test_g722.o, 	          
		  -I../../../include/ should be set VoIP-ATA/linux-2.4.18/rtk_voip/include
	test_g722.c: Test program
	test_g722.o: Test program object file

Test program description: 
In the test code, rtk_SetIvrPlayG722() Equivalent to
rtk_IvrStartPlayG722() in VoIP-ATA/AP/rtk_voip/voip_manager/voip_manager.c 
Parameters: 
	nCount: Preparing to write the frame number, ( 10 bytes per frame )
	pData: The position of the data

Return:
	The final written frame number
	If the return value is not equal to nCount need wait a period of time ( recommendation 1 )
	, then try to write will not write the part.
	

