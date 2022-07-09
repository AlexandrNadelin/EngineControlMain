from asyncio.windows_events import NULL
import sys  # We need sys so that we can pass argv to QApplication
import os
import os.path

def appentHexFile(srcFilePath: str,destFilePath: str, mainProgramAddressStr: str, mainProgramSizeStr: str):
    UP_ADDRESS = 0x04
    DATA = 0x00
    END_OF_FILE = 0x01
    PROPERTIES = 0x05 #can be ignored
    
    srcFile = NULL
    destFile = NULL
    mainProgramAddress = 0
    mainProgramSize = 0

    try:
        mainProgramAddress = int(mainProgramAddressStr,16)
        mainProgramSize = int(mainProgramSizeStr,16)
    
        srcFile = open(srcFilePath,'r')
        destFile = open(destFilePath,'w')
    except Exception:
        print(Exception)    
        if srcFile!=NULL:
            srcFile.close()
        if destFile!=NULL:
            destFile.close()
        return
    
    upAddress: int = 0x0000
    lowAddress: int = 0x0000
    fullAddress: int = 0x00000000
    numberOfDataBytes: int = 0x00
    crc: int = 0xFFFF
    dataUnitTmp: int = 0x00
        
    line = srcFile.readline()
    while line:
        lineTmp = line[1:].strip()
        bytes = bytearray.fromhex(lineTmp)
        if bytes[3] == UP_ADDRESS:
            destFile.write(line) 
            upAddress = bytes[4]<<8|bytes[5]
        elif bytes[3] == DATA:
            destFile.write(line) 
            lowAddress = bytes[1]<<8|bytes[2]
            fullAddress = upAddress<<16|lowAddress
            numberOfDataBytes = bytes[0]

            for j in range(numberOfDataBytes):
                crc^=bytes[4+j]
                for i in range(8):
                    if ((crc & 1) != 0):
                        crc >>= 1
                        crc ^=0xA001
                    else:
                        crc >>= 1

            lowAddress += numberOfDataBytes
            fullAddress += numberOfDataBytes
        elif bytes[3] == PROPERTIES: #calculate and add checksum
            numberOfFFFFDataUnits: int = mainProgramAddress + mainProgramSize - fullAddress  - 2
            for j in range(numberOfFFFFDataUnits):
                crc^=0xFF
                for i in range(8):
                    if ((crc & 1) != 0):
                        crc >>= 1
                        crc ^=0xA001
                    else:
                        crc >>= 1

            crcInMemory = ((crc&0xFF00) >> 8 | (crc&0x00FF) <<8) # в памяти в порядке LittleEndian
            upAddressCRC = (mainProgramAddress+mainProgramSize-2)>>16
            lineTmp = '02000004'+format(upAddressCRC,'04X')
            bytes = bytearray.fromhex(lineTmp)
            checkSumOfLine = 0
            for b in bytes:
                checkSumOfLine+=b

            lineTmp = ':' + lineTmp + format(0xFF-(checkSumOfLine&0xFF) + 1,'02X') + '\n'
            destFile.write(lineTmp)

            lowAddressCRC = ((mainProgramAddress + mainProgramSize - 2) & 0xFFFF)
            lineTmp = '02'+format(lowAddressCRC,'04X')+'00'+format(crcInMemory,'04X')
            bytes = bytearray.fromhex(lineTmp)
            checkSumOfLine = 0
            for b in bytes:
                checkSumOfLine+=b
            lineTmp = ':' + lineTmp + format(0xFF-(checkSumOfLine&0xFF) + 1,'02X') + '\n'
            destFile.write(lineTmp)

            destFile.write(line) 

        line = srcFile.readline()

    srcFile.close()
    destFile.close()


