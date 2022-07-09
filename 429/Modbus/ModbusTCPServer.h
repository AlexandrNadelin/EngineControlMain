#ifndef __ModbusTCPServer_H
#define __ModbusTCPServer_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "main.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"

typedef struct{
	struct tcp_pcb *pcb;
	void* server;
	err_t err;
	uint32_t lastReceiveTime;
	uint32_t receiveTimeout;

	/*char* nextDataTxPtr;
	uint16_t nextDataLength;*/
	uint16_t property;
	uint16_t connectionNumber;
}ModbusTCPConnection;

typedef struct{
  uint8_t numberOfConnections;
  uint16_t port;
  ModbusTCPConnection* connections[MEMP_NUM_TCP_PCB-1];
	struct tcp_pcb *tcpServerPcb;
}ModbusTCPServer;


void ModbusTCPServer_Init(ModbusTCPServer* server);


#ifdef __cplusplus
 }
#endif
#endif

