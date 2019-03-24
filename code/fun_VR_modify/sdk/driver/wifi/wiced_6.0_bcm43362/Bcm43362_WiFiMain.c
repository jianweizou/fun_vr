#include "FreeRTOS.h"
#include "task.h"
#include "wiced_defaults.h"
#include "wiced_utilities.h"
#include "wiced.h"
#include "apps/dhcps.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define MAX_PASSPHRASE_LEN   ( 64 )
#define MIN_PASSPHRASE_LEN   (  8 )
#define SCAN_RETRY_CNT  (5)
#define SCAN_RETRY_WAIT_PERIOD ( 500 )	


/******************************************************
 *                    Structures
 ******************************************************/
typedef struct
{
    wiced_semaphore_t   semaphore;      /* Semaphore used for signaling scan complete */
    uint32_t            result_count;   /* Count to measure the total scan results    */
} app_scan_data_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

#ifndef APPLICATION_STACK_SIZE
#define APPLICATION_STACK_SIZE WICED_DEFAULT_APPLICATION_STACK_SIZE
#endif

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER( x ) ( (void)(x) )
#endif

//---------------------------------------------------------------------------------------------------------------------
wiced_result_t bcm43362_init(void){
	wiced_result_t result;
	if((result = wiced_init())!=WICED_SUCCESS)
		WPRINT_APP_INFO(("leave failed 0x%x\n", result));

   return result;
}

//---------------------------------------------------------------------------------------------------------------------
wiced_result_t bcm43362_sta_join(char *ssid, char *key)
{
	wiced_result_t result;
	uint8_t retry_cnt = 0;
	wiced_scan_result_t ap_info;

	wiced_config_ap_entry_t *ap, ap_entry;

	memset(&ap_info, 0, sizeof(wiced_scan_result_t));
	memset(&ap_entry, 0, sizeof(wiced_config_ap_entry_t));
	
	
	ap_info.security = WICED_SECURITY_UNKNOWN;
	while((retry_cnt < SCAN_RETRY_CNT) && ( result != WICED_SUCCESS)){
         result = wiced_wifi_find_ap(ssid, &ap_info, NULL );
         retry_cnt++;
         host_rtos_delay_milliseconds(SCAN_RETRY_WAIT_PERIOD);
  }


  if(retry_cnt == SCAN_RETRY_CNT)
  {
            WPRINT_APP_INFO(("Can not get ap info for %s.\n", ssid));
						return WICED_BADARG;
  }

  if(ap_info.security != WICED_SECURITY_OPEN && key == NULL){
        WPRINT_APP_INFO(("Key can not NULL.\n"));
        return WICED_BADARG;
  }
		
	ap = &ap_entry;
	ap->details.SSID.length = strlen(ssid);
  strncpy((char *)&ap->details.SSID.value, ssid, ap->details.SSID.length);
  ap->details.security = ap_info.security ;
	ap->security_key_length = strlen(key);
	strncpy((char *)&ap->security_key, key, ap->security_key_length);
	
	if((result = wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL, &ap_entry)) != WICED_SUCCESS)
		WPRINT_APP_INFO(("start sta failed 0x%x\n", result));

	return result;

}

//---------------------------------------------------------------------------------------------------------------------
wiced_result_t bcm43362_sta_leave(void)
{
     wiced_result_t result;
     result = wiced_network_down( WICED_STA_INTERFACE );

     if (WICED_SUCCESS != result)
     {
         WPRINT_APP_INFO(("leave failed 0x%x\n", result));
     }else
				WPRINT_APP_INFO(("leave Success \n"));
     return result;
}

//---------------------------------------------------------------------------------------------------------------------
wiced_security_t wifi_utils_str_to_authtype( char* arg )
{
    if ( strcmp( arg, "open" ) == 0 )
    {
        return WICED_SECURITY_OPEN;
    }
    else if ( strcmp( arg, "wep" ) == 0 )
    {
        return WICED_SECURITY_WEP_PSK;
    }
    else if ( strcmp( arg, "wep_shared" ) == 0 )
    {
        return WICED_SECURITY_WEP_SHARED;
    }
    else if ( strcmp( arg, "wpa2_tkip" ) == 0 )
    {
        return WICED_SECURITY_WPA2_TKIP_PSK;
    }
    else if ( strcmp( arg, "wpa2_aes" ) == 0 )
    {
        return WICED_SECURITY_WPA2_AES_PSK;
    }
    else if ( strcmp( arg, "wpa2" ) == 0 )
    {
        return WICED_SECURITY_WPA2_MIXED_PSK;
    }
    else if ( strcmp( arg, "wpa_aes" ) == 0 )
    {
        return WICED_SECURITY_WPA_AES_PSK;
    }
    else if ( strcmp( arg, "wpa_tkip" ) == 0 )
    {
        return WICED_SECURITY_WPA_TKIP_PSK;
    }
    else
    {
        WPRINT_APP_INFO( ("Bad auth type: '%s'\r\n", arg) );
                                 return WICED_SECURITY_UNKNOWN;
    }
}

//---------------------------------------------------------------------------------------------------------------------
wiced_result_t bcm43362_ap_start(char *ssid, char *security, char* key)
{
    wiced_result_t result;
    wiced_security_t auth_type = wifi_utils_str_to_authtype(security);
    uint8_t  key_length = 0;
		wiced_config_ap_entry_t* ap, ap_entry;
		
	
    static const wiced_ip_setting_t ap_ip_settings =
    {
        INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS( 192,168,  99,  1 ) ),
        INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS( 255,255,255,  0 ) ),
        INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS( 192,168,  99,  1 ) ),
    };

    if ( wwd_wifi_is_ready_to_transceive( WWD_AP_INTERFACE ) == WWD_SUCCESS )
    {
        WPRINT_APP_INFO(( "AP already started\n" ));
        return WICED_SUCCESS;
    }

    if ( ( auth_type != WICED_SECURITY_WPA2_AES_PSK ) &&
         ( auth_type != WICED_SECURITY_OPEN ) &&
         ( auth_type != WICED_SECURITY_WPA2_MIXED_PSK ) &&
         ( auth_type != WICED_SECURITY_WEP_PSK ) &&
         ( auth_type != WICED_SECURITY_WEP_SHARED ) )
    {
        WPRINT_APP_INFO(( "Error: Invalid security type\n" ));
        return WICED_BADARG;
    }

    key_length = strlen(key);

    if ( ( auth_type & WPA2_SECURITY ) && ( key_length < MIN_PASSPHRASE_LEN ) )
    {
        WPRINT_APP_INFO(("Error: WPA key too short\n" ));
        return WICED_BADARG;
    }
 /* Check arg */
    if ( ssid == NULL )
    {
        WPRINT_APP_INFO(( "SSID can not be null\n" ));
        return WICED_BADARG;
    }

    if ( key_length > MAX_PASSPHRASE_LEN)
    {
        WPRINT_APP_INFO(( "Error: Key than 64 characters\n" ));
        return WICED_BADARG;
    }

    if (strlen(ssid) > SSID_NAME_SIZE)
    {
        WPRINT_APP_INFO(( "Error: SSID longer than 32 characters\n" ));
        return WICED_BADARG;
    }

		ap = &ap_entry;
		ap->details.SSID.length = strlen(ssid);
		strncpy((char *)&ap->details.SSID.value, ssid, ap->details.SSID.length);
		ap->details.security = auth_type ;
		ap->security_key_length = strlen(key);
		strncpy((char *)&ap->security_key, key, ap->security_key_length);

		if ( ( result = wiced_network_up( WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &ap_ip_settings, ap ) ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("Error starting AP %u\n", (unsigned int)result));
    }else{
        dhcps_init();
    }
    return result;

}

//---------------------------------------------------------------------------------------------------------------------
wiced_result_t bcm43362_ap_stop(void)
{
    wiced_result_t result;

    if (wwd_wifi_is_ready_to_transceive( WWD_AP_INTERFACE ) != WWD_SUCCESS)
    {
        return WICED_SUCCESS;
    }

    wwd_wifi_deauth_all_associated_client_stas( WWD_DOT11_RC_UNSPECIFIED, WWD_AP_INTERFACE );

    result = wiced_network_down( WICED_AP_INTERFACE );
    if( wiced_network_down( WICED_AP_INTERFACE ) == WWD_SUCCESS)
    {
        dhcps_deinit();
    }else{
        WPRINT_APP_INFO(("stop ap failed.\n"));
    }
    return result;
}
//---------------------------------------------------------------------------------------------------------------------

/*
 * Callback function to handle scan results
 */
wiced_result_t scan_result_handler( wiced_scan_handler_result_t* malloced_scan_result )
{
    /* Validate the input arguments */
    wiced_assert("Bad args", malloced_scan_result != NULL);

    if ( malloced_scan_result != NULL )
    {
        app_scan_data_t* scan_data  = (app_scan_data_t*)malloced_scan_result->user_data;

        malloc_transfer_to_curr_thread( malloced_scan_result );

        if ( malloced_scan_result->status == WICED_SCAN_INCOMPLETE )
        {
            wiced_scan_result_t* record = &malloced_scan_result->ap_details;

            WPRINT_APP_INFO( ( "%3ld ", scan_data->result_count ) );
            print_scan_result(record);
            scan_data->result_count++;
        }
        else
        {
            wiced_rtos_set_semaphore( &scan_data->semaphore );
        }

        free( malloced_scan_result );
    }
    return WICED_SUCCESS;
}

//---------------------------------------------------------------------------------------------------------------------
void bcm43362_scan(void)
{
    wiced_time_t    scan_start_time;
    wiced_time_t    scan_end_time;
    app_scan_data_t scan_data;

    /* Initialize the semaphore that will tell us when the scan is complete */
    wiced_rtos_init_semaphore(&scan_data.semaphore);
    scan_data.result_count = 0;
    WPRINT_APP_INFO( ( "Waiting for scan results...\n" ) );
    WPRINT_APP_INFO( ("  # Type  BSSID              RSSI Rate Chan Security               SSID                            CCode    Flag\n" ) );
    WPRINT_APP_INFO( ("------------------------------------------------------------------------------------------------------------------\n" ) );
    /* Start the scan */
    wiced_time_get_time(&scan_start_time);
    wiced_wifi_scan_networks(scan_result_handler, &scan_data );

    /* Wait until scan is complete */
    wiced_rtos_get_semaphore(&scan_data.semaphore, WICED_WAIT_FOREVER);
    wiced_time_get_time(&scan_end_time);

    WPRINT_APP_INFO( ("\nScan complete in %lu milliseconds\n", (unsigned long )(scan_end_time - scan_start_time) ) );

    /* Clean up */
    wiced_rtos_deinit_semaphore(&scan_data.semaphore);

}

//---------------------------------------------------------------------------------------------------------------------
 wiced_result_t bcm43362_powersave(int enable)
{
    if(!enable)
        return wiced_disable_powersave();
     else if(enable ==1)
         return wiced_enable_powersave();
     else
         return wiced_wifi_enable_powersave_with_throughput(enable);

}
