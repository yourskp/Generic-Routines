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
#include <project.h>
#include <SW_Framer.h>
#include <CLI.h>
#include <stdio.h>

uint16 dataLength;
uint8 hexCharCount;
uint8 dataByte;
uint8 ackBuffer[5];
char cliBuffer[64];

void (*parse_next)(void) = NULL;

void User_Write_Message(void);
void Wait_For_Write(void);
void Error_Message(void);
void Receive_Data(void);
void ClearCLIDataStructures(uint8 clearLength);

void Init_CLI(void)
{
	CLI_UART_Start(); /* Start the UART interface */
	parse_next = User_Write_Message;
	ClearCLIDataStructures(CLEAR_RECEIVED_LENGTH);
}

void Parse_CLI(void) 
{
    if (parse_next != NULL)
	{ 	
        parse_next(); /* Call the state function */
    }
}

void User_Write_Message(void)
{
	CLI_UART_PutString("Enter a packet for wireless transmission\r\nPacket format = [W][data][return]/[Q-Abort]\r\n");
	parse_next = Wait_For_Write;
}	

void Wait_For_Write(void)
{
	if(CLI_UART_GetRxBufferSize() > 0)
	{
		uint8 asciiChar;
		asciiChar = CLI_UART_GetByte();
		if(asciiChar == 'W' || asciiChar == 'w')
		{
			parse_next = Receive_Data;
		}
		else if(asciiChar == ' ')
		{
			parse_next = Wait_For_Write; /* Ignore any leading spaces */		
		}
		else
		{
			parse_next = Error_Message;
		}
	}
}

void Error_Message(void)
{
	CLI_UART_PutString("\r\nInvalid packet format\r\nPacket format = [W][data][return]/[Q-Abort]\r\n");
	CLI_UART_ClearRxBuffer();
	ClearCLIDataStructures(DONT_CLEAR_RECEIVED_LENGTH);
	parse_next = Wait_For_Write;
}

void Receive_Data(void)
{
	uint8 asciiChar;
	if(CLI_UART_GetRxBufferSize() > 0)
	{
		asciiChar = CLI_UART_GetByte();
		
		if(asciiChar == ' ')
		{
			if(hexCharCount != 0) /* Atleast once character was received for this byte parsing */
			{
				UpdateTxBuffer(dataLength, dataByte);
				dataLength++;
				ClearCLIDataStructures(DONT_CLEAR_RECEIVED_LENGTH);
			}
		}
		else if (IsValidHexCharacter(asciiChar))
		{
			dataByte = (dataByte << (hexCharCount * 4)) | HexToNibble(asciiChar);
			hexCharCount++;
			if(hexCharCount == HEX_CHARACTERS_TO_BYTE)
			{
				UpdateTxBuffer(dataLength, dataByte);
				dataLength++;
				ClearCLIDataStructures(DONT_CLEAR_RECEIVED_LENGTH);
			}
		}
		else if(asciiChar == 0x0D) /* If return key is hit, fill Tx buffer with zeros before transmitting */
		{
			if(hexCharCount !=0)
			{
				UpdateTxBuffer(dataLength, dataByte);
				dataLength++;
			}
			
			for(;dataLength<MAX_PAYLOAD_LENGTH-1;dataLength++)
			{
				UpdateTxBuffer(dataLength, DEFAULT_DATA_BYTE);
			}
		}
		else if(asciiChar == 'q' || asciiChar == 'Q')
		{
			ClearCLIDataStructures(CLEAR_RECEIVED_LENGTH);
			CLI_UART_ClearRxBuffer();
			CLI_UART_PutString("\r\nPacket TX Aborted\r\n");
			parse_next = User_Write_Message;
		}
		else
		{
			parse_next = Error_Message;
		}
		
		if(dataLength == MAX_PAYLOAD_LENGTH-1) /* First byte is the data toggle byte that is not updated by CLI */
		{
			CLI_UART_PutString("\r\nPacket TX in progress\r\n");
			Radio_Blocking_Transfer(ackBuffer);
			ClearCLIDataStructures(CLEAR_RECEIVED_LENGTH);
			sprintf(cliBuffer, "Received ACK packet data = %x %x %x %x\r\n", ackBuffer[1], ackBuffer[2], ackBuffer[3], ackBuffer[4]);
			CLI_UART_PutString((const char*)cliBuffer);
			parse_next = User_Write_Message;
		}
	}
}

cystatus IsValidHexCharacter(uint8 asciiCharacter)
{
	if((asciiCharacter >= '0' && asciiCharacter <= '9') ||
		(asciiCharacter >= 'a' && asciiCharacter <= 'f') ||
		(asciiCharacter >= 'A' && asciiCharacter <= 'F'))
	{
		return 1;	
	}
	else
	{
		return 0;	
	}
}

uint8 HexToNibble(uint8 asciiCharacter)
{
	if((asciiCharacter >= '0' && asciiCharacter <= '9'))
	{
		return 	(asciiCharacter - '0');
	}
	else if (asciiCharacter >= 'a' && asciiCharacter <= 'f')
	{
		return (asciiCharacter - 'a' + 10);
	}	
	else if (asciiCharacter >= 'A' && asciiCharacter <= 'F')
	{
		return (asciiCharacter - 'A' + 10);
	}
	else
	{
		return 0;	
	}
}

void ClearCLIDataStructures(uint8 clearLength)
{
	hexCharCount = 0;
	dataByte = 0;
	if(clearLength)
	{
		dataLength = 0;
	}
}

/* [] END OF FILE */
