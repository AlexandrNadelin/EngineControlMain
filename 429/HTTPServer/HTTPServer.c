#include "HTTPServer.h"
#include <string.h>
#include "FileSystem.h"
#include "Memory.h"
#include "json.h"
#include "TimeSpan.h"
#include "DeviceData.h"

const static char PAGE_HEADER[] =
"HTTP/1.1 200 OK\r\n\
Server: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)\r\n\
Content-type: text/html\r\n\
Content-Length: 0\r\n\
Connection: keep-alive\r\n\r\n";

#define REBOOT_BIT (uint16_t)0x0001

//---Extern---//
extern Memory* memory;
extern DeviceData deviceData;
extern float crankshaftTimeSpans1[CRANKSHAFT_POS_NUM];
extern float crankshaftTimeSpans2[CRANKSHAFT_POS_NUM];
//extern NetworkParameters networkParameters;
/*extern ClimateControlDevice climateControlDevice;
extern UP_CANClient upCanClient;*/
//---End extern ---//

#define SIZE_HEADER_TMP_BUFFER 256

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
		                                           :err==ERR_ALREADY?"ERR_ALREADY"
		                                           :err==ERR_ISCONN?"ERR_ISCONN"
		                                           :err==ERR_CONN?"ERR_CONN"
		                                           :err==ERR_IF?"ERR_IF"
		                                           :err==ERR_ABRT?"ERR_ABRT"
		                                           :err==ERR_RST?"ERR_RST"
		                                           :err==ERR_CLSD?"ERR_CLSD"
		                                           :err==ERR_ARG?"ERR_ARG"
		                                           :"");
}

void tcpDataProcess(HTTPConnection* connection, uint8_t* request, uint16_t requestLength)
{
	struct fs_file file;
	if(memcmp(request,"GET / HTTP/1.1",sizeof("GET / HTTP/1.1")-1)==0)//
	{
		if(fs_open(&file,"Index.html")==ERR_OK)
		{
			uint8_t HttpHeaderBuffer[SIZE_HEADER_TMP_BUFFER];

			uint16_t headerLenght = sprintf((char*)HttpHeaderBuffer
						    ,"HTTP/1.1 200 OK\r\nServer: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)\r\nContent-type: text/html\r\nContent-Length: %u\r\nConnection: keep-alive\r\n\r\n"
					      ,file.len);

			if((connection->err = tcp_write(connection->pcb,HttpHeaderBuffer,headerLenght,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
			{
				printf("write Index http header ");
				printErrT(connection->err);
				return;
			}

			connection->nextDataTxPtr = (char*)file.data - headerLenght;
			connection->nextDataLength = file.len + headerLenght;
	  }
	}
	else if(memcmp(request,"GET /Update.html",sizeof("GET /Update.html")-1)==0)//
	{
		memoryWriteMagicFlag(RUN_BOOTLOADER_MAGIC_FLAG);//commonData.MagicFlag=RUN_BOOTLOADER_MAGIC_FLAG;
		HAL_NVIC_SystemReset();
	}
	else if(memcmp(request,"GET /",sizeof("GET /")-1)==0 && fs_open(&file, (char*)&request[sizeof("GET /")-1])==ERR_OK)//"GET /style.css"
	{
		char* contentType="text/html";

		for(int i =sizeof("GET /")-1;i<requestLength;i++)
		{
			if(request[i]=='.')
			{
			  if(memcmp(&request[i],".html",sizeof(".html")-1)==0)contentType="text/html";
			  else if(memcmp(&request[i],".css",sizeof(".css")-1)==0)contentType="text/css";
			  else if(memcmp(&request[i],".ico",sizeof(".ico")-1)==0)contentType="image/x-icon";
			  else contentType="application/json";

			  break;
			}
		}

	  uint8_t HttpHeaderBuffer[SIZE_HEADER_TMP_BUFFER];

	  uint16_t headerLenght = sprintf((char*)HttpHeaderBuffer//Content-type: text/html\r\n
								,"HTTP/1.1 200 OK\r\nServer: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)\r\nContent-type: %s\r\nContent-Length: %u\r\nConnection: keep-alive\r\n\r\n"
								,contentType
								,file.len);
		if((connection->err = tcp_write(connection->pcb,HttpHeaderBuffer,headerLenght,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
		{
			printf("write another request header ");
			printErrT(connection->err);
			return;
		}

		connection->nextDataTxPtr = (char*)file.data - headerLenght;
		connection->nextDataLength = file.len + headerLenght;
	}
	else if(memcmp(request, "GET /NetworkParameters.property?=",sizeof("GET /NetworkParameters.property?=")-1) == 0)
	{
		char HttpAnswerBuffer[720];

		uint16_t bodyLenght = sprintf(&HttpAnswerBuffer[256]
				,"{\"IPAddress\":\"%u.%u.%u.%u\",\"SubnetMask\":\"%u.%u.%u.%u\",\"GateWay\":\"%u.%u.%u.%u\",\"Port\":\"%u\"}"
				,memory->networkParameters.ipAddr[0],memory->networkParameters.ipAddr[1],memory->networkParameters.ipAddr[2],memory->networkParameters.ipAddr[3]
				,memory->networkParameters.netMask[0],memory->networkParameters.netMask[1],memory->networkParameters.netMask[2],memory->networkParameters.netMask[3]
				,memory->networkParameters.gateWay[0],memory->networkParameters.gateWay[1],memory->networkParameters.gateWay[2],memory->networkParameters.gateWay[3]
		    ,memory->networkParameters.modbusPort);

		uint16_t headerLenght = sprintf(HttpAnswerBuffer
				,"HTTP/1.1 200 OK\r\nServer: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)\r\nContent-type: application/json\r\nContent-Length: %u\r\nConnection: keep-alive\r\n\r\n"
				,bodyLenght);

		memcpy(&HttpAnswerBuffer[headerLenght],&HttpAnswerBuffer[256],bodyLenght);

		if((connection->err = tcp_write(connection->pcb,HttpAnswerBuffer,headerLenght+bodyLenght,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
		{
			printf("NetworkParameters.property?= ");
			printErrT(connection->err);
			return;
		}
	}
	else if(memcmp(request, "GET /NetworkParameters.property=",sizeof("GET /NetworkParameters.property=")-1) == 0)
	{
		uint16_t startJsonByte = sizeof("GET /NetworkParameters.property=")-1;
		uint16_t inputDataLenght = requestLength-startJsonByte;

		uint16_t urlCursor=0,jsonCursor=0;
		DecodeURL((char*)&request[startJsonByte],&urlCursor,(char*)&request[0],&jsonCursor,inputDataLenght);
		NetworkParameters networkParameters;
		jsonStrToNetParameters((char*)request,jsonCursor, &networkParameters);

		memoryWriteNetworkParameters(&networkParameters);

	  if((connection->err = tcp_write(connection->pcb,PAGE_HEADER,sizeof(PAGE_HEADER)-1,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
		{
			printf("NetworkParameters.property= ");
			printErrT(connection->err);
			return;
		}
	}
	else if(memcmp(request, "GET /reboot",sizeof("GET /reboot")-1) == 0)
	{
		if((connection->err = tcp_write(connection->pcb,PAGE_HEADER,sizeof(PAGE_HEADER)-1,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
		{
			printf("GET /cmd.reboot=1 answer error ");
			printErrT(connection->err);
			return;
		}
		connection->property=REBOOT_BIT;
	}
	else if(memcmp(request, "GET /DeviceData?=",sizeof("GET /DeviceData?=")-1) == 0)
	{
		char HttpAnswerBuffer[1000];

		uint16_t bodyLenght = sprintf(&HttpAnswerBuffer[256]
				,"{\"misfire\":[%u,%u,%u,%u,%u,%u],\"fireError\":[%u,%u,%u,%u,%u,%u],\"misfireCounter\":%u,\"fireErrorCylinder\":%u,\"Times\":[%f,%f,%f,%f,%f,%f]}"
				,deviceData.misfireCylinder[0],deviceData.misfireCylinder[1],deviceData.misfireCylinder[2],deviceData.misfireCylinder[3],deviceData.misfireCylinder[4],deviceData.misfireCylinder[5]
				,deviceData.fireErrorCylinder[0],deviceData.fireErrorCylinder[1],deviceData.fireErrorCylinder[2],deviceData.fireErrorCylinder[3],deviceData.fireErrorCylinder[4],deviceData.fireErrorCylinder[5]
				,deviceData.misfireCounter
		    ,deviceData.fireErrorCounter
		    ,deviceData.cylinder_1_Time
		    ,deviceData.cylinder_2_Time
		    ,deviceData.cylinder_3_Time
		    ,deviceData.cylinder_4_Time
		    ,deviceData.cylinder_5_Time
		    ,deviceData.cylinder_6_Time);

		uint16_t headerLenght = sprintf(HttpAnswerBuffer
				,"HTTP/1.1 200 OK\r\nServer: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)\r\nContent-type: application/json\r\nContent-Length: %u\r\nConnection: keep-alive\r\n\r\n"
				,bodyLenght);

		memcpy(&HttpAnswerBuffer[headerLenght],&HttpAnswerBuffer[256],bodyLenght);

		if((connection->err = tcp_write(connection->pcb,HttpAnswerBuffer,headerLenght+bodyLenght,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
		{
			printf("NetworkParameters.property?= ");
			printErrT(connection->err);
			return;
		}
	}
	else if(memcmp(request, "GET /DebugTimes?=",sizeof("GET /DebugTimes?=")-1) == 0)
	{
		char HttpAnswerBuffer[2000];

		uint16_t bodyLenght = sprintf(&HttpAnswerBuffer[256]
				,"{\"times\":[");
		for(int i =0;i<CRANKSHAFT_POS_NUM;i++)
		{
		  bodyLenght += sprintf(&HttpAnswerBuffer[256+bodyLenght],"%f,",crankshaftTimeSpans1[i]);
		}
		for(int i =0;i<CRANKSHAFT_POS_NUM;i++)
		{
		  bodyLenght += sprintf(&HttpAnswerBuffer[256+bodyLenght],"%f,",crankshaftTimeSpans2[i]);
		}
		
		bodyLenght--;//убрать последнюю запятую
		
		bodyLenght += sprintf(&HttpAnswerBuffer[256+bodyLenght],"]}");

		uint16_t headerLenght = sprintf(HttpAnswerBuffer
				,"HTTP/1.1 200 OK\r\nServer: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)\r\nContent-type: application/json\r\nContent-Length: %u\r\nConnection: keep-alive\r\n\r\n"
				,bodyLenght);

		memcpy(&HttpAnswerBuffer[headerLenght],&HttpAnswerBuffer[256],bodyLenght);

		if((connection->err = tcp_write(connection->pcb,HttpAnswerBuffer,headerLenght+bodyLenght,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
		{
			printf("GET /DebugTimes?= write ");
			printErrT(connection->err);
			return;
		}

		/*if((connection->err = tcp_write(connection->pcb,HttpAnswerBuffer,headerLenght+bodyLenght,TCP_WRITE_FLAG_COPY))!=ERR_OK)
		{
			printf("GET /DebugTimes?= write ");
			printErrT(connection->err);
			return;
		}
		
		if((connection->err = tcp_output(connection->pcb))!=HAL_OK)
		{
			printf("GET /DebugTimes?= output ");
			printErrT(connection->err);
			return;
		}*/
	}
	else if(fs_open(&file, "badRequest.html")==ERR_OK)
	{
		uint8_t HttpHeaderBuffer[SIZE_HEADER_TMP_BUFFER];

		uint16_t lenght = sprintf((char*)HttpHeaderBuffer
				                         ,"HTTP/1.1 400 Bad Request\r\nServer: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)\r\nContent-type: text/html\r\nContent-Length: %u\r\nConnection: keep-alive\r\n\r\n"
				                         ,file.len);

		if((connection->err = tcp_write(connection->pcb,HttpHeaderBuffer,lenght,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
		{
			printf("write bad request header ");
			printErrT(connection->err);
			return;
		}

	  if((connection->err = tcp_write(connection->pcb,file.data, file.len,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
		{
			printf("write another request body ");
			printErrT(connection->err);
			return;
		}
	}
}

err_t connectionRecv(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err)
{
	HTTPConnection* connection = arg;

	if(p==NULL)//browser closed connection
	{
		//tcp_arg(tpcb, NULL);
		tcp_close(tpcb);

		if(arg==NULL)return ERR_BUF;
		HTTPServer* server = connection->server;

		for(uint16_t i=0;i<MEMP_NUM_TCP_PCB-1;i++)
	  {
		  if(server->connections[i]==connection)
		  {
		    server->connections[i]->connectionNumber=0;
        server->connections[i]=0x00000000;
		    free(connection);
	      server->numberOfConnections--;
			  break;
		  }
	  }

		return ERR_OK;
	}

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

	if(arg!=NULL)
	{
		connection->lastReceiveTime = HAL_GetTick();
		tcpDataProcess(connection,requestBuffer,p->tot_len);
	}

	return ERR_OK;
}

static err_t connectionSend(void *arg, struct tcp_pcb *tpcb,u16_t len)//data acknowledged callback
{
	if(arg==NULL)
	{
		return ERR_OK;
	}
	HTTPConnection* connection = arg;

	if(connection->property&REBOOT_BIT)
	{
		HAL_NVIC_SystemReset();
	}
	//!!!Может отправлять кусками то что было приказано отправить командой tcp_write и после каждого куска будет вызвана эта функция
	if(connection->nextDataTxPtr != 0x00000000)
	{
	  connection->nextDataTxPtr+=len;
	  connection->nextDataLength-=len;
	  if(connection->nextDataLength==0)
	  {
		  if((uint32_t)connection->nextDataTxPtr<0x08000000)
		  {
			  //free(connection->nextDataTxPtr)
		  }
		  connection->nextDataTxPtr = 0x00000000;
		  //tcp_close(tpcb);
		  return ERR_OK;
	  }
  }
	else
	{
		//tcp_close(tpcb);
		return ERR_OK;
	}


	uint16_t lenToSend = connection->nextDataLength;

	uint16_t lenSndBuf = tcp_sndbuf(tpcb);
	uint16_t lenMms = tcp_mss(tpcb);// *2/////Чтобы сколько кусков отправлено - столько раз и сработал Колбэк//

	if(lenToSend>lenSndBuf)lenToSend=lenSndBuf;
	if(lenToSend>lenMms)lenToSend=lenMms;

	if((connection->err = tcp_write(connection->pcb,connection->nextDataTxPtr,lenToSend,TCP_WRITE_FLAG_COPY))!=ERR_OK||(connection->err = tcp_output(connection->pcb))!=HAL_OK)
	{
		printf("send next part ");
		printErrT(connection->err);

		connection->nextDataTxPtr = NULL;
		return connection->err;
	}

	return ERR_OK;
}

void  connectionErr(void *arg, err_t err)
{
	printErrT(err);

	if(arg==0x00000000)return;

	HTTPConnection* connection = arg;
	HTTPServer* server = connection->server;

	for(uint16_t i=0;i<MEMP_NUM_TCP_PCB-1;i++)
	{
		if(server->connections[i]==connection)
		{
	    server->connections[i]->connectionNumber=0;
		  server->connections[i]=0x00000000;
		  free(connection);

	    server->numberOfConnections--;
	    printf("UP TCP Server stop con error. Number of con: %u\r\n",server->numberOfConnections);
			break;
		}
	}
}

err_t connectionPoll(void *arg, struct tcp_pcb *tpcb)
{
	if(arg==0x00000000)return ERR_OK;
	HTTPConnection* connection = arg;
	if(GetTimeSpan(connection->lastReceiveTime, HAL_GetTick())>connection->receiveTimeout)//getTimeSpan(connection->lastReceiveTime)>connection->receiveTimeout)
	{
		//tcp_arg(tpcb, NULL);
		tcp_close(tpcb);

		HTTPServer* server = connection->server;

		for(uint16_t i=0;i<MEMP_NUM_TCP_PCB-1;i++)
	  {
		  if(server->connections[i]==connection)
		  {
		    printf("UP TCP connection receive timeout\r\n");
		    server->connections[i]->connectionNumber=0;
        server->connections[i]=0x00000000;
		    free(connection);

	      server->numberOfConnections--;

	      //printf("UP TCP Server stop con. Number of con: %u\r\n",server->numberOfConnections);
			  break;
		  }
	  }
	}

	return ERR_OK;
}

err_t HTTPServerAccept(void *arg,struct tcp_pcb *pcb,err_t err)
{
	uint16_t i=0;
	HTTPServer* httpServer = arg;
	if(httpServer->numberOfConnections>MEMP_NUM_TCP_PCB-2)
	{
		printf("HTTP number of allowed connections exceeded.\r\n");
		tcp_close(pcb);
		HAL_NVIC_SystemReset();
		return ERR_MEM;
	}

	for(i=0;i<MEMP_NUM_TCP_PCB-1;i++)
	{
		if(httpServer->connections[i]==NULL)
		{
			HTTPConnection* connection = malloc(sizeof(HTTPConnection));

			if(connection==0x00000000)
			{
				printf("connection malloc error.\r\n");
		    tcp_close(pcb);
				HAL_NVIC_SystemReset();
				return ERR_MEM;//break;
			}

			connection->receiveTimeout=300000;//10000;//0;
			connection->server=httpServer;
			connection->pcb=pcb;
			connection->err=ERR_OK;

			static uint16_t connectionNumber=0;
      if(++connectionNumber==0)connectionNumber++;

      connection->connectionNumber=connectionNumber;

			httpServer->numberOfConnections++;

			httpServer->connections[i]=connection;

			tcp_arg(pcb, connection);
			tcp_recv(pcb, connectionRecv);//receive complated
      tcp_sent(pcb, connectionSend);//transmit complated
      tcp_err(pcb, connectionErr);//error occurred
      tcp_poll(pcb, connectionPoll, 2);//It can be used by the application to
                                        //check if there are remaining application data that
                                        //needs to be sent or if there are connections that
                                        //need to be closed. Time - interval*0.5 second

      connection->lastReceiveTime=HAL_GetTick();
      connection->nextDataTxPtr=NULL;
      connection->property=0x0000;

			//printf("HTTP Server start con. Number of con: %u\r\n",httpServer->numberOfConnections);
			break;
		}
	}

	if(i==MEMP_NUM_TCP_PCB-1)
	{
	  tcp_close(pcb);
		printf("i==MEMP_NUM_TCP_PCB-1\r\n");
		HAL_NVIC_SystemReset();
		return ERR_MEM;
	}

  return ERR_OK;
}

void HTTPServer_Init(HTTPServer* httpServer)
{
	err_t err;
	httpServer->numberOfConnections=0;
	for(uint16_t i=0;i<MEMP_NUM_TCP_PCB-1;i++)
	{
		httpServer->connections[i]=0x00000000;
	}

	if((httpServer->mainPcb = tcp_new())==0)
	{
	  printf("UP_TCP Server main pcb create error\r\n");
		return;
	}

	if((err = tcp_bind(httpServer->mainPcb,IP_ADDR_ANY,80))!=ERR_OK)
	{
	  printf("UP_TCP Server main pcb bind.");
		printErrT(err);
		return;
	}

	tcp_arg(httpServer->mainPcb, httpServer);

	httpServer->mainPcb = tcp_listen(httpServer->mainPcb);
	tcp_accept(httpServer->mainPcb,HTTPServerAccept);

	//tcp_poll(httpServer->mainPcb, httpServerPoll, 2);
	printf("HTTP Server started.\r\n");
}

