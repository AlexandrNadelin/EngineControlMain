#ifndef __MODBUS_COMMON_H
#define __MODBUS_COMMON_H
#include "main.h"

//-----Mdbus Functions------//
#define BROADCAST_REQUEST        (uint8_t)0x00
#define READ_COIL_REGS           (uint8_t)0x01
#define READ_CONTACT_REGS        (uint8_t)0x02
#define READ_HOLDING_REGS        (uint8_t)0x03
#define READ_INPUT_REGS          (uint8_t)0x04
#define WRITE_COIL_REGISTER      (uint8_t)0x05
#define WRITE_HOLDING_REGISTER   (uint8_t)0x06
#define READ_STATUS              (uint8_t)0x07//прочитать статус прибора (8 бит)(Serial Line only)
#define TEST_FUNCTION            (uint8_t)0x08//Тестовая функция(Serial Line only)
#define WRITE_COIL_REGISTERS     (uint8_t)0x0F
#define WRITE_HOLDING_REGISTERS  (uint8_t)0x10
#define READ_INFO_ABOUT_DEVICE   (uint8_t)0x11//(Serial Line only)
#define READ_FILE_RECORD         (uint8_t)0x14//(Serial Line only)
#define WRITE_FILE_RECORD        (uint8_t)0x15
#define READ_FIFO                (uint8_t)0x18//чтение очереди данных

#define WRITE_COEFICIENTS        (uint8_t)0x65//пользовательская функция

//-----End Mdbus Functions------//

//-----Exception code------------//
#define ILLEGAL_FUNCTION          (uint8_t)0x01//Недопустимый код функции. Этот код исключения может возвращаться, если устройство не реализует поддержку некоторых функций Modbus протокола.
#define ILLEGAL_DATA_ADDRESS      (uint8_t)0x02//Недопустимый адрес регистра. Этот код исключения может возвращаться, если запрос указывает для чтения несуществующие регистры.
#define ILLEGAL_DATA_VALUE        (uint8_t)0x03//Недопустимое значение данных. Этот код исключения может возвращаться, если запрос содержит недопустимое значение для записи в регистр.
#define SLAVE_DEVICE_FAILURE      (uint8_t)0x04//Во время выполнения запрошенного действия произошла ошибка и действие не может быть завершено.
#define ACKNOWLAGE                (uint8_t)0x05//Специальный ответ, который используется совместно с командами программирования. Означает, что ведомое устройство приняло запрос и обрабатывает его, но данная операция потребует продолжительное время для завершения. Такой ответ возвращается, что бы предотвратить таймаут на мастере.
#define SLAVE_DEVICE_BUSY         (uint8_t)0x06//Специальный ответ, который используется совместно с командами программирования. Означает, что устройство занято обработкой длинного запроса.
#define NEGATIVE_ACKNOWLAGE       (uint8_t)0x07//Специальный ответ, который используется совместно с командами программирования. Означает, что длинная операция закончилась с ошибкой.
#define MEMORY_PARITY_ERROR       (uint8_t)0x08//Специальный ответ, который используется совместно с командами 20 и 21. Эти команды можно найти в официальной документации протокола Modbus.
#define GATEWAY_PATH_UNAVAILABLE  (uint8_t)0x10//Этот ответ означает, что устройство выступает в роли gateway (портала) и не может переслать запрос с входного порта на запрошенное устройство. Обычно значит, что запрос не может быть переслан, так как канал коммуникации отсутствует (сеть не подключена, порт не инициализирован и т.д.).
#define GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND  (uint8_t)0x11//Этот ответ означает, что устройство выступает в роли gateway (портала) и не может получить ответ от запрошенного устройства. Т.е. запрос был переслан на порт устройства. Но gateway не получил от него ответ.
//------End exceptio code---------//

//---Register Types---//
typedef struct{
  uint8_t *data;
	uint8_t len;
}InfoRegisters;

typedef struct{
	uint8_t *bytes;
	uint16_t count;
}CoilRegisters;

typedef struct{ 
	uint8_t *bytes;
	uint16_t count;
}ContactRegisters;

typedef struct{ 
	uint16_t *registers;
	uint16_t count;
}HoldingRegisters;

typedef struct{ 
	uint16_t *registers;
	uint16_t count;
}InputRegisters;
//--------------------------------------------------//

//---Packet Structure---//
typedef struct{
	uint8_t deviceAddress;
	uint8_t command;
	uint8_t data[];
}ModbusFrameRTU;
//--------------------------------------//

//---Common functions---//
uint8_t twoCharToUint8(char* chars);
void Uint8ToTwoChars(char* dest,uint8_t source);
uint16_t Crc16(uint8_t *pData, uint16_t length);
uint8_t Lrc8(uint8_t *pData, uint16_t length);
//--------------------------------------------------//

uint16_t ModbusRequestCallback(uint8_t* rxDataPtr, uint16_t rxDataLen,uint8_t* txDataPtr);//Переопределение

void modbusRequestProcessedCallback(uint8_t modbusRequestType,uint16_t regAddress,uint16_t regCount);

#endif

