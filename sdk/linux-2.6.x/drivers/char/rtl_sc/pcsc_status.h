#ifndef __PCSCSTATUS__
#define __PCSCSTATUS__


// Normal Processing
unsigned char *normalProcess[] = {0x9000, 0x6100}; // 0x6100 maps all 0x61XX
unsigned char *normalProcessMessage[] = {"No further qualification", 
										 "SW2 encodes the number of data bytes still available"};
// Warning Processing
unsigned char *warningProcess[] = {0x6200, 0x6300}; // 0x6200 maps all 0x62xx, 0x6300 maps all 0x63xx
unsigned char *warningProcessMessage[] = {"State of non-volatile memory is unchanged", 
										  "State of non-volatile memory has unchanged"};
										  
unsigned char *warningProcessDetail[] = {0x6200, 0x6281, 0x6282, 0x6283, 0x6284, 0x6285, 0x6286, 0x6300, 0x6381, 0x63C0};
unsigned char *warningProcessDetailMessage[] = {"No information given",
												"Part of returned data may be corrupted",
												"End of file or record reached before reading Ne bytes",
												"Selected file deactivated",
												"File control information not formatted",
												"Selected file in termination state",
												"No input data available from a sensor on the card"
												"No information given",
												"File filled up by the last write",
												"Counter from 0 to 15 encode by X" //0x63CX
												};
// Execution Error
unsigned char *execError[] = {0x6400, 0x6500, 0x6600}; // 0x6400 maps all 0x64xx, 0x6500 maps all 0x65xx, 0x6600 maps all 0x66xx
unsigned char *execErrorMessage[] = {"State of non-volatile memory is unchanged", 
									"State of non-volatile memory has unchanged",
									"Security-related issues"
									};
									
unsigned char *execErrorDetail[] = {0x6400, 0x6401, 0x6500, 0x6581};
unsigned char *execErrorDetailMessage[] = {"Execution error",
											"Immediate response required by the card",
											"No information given",
											"Memory failure"
											};
// Checking Error
/* 
	0x6800 maps all 0x68xx, 0x6900 maps all 0x69xx, 0x6a00 maps all 0x6axx, 0x6c00 maps all 0x6cxx,
*/
unsigned char *checkError[] = {0x6700, 0x6800, 0x6900, 0x6a00, 0x6b00, 0x6c00, 0x6d00, 0x6e00, 0x6f00}; 
unsigned char *checkErrorMessage[] = {"Wrong length; no further indication",
										"Functions in CLA not supported",	// further qualification in SW2
										"Command not allowed",		// // further qualification in SW2
										"Wrong parameters P1-P2",	// further qualification in SW2
										"Wrong parameters P1-P2",
										"Wrong Le field",	//SW2 encodes the exact number of data bytes still available
										"Instruction code not supported or invalid",
										"class not supported",
										"No precise diagnosis"};
										
unsigned char *checkErrorDetail[] = {0x6800, 0x6881, 0x6882, 0x6883, 0x6884,
									0x6900, 0x6981, 0x6982, 0x6983, 0x6984, 0x6981,  0x6982, 0x6983, 0x6984,
									0x6A00, 0x6A80, 0x6A81, 0x6A82, 0x6A83, 0x6A84, 0x6A85, 0x6A86, 0x6A87, 0x6A88, 0x6A89, 0x6A8A};
									
unsigned char *checkErrorDetailMessage[] = {"No information given", "Logical channel not supported", "Secure messaging not supported", "Last command of the chain expected", "Command chaining not supported",
											"No information given", "Command incompatible with file structure", "Security status not satisfied", "Authentication method blocked", "Reference data not usable", "Conditions of use not satisfied", "Command not allowed(no current EF)", "Expected secure messaging data objects missing", "Incorrect secure messaging data objects",
											"No information given", "Incorrect parameters in the command data field", "Function not supported", "file or application not found", "Record not found", "Not enough memory space in the file", "Nc inconsistent with TLV structure", "Incorrect parameters P1-P2", "Nc inconsistent with parameters P1-P2", "Referenced data or reference data not found(exact meaning depending on the command)", "File already exists", "DF name already exists"
											};
#endif