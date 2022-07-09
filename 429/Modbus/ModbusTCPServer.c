#include "ModbusTCPServer.h"
#include "ModbusCommon.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Memory.h"
#include "TimeSpan.h"

extern Memory* memory;

//Коллбэк - принят запрос который предназначен для обработки этим устройством
/*__WEAK unsigned short processUPRequestDataCallback(unsigned char* packetData,unsigned short packetDataLenght, unsigned char* outBufferData)
{
  LWIP_UNUSED_ARG(packetData);
  LWIP_UNUSED_ARG(packetDataLenght);
  LWIP_UNUSED_ARG(outBufferData);
	return 0;
}*/

static void printErrT(err_t err)
{
	printf("Error: %s\r\n",err==ERR_MEM?"ERR_MEM":err==ERR_OK?"ERR_OK"
		                                           :err==ERR_BUF?"ERR_BUF"
		                                           :err==ERR_TIMEOUT?"ERR_TIMEOUT"
		                                           :err==ERR_RTE?"ERR_RTE"
		                                           :err==ERR_INPROGRESS?"ERR_INPROGRESS"
		                                           :err==ERR_VAL?"ERR_VAL"
		                                           :err==ERR_WOULDBLOCK?"ERR_WOULDBLOCK"
		                                           :err==ERR_USE?"ERR_USE"
		                                           //:err==ERR_ALREADY?"ERR_ALREADY"
		                                           :err==ERR_ISCONN?"ERR_ISCONN"
		                                           :err==ERR_CONN?"ERR_CONN"
		                                           :err==ERR_IF?"ERR_IF"
		                                           :err==ERR_ABRT?"ERR_ABRT"
		                                           :err==ERR_RST?"ERR_RST"
		                                           :err==ERR_CLSD?"ERR_CLSD"
		                                           :err==ERR_ARG?"ERR_ARG"
		                                           :"");
}

//Колбэк - пакет принят
err_t modbusTCPConnectionRecv(void *_connection, struct tcp_pcb *tpcb,struct pbuf *p, err_t err)
{
	LWIP_UNUSED_ARG(err);
	ModbusTCPConnection* connection = _connection;

	if(p==NULL)//Запрос на закрытие соединения
	{
		tcp_close(tpcb);

		if(_connection==NULL)return ERR_BUF;
	  ModbusTCPServer* server = connection->server;

		for(uint16_t i=0;i<MEMP_NUM_TCP_PCB-3;i++)
	  {
		  if(server->connections[i]==connection)
		  {
		    server->connections[i]->connectionNumber=0;
        server->connections[i]=NULL;
		    free(connection);
	      server->numberOfConnections--;
			  break;
		  }
	  }

		return ERR_OK;
	}

	//Пришли данные - обрабатываем
	struct pbuf *p_temp = p;

	uint8_t requestBuffer[p_temp->tot_len+1];
	uint16_t position=0;

	while(p_temp != NULL)
  {
    memcpy(&requestBuffer[position],p_temp->payload,p_temp->len);
		position+=p_temp->len;
    p_temp = p_temp->next;
  }

	tcp_recved(tpcb,p->tot_len);
	pbuf_free(p);

	requestBuffer[p->tot_len]=0;

	connection->lastReceiveTime = HAL_GetTick();
	uint16_t answerLen=0;
	uint8_t outBuffer[256];
	if((answerLen = ModbusRequestCallback(requestBuffer,p->tot_len,outBuffer))>0)//Обработка запроса
	{
		if((connection->err = tcp_write(connection->pcb,outBuffer,answerLen,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
		{
		   printf("ModbusTCPServer write error \r\n");
		   printErrT(connection->err);
		   return ERR_OK;
		}
	}

	return ERR_OK;
}

//Колбэк завершения отправки ( не используется)
static err_t modbusTCPConnectionSent(void *arg, struct tcp_pcb *tpcb,u16_t len)//data acknowledged callback
{
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(tpcb);
	LWIP_UNUSED_ARG(len);

	return ERR_OK;
}

//Колбэк ошибки
void  modbusTCPConnectionErr(void *arg, err_t err)
{
	printf("ModbusTCPServer error callback. ");
	printErrT(err);

	if(arg==NULL)return;

	ModbusTCPConnection* connection = arg;
	ModbusTCPServer* server = connection->server;

	for(uint16_t i=0;i<MEMP_NUM_TCP_PCB-3;i++)
	{
		if(server->connections[i]==connection)
		{
	    server->connections[i]->connectionNumber=0;
		  server->connections[i]=NULL;
		  free(connection);

	    server->numberOfConnections--;
	    printf("ModbusTCPServer connection error: ");
	    printErrT(err);
			break;
		}
	}
}

//Колбэк который возникает периодически пока подключение активно. В нем можно отслеживать таймаут
err_t modbusTCPConnectionPoll(void *arg, struct tcp_pcb *tpcb)
{
	if(arg==NULL)return ERR_OK;
	ModbusTCPConnection* connection = arg;
	if(GetTimeSpan(connection->lastReceiveTime, HAL_GetTick())>connection->receiveTimeout)
	{
		tcp_close(tpcb);

		ModbusTCPServer* server = connection->server;

		for(uint16_t i=0;i<MEMP_NUM_TCP_PCB-3;i++)
	  {
		  if(server->connections[i]==connection)
		  {
		    server->connections[i]->connectionNumber=0;
        server->connections[i]=NULL;
		    free(connection);

	      server->numberOfConnections--;
			  break;
		  }
	  }

	  printf("ModbusTCPServer connection timeout.");
	}

	return ERR_OK;
}

//Колбэк входящего подключения протокола TCP
err_t modbusTCPServerAccept(void *arg,struct tcp_pcb *pcb,err_t err)
{
	if(err!=ERR_OK)return err;
	uint16_t i=0;
	ModbusTCPServer* server = arg;
	if(server->numberOfConnections>MEMP_NUM_TCP_PCB-2)
	{
	  printf("ModbusTCPServer number of allowed connections exceeded.\r\n");
		tcp_close(pcb);
		NVIC_SystemReset();
		return ERR_MEM;
	}

	for(i=0;i<MEMP_NUM_TCP_PCB-3;i++)
	{
		if(server->connections[i]==NULL)
		{
			ModbusTCPConnection* upTCPConnection = malloc(sizeof(ModbusTCPConnection));

			if(upTCPConnection==NULL)
			{
	      printf("ModbusTCPServer malloc error.\r\n");
		    tcp_close(pcb);
				NVIC_SystemReset();
				return ERR_MEM;//break;
			}

			upTCPConnection->receiveTimeout=30000;//10000;//0;
			upTCPConnection->server=server;
			upTCPConnection->pcb=pcb;
			upTCPConnection->err=ERR_OK;

			static uint16_t connectionNumber=0;
      if(++connectionNumber==0)connectionNumber++;

      upTCPConnection->connectionNumber=connectionNumber;

			server->numberOfConnections++;

			server->connections[i]=upTCPConnection;

			tcp_arg(pcb, upTCPConnection);
			tcp_recv(pcb, modbusTCPConnectionRecv);//receive complated
      tcp_sent(pcb, modbusTCPConnectionSent);//transmit complated
      tcp_err(pcb, modbusTCPConnectionErr);//error occurred
      tcp_poll(pcb, modbusTCPConnectionPoll, 2);//It can be used by the application to
                                        //check if there are remaining application data that
                                        //needs to be sent or if there are connections that
                                        //need to be closed. Time - interval*0.5 second

      upTCPConnection->lastReceiveTime=HAL_GetTick();
      upTCPConnection->property=0x0000;
			break;
		}
	}

	if(i==MEMP_NUM_TCP_PCB-1)
	{
	  tcp_close(pcb);
	  printf("ModbusTCPServer malloc error.\r\n");
		NVIC_SystemReset();
		return ERR_MEM;
	}

  return ERR_OK;
}

//Инициализация сервера универсального протокола
void ModbusTCPServer_Init(ModbusTCPServer* server)
{
  err_t err;

  //Инициализация TCP сервера
	server->numberOfConnections=0;
	for(uint16_t i=0;i<MEMP_NUM_TCP_PCB-3;i++)
	{
		server->connections[i]=NULL;
	}

  if((server->tcpServerPcb = tcp_new())==0)//Ошибка выделения памяти под объект сервера
	{
  	printf("ModbusTCPServer main pcb create error\r\n");
		return;
	}

  if((err = tcp_bind(server->tcpServerPcb,IP_ADDR_ANY,server->port))!=ERR_OK)//Ошибка привязки к порту TCP сервера
  {
  	printf("ModbusTCPServer main pcb bind.");
  	printErrT(err);
		return;
  }

  tcp_arg(server->tcpServerPcb, server);//Установка аргумента TCP сервера

  server->tcpServerPcb = tcp_listen(server->tcpServerPcb);//Запуск прослушивания TCP сервера
  tcp_accept(server->tcpServerPcb,modbusTCPServerAccept);
  
  printf("ModbusTCPServer started. IP: %u.%u.%u.%u; Port: %u\r\n",memory->networkParameters.ipAddr[0],memory->networkParameters.ipAddr[1],memory->networkParameters.ipAddr[2],memory->networkParameters.ipAddr[3],memory->networkParameters.modbusPort);
}

