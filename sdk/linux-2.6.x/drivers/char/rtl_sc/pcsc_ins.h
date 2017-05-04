#ifndef __SCINS__
#define __SCINS__


#define DEACTIVATE_FILE	0x04
#define ERASE_RECORD	0x0c
#define ERASE_BINARY_1	0x0E
#define ERASE_BINARY_2	0x0F
#define PERFORM_SCQL_OP	0x10	// Perform Structured Card Query Language Operation
#define PERFORM_TRNAS_OP	0x12	// Perform Transaction Operation
#define PERFORM_USER_OP	0x14	// Perform User Operation
#define VERIFY_1	0x20
#define VERIFY_2	0x21
#define MANAGE_SEC_ENV	0x22	// Manage security environment
#define CHANGE_REF_DATA	0x24	// Change reference data
#define DISABLE_VERIFI_REQUIRE	0x26	// Disable verification requirement
#define ENABLE_VERIFI_REQUIRE	0x28	// Enable verification requirement
#define PERFORM_SEC_OP	0x2A	// Perform security operation
#define RESET_RETRY_COUNT	0x2C	// Reset retry counter
#define ACTIVATEFILE 0x44
#define GEN_ASYMMETRIC_KEY_PAIR	0x46	// Generate asymmetric key pair
#define MANAGE_CHANNEL	0x70
#define EXTERNAL_AUTH	0x82	// External authenticate
#define GENERAL_AUTH_1	0x86	// General authenticate
#define GENERAL_AUTH_2	0x87	// General authenticate
#define INTERNAL_AUTH	0x88
#define SEARCH_BIN	0xA0	// Search binary
#define SEARCH_REC	0xA2	// Search record
#define SELECT	0xA4
#define READ_BIN_1	0xB0	// Read binary
#define READ_BIN_2	0xB1	// Read binary
#define READ_REC_1	0xB2	// Read record
#define READ_REC_2	0xB3	// Read record
#define GET_RESPONSE	0xC0
#define ENVELOPE_1	0xC2
#define ENVELOPE_2	0xC3
#define GET_DATA_1	0xCA
#define GET_DATA_2	0xCB
#define WRITE_BIN_1	0xD0	// Write binary
#define WRITE_BIN_2	0xD1	// Write binary
#define WRITE_REC	0xD2	// Write record
#define UPDATE_BIN_1	0xD6	// Update binary
#define UPDATE_BIN_2	0xD7	// Update binary
#define PUT_DATA	0xDA

#define	UPDATE_REC_1	0xDC	// Update record
#define UPDATE_REC_2	0xDD	// Update record
#define CREATE_FILE	0xE0
#define APPEND_RECORD	0xE2
#define DELETE_FILE	0xE4
#define TERMINATE_DF	0xE6
#define TERMINATE_EF	0xE8
#define TERMINATE_CARD_USAGE	0xFE

#endif
