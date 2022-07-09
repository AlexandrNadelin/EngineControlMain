#include "ModbusCommon.h"
#include "Memory.h"

extern InfoRegisters infoRegisters;
extern InputRegisters inputRegisters;
extern HoldingRegisters holdingRegisters;
extern ContactRegisters contactRegisters;
extern CoilRegisters coilRegisters;

__WEAK void modbusRequestProcessedCallback(uint8_t modbusRequestType,uint16_t regAddress,uint16_t regCount)
{
	UNUSED(modbusRequestType);
	UNUSED(regAddress);
	UNUSED(regCount);
}

uint8_t twoCharToUint8(char* chars)
{
	uint8_t number=0;
	if(chars[0]>47&&chars[0]<58)number=(chars[0]-48)<<4;
	else if(chars[0]>64&&chars[0]<71)number=(chars[0]-55)<<4;
	
	if(chars[1]>47&&chars[1]<58)number=number|(chars[1]-48);
	else if(chars[1]>64&&chars[1]<71)number=number|(chars[1]-55);
  
	return number;
}

void Uint8ToTwoChars(char* dest,uint8_t source)
{
	uint8_t utmp;
	utmp=source>>4;
	if(utmp<10)dest[0]='0'+utmp;
	else dest[0]='A'+utmp-10;
	
	utmp=source&0x0F;
	if(utmp<10)dest[1]='0'+utmp;
	else dest[1]='A'+utmp-10;
}

static const unsigned char aucCRCHi[] = {
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40
};
 
 
static const unsigned char aucCRCLo[] = {
  0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
  0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
  0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
  0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
  0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
  0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
  0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
  0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 
  0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
  0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
  0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
  0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
  0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 
  0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
  0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
  0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
  0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
  0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
  0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
  0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
  0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
  0x41, 0x81, 0x80, 0x40
};
 
uint16_t Crc16(uint8_t *pData, uint16_t length)
{
  unsigned char   ucCRCHi = 0xFF;
  unsigned char   ucCRCLo = 0xFF;
  volatile int idx;
 
  while( length-- )
  {
    idx = ucCRCLo ^ *( pData++ );
    ucCRCLo = ( uint8_t )( ucCRCHi ^ aucCRCHi[idx] );
    ucCRCHi = aucCRCLo[idx];
  }
     
  return ( unsigned int )( ucCRCHi << 8 | ucCRCLo );
}

uint8_t Lrc8(uint8_t *pData, uint16_t length)
{
  uint8_t crc = 0;
 
  volatile int i;
  for(i=0; i < length; i++)
    crc += pData[i];
 
  crc = (uint8_t)(0x100 - crc);
 
  return crc; 
}

//Переопределение
uint16_t ModbusRequestCallback(uint8_t* rxDataPtr, uint16_t rxDataLen,uint8_t* txDataPtr)
{
	ModbusFrameRTU* request = (void*)rxDataPtr;
	ModbusFrameRTU* answer = (void*)txDataPtr;
	answer->deviceAddress = request->deviceAddress;
	answer->command=request->command;
	uint16_t dataLen=0;
	
	if(request->command == READ_COIL_REGS)
	{
		if(coilRegisters.count==0)
		{
			answer->command=request->command|0x80;
			answer->data[0]=ILLEGAL_FUNCTION;
			return 3;				
		}
		else
		{
			uint16_t regAddress = (request->data[0]<<8)|request->data[1];
      uint16_t regCount = (request->data[2]<<8)|request->data[3];
				
			if(coilRegisters.count<regAddress+regCount)
			{
				answer->command=request->command|0x80;
			  answer->data[0]=ILLEGAL_DATA_ADDRESS;
			  return 3;	
			}
			if(regCount%8)
			{	
				uint8_t amountOfBytesForAnswer = regCount/8 + 1;        
        answer->data[0]=amountOfBytesForAnswer;
				dataLen = 3+amountOfBytesForAnswer;	
        volatile int i=0;				
				for(i=0;i<amountOfBytesForAnswer;i++)
				{
					answer->data[1+i]=((coilRegisters.bytes[regAddress/8+i])>>regAddress%8)|((coilRegisters.bytes[regAddress/8+i+1])<<(8-regAddress%8));
				}
				uint8_t amountOfnull = amountOfBytesForAnswer*8 - regCount;
				uint8_t mask=0xFF>> amountOfnull;
				answer->data[i]&=mask;
			}
			else
			{
				uint8_t amountOfBytesForAnswer = regCount/8;        
        answer->data[0]=amountOfBytesForAnswer;
				dataLen = 3+amountOfBytesForAnswer;	
        volatile int i=0;
        if(regAddress%8)//не ровно
				{						
				  for(i=0;i<amountOfBytesForAnswer;i++)
				  {
					  answer->data[1+i]=((coilRegisters.bytes[regAddress/8+i])>>regAddress%8)|((coilRegisters.bytes[regAddress/8+i+1])<<(8-regAddress%8));
				  }
				}
			  else
				{
					for(i=0;i<amountOfBytesForAnswer;i++)
				  {
					  answer->data[1+i]=coilRegisters.bytes[regAddress/8+i];
				  }
				}
			}
			modbusRequestProcessedCallback(READ_COIL_REGS,regAddress,regCount);
      return dataLen;			
		}	
	}
  else if(request->command == READ_CONTACT_REGS)
	{
		if(contactRegisters.count==0)
		{
			answer->command=request->command|0x80;
			answer->data[0]=ILLEGAL_FUNCTION;
			return 3;				
		}
		else
		{
			uint16_t regAddress = (request->data[0]<<8)|request->data[1];
      uint16_t regCount = (request->data[2]<<8)|request->data[3];
				
			if(contactRegisters.count<regAddress+regCount)
			{
				answer->command=request->command|0x80;
			  answer->data[0]=ILLEGAL_DATA_ADDRESS;
			  return 3;	
			}
			if(regCount%8)
			{	
				uint8_t amountOfBytesForAnswer = regCount/8 + 1;        
        answer->data[0]=amountOfBytesForAnswer;
				dataLen = 3+amountOfBytesForAnswer;	
        volatile int i=0;				
				for(i=0;i<amountOfBytesForAnswer;i++)
				{
					answer->data[1+i]=((contactRegisters.bytes[regAddress/8+i])>>regAddress%8)|((contactRegisters.bytes[regAddress/8+i+1])<<(8-regAddress%8));
				}
				uint8_t amountOfnull = amountOfBytesForAnswer*8 - regCount;
				uint8_t mask=0xFF>> amountOfnull;
				answer->data[i]&=mask;
			}
			else
			{
				uint8_t amountOfBytesForAnswer = regCount/8;        
        answer->data[0]=amountOfBytesForAnswer;
				dataLen = 3+amountOfBytesForAnswer;	
        volatile int i=0;
        if(regAddress%8)//не ровно
				{						
				  for(i=0;i<amountOfBytesForAnswer;i++)
				  {
					  answer->data[1+i]=((contactRegisters.bytes[regAddress/8+i])>>regAddress%8)|((contactRegisters.bytes[regAddress/8+i+1])<<(8-regAddress%8));
				  }
				}
				else
				{
					for(i=0;i<amountOfBytesForAnswer;i++)
				  {
					  answer->data[1+i]=contactRegisters.bytes[regAddress/8+i];
				  }
				}
			} 
			modbusRequestProcessedCallback(READ_CONTACT_REGS,regAddress,regCount);
      return dataLen;       				
		}	
	}
  else if(request->command == READ_HOLDING_REGS)
	{
		if(holdingRegisters.count==0)
		{
			answer->command=request->command|0x80;
			answer->data[0]=ILLEGAL_FUNCTION;
			return 3;
		}
		else
		{
			uint16_t regAddress = (request->data[0]<<8)|request->data[1];
      uint16_t regCount = (request->data[2]<<8)|request->data[3];
				
			if(holdingRegisters.count<regAddress+regCount)//(server->holdingRegisters->amountHoldingRegisters<addressStartReg+amountOfRegs)
			{
				answer->command=answer->command|0x80;
			  answer->data[0]=ILLEGAL_DATA_ADDRESS;
			  return 3;	
			}
      else
			{
				uint8_t amountOfBytesForAnswer = regCount*2;        
        answer->data[0]=amountOfBytesForAnswer;
				dataLen = 3+amountOfBytesForAnswer;	
        volatile int i=0;
					
				for(i=0;i<regCount;i++)
				{					
					answer->data[1+i*2]=holdingRegisters.registers[regAddress+i]&0xFF;
				  answer->data[2+i*2]=holdingRegisters.registers[regAddress+i]>>8;
				}
			}	
			modbusRequestProcessedCallback(READ_HOLDING_REGS,regAddress,regCount);
      return dataLen;
		}			
	}	
  else if(request->command == READ_INPUT_REGS)
	{
    if(inputRegisters.count==0)
		{
			answer->command=request->command|0x80;
			answer->data[0]=ILLEGAL_FUNCTION;
			return 3;
		}
		else
		{
			uint16_t regAddress = (request->data[0]<<8)|request->data[1];
      uint16_t regCount = (request->data[2]<<8)|request->data[3];
				
			if(inputRegisters.count<regAddress+regCount)
			{
				answer->command=request->command|0x80;
			  answer->data[0]=ILLEGAL_DATA_ADDRESS;
			  return 3;	
			}
      else
			{
				uint8_t amountOfBytesForAnswer = regCount*2;        
        answer->data[0]=amountOfBytesForAnswer;
				dataLen = 3+amountOfBytesForAnswer;	
        volatile int i=0;
        for(i=0;i<regCount;i++)
				{
					answer->data[1+i*2]=inputRegisters.registers[regAddress+i]&0xFF;
				  answer->data[2+i*2]=inputRegisters.registers[regAddress+i]>>8;
			  }
			}	
			modbusRequestProcessedCallback(READ_INPUT_REGS,regAddress,regCount);
      return dataLen;			
		}	
	}
  else if(request->command == WRITE_COIL_REGISTER)
	{
		if(coilRegisters.count==0)
		{
			answer->command=request->command|0x80;
			answer->data[0]=ILLEGAL_FUNCTION;
			return 3;
		}
		else
		{
			uint16_t regAddress = (request->data[0]<<8)|request->data[1];
				
			if(coilRegisters.count<=regAddress)
			{
				answer->command=request->command|0x80;
			  answer->data[0]=ILLEGAL_DATA_ADDRESS;
			  return 3;	
			}
				
			uint16_t writeValue = (request->data[2]<<8)|request->data[3];
				
			uint8_t registerByte = regAddress/8;
			uint8_t registerShift = regAddress%8;
			uint8_t mask = 1<<registerShift;
				
      if(writeValue==0xFF00)coilRegisters.bytes[registerByte]=coilRegisters.bytes[registerByte]|mask;
      else coilRegisters.bytes[registerByte]=coilRegisters.bytes[registerByte]&~(uint8_t)mask;
			
			modbusRequestProcessedCallback(WRITE_COIL_REGISTER,regAddress,1);
			return 6;
		}
	}			
  else if(request->command == WRITE_COIL_REGISTERS)
	{
		if(coilRegisters.count==0)
		{
			answer->command=request->command|0x80;
			answer->data[0]=ILLEGAL_FUNCTION;
			return 3;
		}
		else
		{
			uint16_t regAddress = (request->data[0]<<8)|request->data[1];
      uint16_t regCount = (request->data[2]<<8)|request->data[3];
				
			if(coilRegisters.count<regAddress+regCount)
			{
				answer->command=request->command|0x80;
			  answer->data[0]=ILLEGAL_DATA_ADDRESS;
			  return 3;	
			}
			//uint8_t amountOfBytesForWrite=frameRTU->data[4];
			volatile int i=0;
			uint8_t registerByte;
			uint8_t registerShift;
			uint8_t mask;
			for(i=0;i<regCount;i++)
			{
				registerByte = (regAddress+i)/8;
				registerShift = (regAddress+i)%8;
				mask = 1<<registerShift;
				if(request->data[5+(i/8)]&(1<<(i%8)))coilRegisters.bytes[registerByte]=coilRegisters.bytes[registerByte]|mask;
        else coilRegisters.bytes[registerByte]=coilRegisters.bytes[registerByte]&~(uint8_t)mask;
			}
			modbusRequestProcessedCallback(WRITE_COIL_REGISTERS,regAddress,regCount);
			return 6;
		}
	}		
  else if(request->command == WRITE_HOLDING_REGISTERS)
	{		
		if(holdingRegisters.count==0)
		{
			answer->command=request->command|0x80;
			answer->data[0]=ILLEGAL_FUNCTION;
			return 3;
		}
		else
		{
			uint16_t regAddress = (request->data[0]<<8)|request->data[1];
      uint16_t regCount = (request->data[2]<<8)|request->data[3];
				
      if(holdingRegisters.count < regAddress+regCount)//(server->holdingRegisters->amountHoldingRegisters<addressStartReg+amountOfRegs)
			{
				answer->command=request->command|0x80;
			  answer->data[0]=ILLEGAL_DATA_ADDRESS;
			  return 3;	
			}
				
			volatile int i;
				
			uint16_t registers[regCount];
					
			for(i=0;i<regCount;i++)
			{					
				registers[i]=request->data[5+i*2]|request->data[6+i*2]<<8;
			}
					
			memoryWriteHoldingRegisters(registers,regAddress,regCount);
			modbusRequestProcessedCallback(WRITE_HOLDING_REGISTERS,regAddress,regCount);
			return 6;
		}	
	}
	else if(request->command == READ_INFO_ABOUT_DEVICE)
	{
		if(infoRegisters.len==0)
		{
			answer->command=request->command|0x80;
			answer->data[0]=ILLEGAL_FUNCTION;
			return 3;
		}
		else
		{
			answer->data[0]=infoRegisters.len;
			volatile int i;
			for(i=0;i<infoRegisters.len;i++)
			{
				answer->data[1+i]=infoRegisters.data[i];
			}
			return 3+infoRegisters.len;
		}
	}
	else
	{
		answer->command=request->command|0x80;
		answer->data[0]=ILLEGAL_FUNCTION;
		return 3;
	}
}


