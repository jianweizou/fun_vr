#include "network/wwd_buffer_interface.h"
#include "sdv2_sd.h"
#include "sdv2_sdio.h"

wwd_result_t sdio_cmd52( wwd_bus_transfer_direction_t direction, uint32_t function, uint32_t address, uint8_t value, sdio_response_needed_t response_expected, uint8_t* response );
wwd_result_t sdio_cmd53( wwd_bus_transfer_direction_t direction, uint32_t function, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t address, uint16_t data_size, /*@in@*/ /*@out@*/ uint8_t* data, sdio_response_needed_t response_expected, /*@null@*/ uint32_t* response );

wwd_result_t host_platform_sdio_transfer( wwd_bus_transfer_direction_t direction, sdio_command_t command, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument, /*@null@*/ uint32_t* data, uint16_t data_size, sdio_response_needed_t response_expected, /*@out@*/ /*@null@*/ uint32_t* response )
{
	sdio_cmd_argument_t *arg;
  wwd_result_t result;
	uint32_t function, address;
	uint8_t	 value;
	arg = (sdio_cmd_argument_t *)&argument;

	switch(command){
		case SDIO_CMD_52:
			function = arg->cmd52.function_number;
			address = arg->cmd52.register_address;
			value = arg->cmd52.write_data;
      result = (wwd_result_t)sdio_cmd52( direction, function, address, value, response_expected, (uint8_t*)response);
			break;
		case SDIO_CMD_53:
			function = arg->cmd53.function_number;
			address = arg->cmd53.register_address;
      result = (wwd_result_t)sdio_cmd53( direction, function, ( data_size >= (uint16_t) 64 ) ? SDIO_BLOCK_MODE : SDIO_BYTE_MODE, block_size, address, data_size, (uint8_t *)data, response_expected, NULL );
			break;
		default:
			printf("Command %d not support\n", command);
			break;
	}

	return result;

}	

wwd_result_t host_platform_sdio_enumerate( void )
{

	if( sd_sdio_sd_identify())
		return WWD_TIMEOUT;
	return WWD_SUCCESS;

}	


wwd_result_t host_platform_enable_high_speed_sdio( void )
{
	return WWD_SUCCESS;
}

void host_platform_bus_buffer_freed( wwd_buffer_dir_t direction )
{
	UNUSED_PARAMETER( direction );
}

void host_platform_reset_wifi( wiced_bool_t reset_asserted )
{
	UNUSED_PARAMETER( reset_asserted );

}

void host_platform_power_wifi( wiced_bool_t power_enabled )
 {
	 UNUSED_PARAMETER( power_enabled );
 }

wwd_result_t host_platform_bus_enable_interrupt( void )
{
	sdio_enable_irq();
  return  WWD_SUCCESS;
}

wwd_result_t host_platform_bus_disable_interrupt( void )
{
	return  WWD_SUCCESS;
}

wwd_result_t host_platform_bus_init( void ){
	sdio_init(MS_SD_MODE);
	return  WWD_SUCCESS;
}



