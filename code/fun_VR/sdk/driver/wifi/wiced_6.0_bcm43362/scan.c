/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 *
 * Scan Application
 *
 * Features demonstrated
 *  - WICED scan API
 *
 * This application snippet regularly scans for nearby Wi-Fi access points
 *
 * Application Instructions
 *   Connect a PC terminal to the serial port of the WICED Eval board,
 *   then build and download the application as described in the WICED
 *   Quick Start Guide
 *
 *   Each time the application scans, a list of Wi-Fi access points in
 *   range is printed to the UART
 *
 */

#include <stdlib.h>
#include "wiced.h"
#include "lwip/opt.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "wwd_network.h"
#include "wwd_management.h"
#include "wwd_wifi.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "platform/wwd_platform_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "network/wwd_buffer_interface.h"


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define DELAY_BETWEEN_SCANS       ( 5000 )
//#define AP_SSID              "D-Link_DIR-612"
#define AP_SSID              "wifi-hotspot"
#define AP_PASS              ""
#define AP_SEC               WICED_SECURITY_OPEN

#define COUNTRY              WICED_COUNTRY_AUSTRALIA
#define USE_DHCP             WICED_TRUE
#define IP_ADDR              MAKE_IPV4_ADDRESS( 192, 168,   1,  95 )  /* Not required if USE_DHCP is WICED_TRUE */
#define GW_ADDR              MAKE_IPV4_ADDRESS( 192, 168,   1,   1 )  /* Not required if USE_DHCP is WICED_TRUE */
#define NETMASK              MAKE_IPV4_ADDRESS( 255, 255, 255,   0 )  /* Not required if USE_DHCP is WICED_TRUE */
/* #define PING_TARGET          MAKE_IPV4_ADDRESS( 192, 168,   1, 2 ) */  /* Uncomment if you want to ping a specific IP instead of the gateway*/

#define PING_RCV_TIMEOUT     (1000)    /** ping receive timeout - in milliseconds */
#define PING_DELAY           (1000)    /** Delay between ping response/timeout and the next ping send - in milliseconds */
#define PING_ID              (0xAFAF)
#define PING_DATA_SIZE       (32)      /** ping additional data size to include in the packet */
#define JOIN_TIMEOUT         (10000)   /** timeout for joining the wireless network in milliseconds  = 10 seconds */
#define APP_THREAD_STACKSIZE (5120)


/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
static struct netif wiced_if;
typedef struct
{
    wiced_semaphore_t   semaphore;      /* Semaphore used for signaling scan complete */
    uint32_t            result_count;   /* Count to measure the total scan results    */
} app_scan_data_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

wiced_result_t scan_result_handler( wiced_scan_handler_result_t* malloced_scan_result );

/******************************************************
 *               Variable Definitions
 ******************************************************/
static const wiced_ssid_t ap_ssid =
{
	    .length = sizeof(AP_SSID)-1,
		    .value  = AP_SSID,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

uint8_t length;                  /**< SSID length */
uint8_t value[ SSID_NAME_SIZE ]; /**< SSID name (AP name)  */

char message[1470]="WayneTsai TCP Test\n";
void SNXAPP_SOCKET_UDP_TASK(){
	int socket_fd;
	struct sockaddr_in sa, ra;
		int i=0;

	int sent_data;  
	/* Creates an UDP socket (SOCK_DGRAM) with Internet Protocol Family (PF_INET).
	 * Protocol family and Address family related. For example PF_INET Protocol Family and AF_INET family are coupled.
	 */
	printf("UDP TASK Start...\n");

	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);

	if ( socket_fd < 0 )
	{

		printf("socket call failed");
	}

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
//	sa.sin_addr.s_addr = inet_addr("192.168.119.4");
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(8383);


	/* Bind the TCP socket to the port SENDER_PORT_NUM and to the current
	 * machines IP address (Its defined by SENDER_IP_ADDR). 
	 * Once bind is successful for UDP sockets application can operate
	 * on the socket descriptor for sending or receiving data.
	 */
	if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == -1)
	{
		printf("Bind to Port Number %d ,IP address %d failed\n",3838,IP_ADDR_ANY);
		close(socket_fd);
	}

	memset(&ra, 0, sizeof(struct sockaddr_in));
	ra.sin_family = AF_INET;
	ra.sin_addr.s_addr = inet_addr("10.42.0.1");
	ra.sin_port = htons(3838);


	while(1){
		for(int j=0;j<1;j++){
			memset(message, i++, sizeof(message));
			
			sent_data = sendto(socket_fd, message,sizeof(message),0,(struct sockaddr*)&ra,sizeof(ra));
			if(sent_data < 0)
			{
				printf("send failed\n");
	//			while(1);
	//			close(socket_fd);
				vTaskDelay(5000);
			} 
		}
		//vTaskDelay(10);  //some delay!
	}
//	close(socket_fd);
}
#if 0
#define AP_IP_ADDR              MAKE_IPV4_ADDRESS( 192, 168,   1,  1 )
#define AP_NETMASK              MAKE_IPV4_ADDRESS( 255, 255, 255,   0 )
#define AP_SSID_START           "WICED_Appliance_"
#define AP_PASS                 "12345678"
#define AP_SEC                  WICED_SECURITY_WPA2_AES_PSK  /* WICED_SECURITY_OPEN */
#define AP_CHANNEL              (1)

void application_start( )
{
	wiced_ssid_t ap_ssid;
    wiced_mac_t  my_mac;
    ip4_addr_t ap_ipaddr;
    ip4_addr_t ap_netmask;
    wwd_result_t result;

    /* Initialise Wiced */
    WPRINT_APP_INFO(("Starting Wiced v" WICED_VERSION "\n"));

    wiced_init();
    

    /* Create the SSID */
    wwd_wifi_get_mac_address( &my_mac, WWD_STA_INTERFACE );
    sprintf( (char*) ap_ssid.value, AP_SSID_START "%02X%02X%02X%02X%02X%02X", my_mac.octet[0], my_mac.octet[1], my_mac.octet[2], my_mac.octet[3], my_mac.octet[4], my_mac.octet[5] );
    ap_ssid.length = strlen( (char*)ap_ssid.value );

    WPRINT_APP_INFO(("Starting Access Point: SSID: %s\n", (char*)ap_ssid.value ));
		wwd_wifi_start_ap( &ap_ssid, AP_SEC, (uint8_t*) AP_PASS, sizeof( AP_PASS ) - 1, AP_CHANNEL );
		 /* Setup the network interface */
    ip4_addr_set_u32(&ap_ipaddr, htonl( AP_IP_ADDR ));
    ip4_addr_set_u32(&ap_netmask, htonl( AP_NETMASK ));

    if ( NULL == netif_add( &wiced_if, &ap_ipaddr, &ap_netmask, &ap_ipaddr, (void*) WWD_AP_INTERFACE, ethernetif_init, ethernet_input ) )
    {
        WPRINT_APP_ERROR( ( "Failed to start network interface\n" ) );
        return;
    }
    netif_set_default( &wiced_if );
    netif_set_up( &wiced_if );

    WPRINT_APP_INFO( ( "Network ready IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&wiced_if))));


}
#else
void application_start( )
{
	ip4_addr_t ipaddr, netmask, gw;
//	wwd_result_t result;

	/* Initialise Wiced */
	wiced_init();

	/* Attempt to join the Wi-Fi network */
	WPRINT_APP_INFO(("Joining : " AP_SSID "\n"));
	while ( wwd_wifi_join( &ap_ssid, AP_SEC, (uint8_t*) AP_PASS, sizeof( AP_PASS ) - 1, NULL, WWD_STA_INTERFACE ) != WWD_SUCCESS )
	{
		WPRINT_APP_INFO(("Failed to join  : " AP_SSID "   .. retrying\n"));
	}
	WPRINT_APP_INFO(("Successfully joined : " AP_SSID "\n"));

	/* Setup IP configuration */
	if ( USE_DHCP == WICED_TRUE )
	{
		ip4_addr_set_zero( &gw );
		ip4_addr_set_zero( &ipaddr );
		ip4_addr_set_zero( &netmask );
	}
	else
	{
		ipaddr.addr  = htonl( IP_ADDR );
		gw.addr      = htonl( GW_ADDR );
		netmask.addr = htonl( NETMASK );
	}

	if ( NULL == netif_add( &wiced_if, &ipaddr, &netmask, &gw, (void*) WWD_STA_INTERFACE, ethernetif_init, ethernet_input ) )
	{
		WPRINT_APP_ERROR(( "Could not add network interface\n" ));
		return;
	}

	netif_set_up( &wiced_if );
	netif_set_default( &wiced_if );

	if ( USE_DHCP == WICED_TRUE )
	{
		struct dhcp netif_dhcp;
		WPRINT_APP_INFO(("Obtaining IP address via DHCP\n"));
		dhcp_set_struct( &wiced_if, &netif_dhcp );
		dhcp_start( &wiced_if );
		while ( netif_dhcp.state != DHCP_STATE_BOUND )
		{
			/* wait */
			sys_msleep( 10 );
		}
	}

	WPRINT_APP_INFO( ( "Network ready IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&wiced_if))));
	SNXAPP_SOCKET_UDP_TASK();
	vTaskDelete(NULL);
#if 0
	while(1){
		printf("set 1 bit...\n");
		SDIO_SetBusWidth(0x0);
		vTaskDelay(5000);
		printf("set 4 bit...\n");
		SDIO_SetBusWidth(0x2);
		vTaskDelay(5000);
	}
#endif
}
#endif
