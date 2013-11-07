/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef __CLI_H__
#define __CLI_H__
	
#include <cytypes.h>
#include <project.h>	
	
#define HEX_CHARACTERS_TO_BYTE    	2	
#define DEFAULT_DATA_BYTE         	0x00	
	
#define CLEAR_RECEIVED_LENGTH     	1
#define DONT_CLEAR_RECEIVED_LENGTH  0

typedef enum
{
	CLI_IDLE_STATE,
	CLI_WRITE_COMMAND_STATE,
	CLI_WRITE_DATA_STATE,
	CLI_PACKET_RECEIVED_STATE
} CLIStates;

void Init_CLI(void);
void Parse_CLI(void);

cystatus IsValidHexCharacter(uint8 asciiCharacter);	
uint8 HexToNibble(uint8 asciiCharacter);

#endif	

/* [] END OF FILE */
