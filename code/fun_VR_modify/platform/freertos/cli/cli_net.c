/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"
#include "cli_net.h"

/* Sockets includes. */
#include "lwip/sockets.h"
#include "apps/wifi_stream.h"

/******************************************************
 *                    Constants
 ******************************************************/
#define TP_TEST_TIMEINSEC			( 60 )		/* 60 sec */
#define TP_REPORT_INTERVAL		( 5 )		

/******************************************************
 *            Static Variable Definitions
 ******************************************************/
static paramtpStruct tpparam;
static TaskHandle_t	NtTaskHandle = NULL;
static int ntclifd= -1;
static int ntsvrfd= -1;

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static BaseType_t prvNetNtStartCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvNetNtStopCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvNetNtStreamCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/




static char message[TCP_MSS]="SONiX TCP Test\n";
static void SNXAPP_SOCKET_TCP_CLIENT_TASK(void *id) 
{
	int ret, snd_len, i=0;
	struct sockaddr_in server;
	paramtpStruct *tcp_tpparam;
	unsigned int CurTime = 0, LastTime = 0;
	unsigned long long total_byte = 0;

	printf("TCP client throughput test...\n");	
	tcp_tpparam = (paramtpStruct *)id;
	printf("ip address : %s:%d - test time:%d\n", tcp_tpparam->ip_addr, tcp_tpparam->port, tcp_tpparam->testinsec);
	
	if ((ntclifd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("Could not create socket\n");
		goto Out1;
	}
	
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_family = AF_INET;
	server.sin_port = htons(0);

	ret = bind(ntclifd , (struct sockaddr*)&server, sizeof(server));
	if (ret < 0) {
		printf("bind error ret = %d\n", ret);
		goto Out2;
	} else {
		printf("bind success\n");
	}

	server.sin_addr.s_addr = inet_addr(tcp_tpparam->ip_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(tcp_tpparam->port);

	if (connect(ntclifd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		printf("connect failed. Error\n");
		goto Out2;
	}

//	fcntl(sock, F_SETFL, O_NONBLOCK);

	CurTime = xTaskGetTickCount();
	LastTime = CurTime;

	printf("start send");
	while(1) {
		/* Send some data */
		memset(message, i,sizeof(message));
		snd_len = send(ntclifd , message , sizeof(message), 0);
		if (snd_len <= 0) {
			total_byte >>= 20;
			CurTime = xTaskGetTickCount();
			if (snd_len < 0)
				printf("Send failed err =%d \n", snd_len);
			else
				printf("Remote close\n");

			printf("Result: total send %llu MBytes in %d sec\n", total_byte, (CurTime - LastTime)*portTICK_RATE_MS / 1000);
			break;
		} else {
			if (snd_len!= sizeof(message)) {
				printf("send partial = %d \n", snd_len);
				/* It may resend the reserved data. */
			}
			else
				printf("+");
			total_byte += snd_len;
		}

		CurTime = xTaskGetTickCount();
		if ((CurTime - LastTime) > (tcp_tpparam->testinsec*1000 / portTICK_RATE_MS)) {
			total_byte >>= 20;
			printf("\nTCPc Test done. Exit\n");
			printf("Result: total send %llu MBytes in %d sec\n", total_byte, tcp_tpparam->testinsec);
			break;
		}
	}

Out2:
	close(ntclifd);
	ntclifd = -1;
Out1:
	NtTaskHandle = NULL;
	vTaskDelete(NULL);
}

static void SNXAPP_SOCKET_TCP_SERVER_TASK(void *id) 
{
	int len=0;
	unsigned int tot_len = 0, i=0;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t clilen;
	paramtpStruct *tcp_tpparam;
	portTickType xStartTime, xEndTime, xElapsed;

	printf("TCP server throughput test...\n");
	tcp_tpparam = (paramtpStruct *)id;
	printf("ip address : %s:%d - test time:%d\n", tcp_tpparam->ip_addr, tcp_tpparam->port, tcp_tpparam->testinsec);	
	
	if ((ntsvrfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("Could not create socket\n");
		goto Out1;
	}

	memset(&servaddr, 0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(tcp_tpparam->port);
	
	if (bind(ntsvrfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) == -1) {
		printf("Bind to Port Number %d , IP address %x failed\n", tcp_tpparam->port, IP_ADDR_ANY);
		goto Out2;
	}

	listen(ntsvrfd,1);

	for(;;)
	{
		clilen=sizeof(cliaddr);
		ntclifd = accept(ntsvrfd,(struct sockaddr *)&cliaddr,&clilen);

		xStartTime = xTaskGetTickCount();
		for(;;)
		{
			len = recvfrom(ntclifd, message, sizeof(message), 0, (struct sockaddr *)&cliaddr, &clilen);
			if(len<0){
				printf("recvfrom failed\n");
				break;
			}
			if(len==0){
				printf("connection closed\n");
				goto Out2;
			}
			tot_len = tot_len + len;
			xEndTime = xTaskGetTickCount();
			xElapsed = ( xEndTime - xStartTime ) * portTICK_RATE_MS;
			if(xElapsed >= (TP_REPORT_INTERVAL*1000)/portTICK_RATE_MS){
				printf("%09d \t",i++);
				printf("Every %ds measure Speed %u Kbytes ", TP_REPORT_INTERVAL, ((tot_len/1024)/(xElapsed/1000)));
				printf("Get data %u  Kbytes\n", (tot_len >>= 10));
				xStartTime = xTaskGetTickCount();
				tot_len =0;
			}
		}
	}
	
Out2:
	close(ntclifd);
	ntclifd = -1;
	close(ntsvrfd);
	ntsvrfd = -1;
Out1:
	NtTaskHandle = NULL;
	vTaskDelete(NULL);
	
}

static void SNXAPP_SOCKET_UDP_CLIENT_TASK(void *id) 
{
	struct sockaddr_in sa, ra;
	paramtpStruct *udp_tpparam;
	unsigned int CurTime = 0, LastTime = 0;
	unsigned long long total_byte = 0;
	int sent_data;

	/* Creates an UDP socket (SOCK_DGRAM) with Internet Protocol Family (PF_INET).
	 * Protocol family and Address family related. For example PF_INET Protocol Family and AF_INET family are coupled.
	 */
	
	printf("UDP client throughput test...\n");
	udp_tpparam = (paramtpStruct *)id;
	printf("ip address : %s:%d - test time:%d\n", udp_tpparam->ip_addr, udp_tpparam->port, udp_tpparam->testinsec);

	ntclifd = socket(PF_INET, SOCK_DGRAM, 0);
	if (ntclifd < 0) {
		printf("socket call failed");
		goto Out1;
	}

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(0);

	if (bind(ntclifd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == -1) {
		printf("Bind to Port Number %d , IP address %x failed\n", 3838, IP_ADDR_ANY);
		goto Out2;
	}

	memset(&ra, 0, sizeof(struct sockaddr_in));
	ra.sin_family = AF_INET;
	ra.sin_addr.s_addr = inet_addr(udp_tpparam->ip_addr);
	ra.sin_port = htons(udp_tpparam->port);
	CurTime = xTaskGetTickCount();
	LastTime = CurTime;

	printf("start send");
	while(1) {
		sent_data = sendto(ntclifd, message, sizeof(message), 0, (struct sockaddr*)&ra, sizeof(ra));
		if (sent_data < 0) {
			total_byte >>= 20;
			CurTime = xTaskGetTickCount();
			printf("send failed \n");
			printf("Result: total send %llu MBytes in %d sec\n", total_byte, (CurTime - LastTime)*portTICK_RATE_MS / 1000);
			break;
		} else {
			//printf("+");
			total_byte += sent_data;
		}
		
		CurTime = xTaskGetTickCount();
		if ((CurTime - LastTime) > (udp_tpparam->testinsec*1000 / portTICK_RATE_MS)) {
			total_byte >>= 20;
			printf("\nUDPc Test done. Exit\n");
			printf("Result: total send %llu MBytes in %d sec\n", total_byte, udp_tpparam->testinsec);
			break;
		}
	}

Out2:
	close(ntclifd);
	ntclifd = -1;
Out1:
	NtTaskHandle = NULL;
	vTaskDelete(NULL);	
}

static void SNXAPP_SOCKET_USP_SERVER_TASK(void *id){
	/* file description into transport */
	int length; /* length of address structure      */
	int nbytes; /* the number of read **/
	struct sockaddr_in myaddr; /* address of this service */
	struct sockaddr_in client_addr; /* address of client    */
	paramtpStruct *udp_tpparam;
	unsigned int tot_len = 0, i=0;
	portTickType xStartTime, xEndTime, xElapsed;

	printf("UDP server throughput test...\n");
	udp_tpparam = (paramtpStruct *)id;
	printf("ip address : %s:%d - test time:%d\n", udp_tpparam->ip_addr, udp_tpparam->port, udp_tpparam->testinsec);

	/*
	*      Get a socket into UDP/IP
	*/
	if ((ntsvrfd = socket(AF_INET, SOCK_DGRAM, 0)) <0) {
		printf("socket call failed");
		goto Out1;
	}
	/*
	*    Set up our address
	*/
	memset ((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(udp_tpparam->port);

	/*
	*     Bind to the address to which the service will be offered
	*/
	if (bind(ntsvrfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) <0) {
		printf("Bind to Port Number %d , IP address %x failed\n", udp_tpparam->port, IP_ADDR_ANY);
		goto Out2;	
	}

	/*
	* Loop continuously, waiting for datagrams
	* and response a message
	*/
	length = sizeof(client_addr);
	printf("Server is ready to receive !!\n");
	xStartTime = xTaskGetTickCount();
	while (1) {
		nbytes = recvfrom(ntsvrfd, message, sizeof(message), 0, (struct sockaddr*)&client_addr, (socklen_t *)&length);
		if (nbytes <0) {
			printf ("could not read datagram!!");
			goto Out2;
		}
		tot_len = tot_len + nbytes;
		xEndTime = xTaskGetTickCount();
		xElapsed = ( xEndTime - xStartTime ) * portTICK_RATE_MS;
		if(xElapsed >= (TP_REPORT_INTERVAL*1000)/portTICK_RATE_MS){
			printf("%09d \t",i++);
			printf("Every %ds measure Speed %u Kbytes ", TP_REPORT_INTERVAL, ((tot_len/1024)/(xElapsed/1000)));
			printf("Get data %u  Kbytes\n", (tot_len >>= 10));
			xStartTime = xTaskGetTickCount();
			tot_len =0;
		}
	}
		
Out2:
	close(ntsvrfd);
	ntsvrfd = -1;
Out1:
	NtTaskHandle = NULL;	
	vTaskDelete(NULL);
}

static const CLI_Command_Definition_t xNet_NtStart =
{
	"nt_start", /* The command string to type. */
	"\r\nnt_start [cli/svr] [tcp/udp] [ip] [port]\r\n  Networking test.\r\n",
	prvNetNtStartCommand, /* The function to run. */
	4 /* parameters are expected. */
};

static const CLI_Command_Definition_t xNet_NtStop =
{
	"nt_stop", /* The command string to type. */
	"\r\nnt_stop\r\n  Stop Networking test.\r\n",
	prvNetNtStopCommand, /* The function to run. */
	0 /* parameters are expected. */
};

static const CLI_Command_Definition_t xNet_NtStream =
{
	"nt_stream", /* The command string to type. */
	"\r\nnt_stream [start/stop]\r\n  Start or Stop network streaming transmit.\r\n",
	prvNetNtStreamCommand, /* The function to run. */
	1 /* parameters are expected. */
};

static BaseType_t prvNetNtStartCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	const char	*param; 
	char	dir[4], 
				protocol[4], 
				ip[16],
				port[8];
	BaseType_t 	xParameterStringLength;

	if(NtTaskHandle){
		printf("nt Task is running...\n");
		return pdFALSE;
	}
	
	if(ntsvrfd >= 0){
		printf("nt server fd is used...\n");
		return pdFALSE;
	}
	
	if(ntclifd >= 0){
		printf("nt clinet fd is used...\n");
		return pdFALSE;
	}
		
	
	memset(dir, 0, sizeof(dir));
	memset(protocol, 0, sizeof(protocol));
	memset(ip, 0, sizeof(ip));
	memset(port, 0, sizeof(port));
	
	param = FreeRTOS_CLIGetParameter
					(
							pcCommandString,        /* The command string itself. */
							1,                      /* Return the first parameter. */
							&xParameterStringLength /* Store the parameter string length. */
					);
	strncpy(dir, param, xParameterStringLength);
	
	param = FreeRTOS_CLIGetParameter
					(
							pcCommandString,        /* The command string itself. */
							2,                      /* Return the first parameter. */
							&xParameterStringLength /* Store the parameter string length. */
					);
	strncpy(protocol, param, xParameterStringLength);
	
		param = FreeRTOS_CLIGetParameter
					(
							pcCommandString,        /* The command string itself. */
							3,                      /* Return the first parameter. */
							&xParameterStringLength /* Store the parameter string length. */
					);
	strncpy(ip, param, xParameterStringLength);
	
	param = FreeRTOS_CLIGetParameter
					(
							pcCommandString,        /* The command string itself. */
							4,                      /* Return the first parameter. */
							&xParameterStringLength /* Store the parameter string length. */
					);
	strncpy(port, param, xParameterStringLength);
	
	/* reset test parameters */
	memset(&tpparam, 0x0, sizeof(paramtpStruct));
	strcpy(tpparam.ip_addr, ip);
	tpparam.port = atoi(port);
	tpparam.testinsec = TP_TEST_TIMEINSEC;


	
	if(!strcmp(dir, "svr")){
		if(!strcmp(protocol, "tcp")){
				if (pdPASS != xTaskCreate(SNXAPP_SOCKET_TCP_SERVER_TASK, "TCPs task", 2048, (void *)&tpparam, 60, &NtTaskHandle)) {
						printf("Couldn't create TCPs task\r\n");
						return pdFAIL;
				}
		}else if(!strcmp(protocol, "udp")){
				if (pdPASS != xTaskCreate(SNXAPP_SOCKET_USP_SERVER_TASK, "UDPs task", 2048, (void *)&tpparam, 60, &NtTaskHandle)) {
						printf("Couldn't create UDPs task\r\n");
						return pdFAIL;
				}

		}
	}else if(!strcmp(dir, "cli")){
		if(!strcmp(protocol, "tcp")){ /* TCP Client */
			if (pdPASS != xTaskCreate(SNXAPP_SOCKET_TCP_CLIENT_TASK, "TCPc task", 2048, (void *)&tpparam, 60, &NtTaskHandle)) {
				printf("Couldn't create TCPc task\r\n");
				return pdFAIL;
			}
		}else if(!strcmp(protocol, "udp")){ /* UDP Client */
			if (pdPASS != xTaskCreate(SNXAPP_SOCKET_UDP_CLIENT_TASK, "UDPc task", 2048, (void *)&tpparam, 60, &NtTaskHandle)) {
				printf("Couldn't create UDPc task\r\n");
				return pdFAIL;
			}
		}
	}
	return pdFALSE;
}

static BaseType_t prvNetNtStopCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    
	if(ntsvrfd >= 0){
		printf("Close nt server fd...\n");
		close(ntsvrfd);
		ntsvrfd = -1;
	}
	
	if(ntclifd >= 0){
		printf("Close nt client fd...\n");
		close(ntclifd);
		ntclifd = -1;
	}
	
	if(NtTaskHandle){
		printf("Delete nt task...\n");
		vTaskDelete(NtTaskHandle);
		NtTaskHandle = NULL;
	}
	return pdFALSE;
}

/*-----------------------------------------------------------*/
static BaseType_t prvNetNtStreamCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	BaseType_t 	xParameterStringLength;
	const char	*param;
	char	action[8];

	memset(action, 0, sizeof(action));
	param = FreeRTOS_CLIGetParameter
					(
							pcCommandString,        /* The command string itself. */
							1,                      /* Return the first parameter. */
							&xParameterStringLength /* Store the parameter string length. */
					);

	strncpy(action, param, xParameterStringLength);

	if(!strcmp(action, "start")){
		printf("Start wifi stream...\n");
		if (pdPASS != xTaskCreate(WIFI_STREAM_VERIFY_TASK, "Wifi stream task", 2048, (void *)&tpparam, 60, &NtTaskHandle)) {
				printf("Couldn't create Wifi steam task\r\n");
				return pdFAIL;
		}
	}else if(!strcmp(action, "stop")){
		printf("Stop wifi stream...\n");
	}

	return pdFALSE;
}


/*-----------------------------------------------------------*/

void vRegisterNetCLICommands( void )
{
	FreeRTOS_CLIRegisterCommand( &xNet_NtStart ); 
	FreeRTOS_CLIRegisterCommand( &xNet_NtStop ); 
	FreeRTOS_CLIRegisterCommand( &xNet_NtStream );
}
