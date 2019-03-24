/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		WiFiMain.c
	\brief		WiFi Main function
	\author		Hann Chiu
	\version	2
	\date		2015/01/27
	\copyright	Copyright(C) 2014 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
//#include "typedef.h"
#include "sonix_config.h"
#include "MT7601_WiFiMain.h"
//#include "spif_configs.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"
//#include "connectMgr.h"
//#include "network_handler.h"
#include "lwip/sockets.h"
#include "string.h"
#include "apps/dhcps.h"

//------------------------------------------------------------------------------
#define ETH_P_EAPOL 0x888e

struct netif EMAC_if;

//#ifdef APP_DEV
err_t wifiif_init( struct netif *xNetIf );
//#else
//err_t ethernetif_init(struct netif *netif);
//#endif
//------------------------------------------------------------------------------
/* Called from the TCP/IP thread. */
#define LWIP_PORT_INIT_IPADDR(addr)   IP4_ADDR((addr), 192,168,99,1)
#define LWIP_PORT_INIT_GW(addr)       IP4_ADDR((addr), 172,18,111,254)
#define LWIP_PORT_INIT_NETMASK(addr)  IP4_ADDR((addr), 255,255,255,0)

void LwIPConfig(void *pvParameters)
{
    ip_addr_t xIPAddr, xNetMask, xGateway;

    /* Set up the network interface. */
    ip_addr_set_zero( &xGateway );
    ip_addr_set_zero( &xIPAddr );
    ip_addr_set_zero( &xNetMask );


#if WIFI_AP_MODE
			LWIP_PORT_INIT_GW(&xGateway);
			LWIP_PORT_INIT_IPADDR(&xIPAddr);
			LWIP_PORT_INIT_NETMASK(&xNetMask);
	//printf("Starting lwIP, local interface IP is %s\r\n", ip_ntoa(&xIPAddr));

			netif_set_default( netif_add( &EMAC_if, &xIPAddr, &xNetMask, &xGateway, NULL, wifiif_init, tcpip_input ) );
			netif_set_up( &EMAC_if );
#else
			netif_set_default( netif_add( &EMAC_if, &xIPAddr, &xNetMask, &xGateway, NULL, wifiif_init, tcpip_input ) );
#endif
}
void LwIPConfig_AP(void *pvParameters)
{
    ip_addr_t xIPAddr, xNetMask, xGateway;

    /* Set up the network interface. */
    ip_addr_set_zero( &xGateway );
    ip_addr_set_zero( &xIPAddr );
    ip_addr_set_zero( &xNetMask );


		LWIP_PORT_INIT_GW(&xGateway);
		LWIP_PORT_INIT_IPADDR(&xIPAddr);
		LWIP_PORT_INIT_NETMASK(&xNetMask);
	//printf("Starting lwIP, local interface IP is %s\r\n", ip_ntoa(&xIPAddr));

			netif_set_default( netif_add( &EMAC_if, &xIPAddr, &xNetMask, &xGateway, NULL, wifiif_init, tcpip_input ) );
			netif_set_up( &EMAC_if );
}

void WiFi_Receive(unsigned char* pData, int len)
{
    struct netif *netif;
	struct eth_hdr *ethhdr;
    struct  pbuf *p;

	netif = &EMAC_if;//DummyS	
	p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
		
	if (p == NULL){
        int retry = 0;
		do{
            retry++;
            if(retry >= 100)
                break;
            
            vTaskDelay(10);
            p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);				
		}while(p==NULL);
		
		if(retry >= 100){
            printf("[%s]: pbuf_alloc failed\r\n", __FUNCTION__);
            return;
		}		
	}

	memcpy(p->payload, pData, len);	
	ethhdr = p->payload;
	
	switch (htons(ethhdr->type)) {
    /* IP or ARP packet? */
    case ETHTYPE_IP:
    case ETHTYPE_ARP:
#if PPPOE_SUPPORT
    /* PPPoE packet? */
    case ETHTYPE_PPPOEDISC:
    case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
    /* full packet send to tcpip_thread to process */
        if (netif->input(p, netif)!=ERR_OK){ 
            pbuf_free(p);
            p = NULL;
        }
        break;
    case ETH_P_EAPOL:
        break;
    default:
        pbuf_free(p);
        p = NULL;
        break;
    }
}
void wifi_init(void) 
{
  int wifiInitialdelay = 0;	
	
	//usbh_freertos_init();
	#if WIFI_AP_MODE
		WiFi_Task_Init(NULL, WIFI_RUN_MODE_AP);
	#else
		WiFi_Task_Init(NULL, WIFI_RUN_MODE_DEV);
	#endif  

    do{
        wifiInitialdelay++;
        if(wifiInitialdelay > 99){
						printf("System Reset...\r\n");
						while(1);
           // NVIC_SystemReset(); 
				}
        
        vTaskDelay(33*portTICK_RATE_MS);   
    }while(!WIFI_AP_Initial_Done());
		
    /*Initialize lwip and create APP Task */
    tcpip_init(LwIPConfig, NULL);

 
#if WIFI_AP_MODE
		printf("Start dhcp server...\r\n");
		dhcps_init();
#else
		wifi_Set_AP();
		 while (EMAC_if.ip_addr.addr == 0) {
			vTaskDelay(50);
    }
		printf("Starting lwIP, local interface IP is %s\r\n", ip_ntoa(&EMAC_if.ip_addr));
#endif
}

void wifi_Set_AP(void) 
{
	vTaskDelay(1000);
	
#if 0
//	char ssid[32] = "SNT-SA2-10AU051";
//	int ssid_len = strlen(ssid);
//	char pwd[64]= "dsp2dsp2";
//	int pwd_len = strlen(pwd);
	char ssid[32] = "Sonix-Sw1-TP2.4GHz";
	int ssid_len = strlen(ssid);
	char pwd[64]= "swpc2014";
	int pwd_len = strlen(pwd);
#else	
//	char ssid[32] = "SMC";
		char ssid[32] = "wifi-hotspot";
	int ssid_len = strlen(ssid);
	char pwd[64]= "";
	int pwd_len = strlen(pwd);
#endif	
	WiFi_QueryAndSet(SET_BEACON_SSID, (unsigned char *)ssid, (unsigned short *)&ssid_len);
	WiFi_QueryAndSet(SET_SECURITY_WPA2, (unsigned char *)pwd, (unsigned short *)&pwd_len);
	printf("[%s] SSID(%s), PASSWORD(%s)\r\n", __FUNCTION__, ssid, pwd);
	WiFi_QueryAndSet(SET_START_AP_CONNECT, NULL, NULL);
	
}	

int mt7601_stajoin(char* ssid, char* pwd){
	int wifiInitialdelay = 0;	
	WiFi_Task_Init(NULL, WIFI_RUN_MODE_DEV);
	do{
        wifiInitialdelay++;
        if(wifiInitialdelay > 99){
						printf("System Reset...\r\n");
						while(1);
           // NVIC_SystemReset(); 
				}
        
        vTaskDelay(33*portTICK_RATE_MS);   
    }while(!WIFI_AP_Initial_Done());
	 /*Initialize lwip and create APP Task */
    tcpip_init(LwIPConfig, NULL);
		//wifi_Set_AP();
		vTaskDelay(1000);
//				char ssid[32] = "wifi-hotspot";
		int ssid_len = strlen(ssid);
//	char pwd[64]= "";
		int pwd_len = strlen(pwd);
		WiFi_QueryAndSet(SET_BEACON_SSID, (unsigned char *)ssid, (unsigned short *)&ssid_len);
		WiFi_QueryAndSet(SET_SECURITY_WPA2, (unsigned char *)pwd, (unsigned short *)&pwd_len);
		printf("[%s] SSID(%s), PASSWORD(%s)\r\n", __FUNCTION__, ssid, pwd);
		WiFi_QueryAndSet(SET_START_AP_CONNECT, NULL, NULL);
		 while (EMAC_if.ip_addr.addr == 0) {
			vTaskDelay(50);
    }
		printf("Starting lwIP, local interface IP is %s\r\n", ip_ntoa(&EMAC_if.ip_addr));

		return 0;
	
}

int mt7601_apstart(char* ssid, char* pwd, int security){
	int wifiInitialdelay = 0;	
	WiFi_Task_Init(NULL, WIFI_RUN_MODE_AP);
	do{
        wifiInitialdelay++;
        if(wifiInitialdelay > 99){
						printf("System Reset...\r\n");
						while(1);
           // NVIC_SystemReset(); 
				}
        
        vTaskDelay(33*portTICK_RATE_MS);   
    }while(!WIFI_AP_Initial_Done());
	 /*Initialize lwip and create APP Task */
    tcpip_init(LwIPConfig_AP, NULL);
		dhcps_init();
		printf("Starting lwIP, local interface IP is %s\r\n", ip_ntoa(&EMAC_if.ip_addr));

		return 0;
	
}

