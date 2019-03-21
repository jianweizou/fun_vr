#if defined(CONFIG_PIN_UART_0_TXD) && defined(CONFIG_PIN_CT32B7_PWM2) && (CONFIG_PIN_UART_0_TXD == CONFIG_PIN_CT32B7_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_UART_0_TXD and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_UART_0_TXD and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_UART_0_RXD) && defined(CONFIG_PIN_CT32B7_PWM1) && (CONFIG_PIN_UART_0_RXD == CONFIG_PIN_CT32B7_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_UART_0_RXD and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_UART_0_RXD and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_AIN01) && defined(CONFIG_PIN_CT32B7_PWM0) && (CONFIG_PIN_AIN01 == CONFIG_PIN_CT32B7_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_AIN01 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_AIN01 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_AIN02) && defined(CONFIG_PIN_CT32B6_PWM2) && (CONFIG_PIN_AIN02 == CONFIG_PIN_CT32B6_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_AIN02 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_AIN02 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SD_NF_POWER_ON) && defined(CONFIG_PIN_CT32B6_PWM1) && (CONFIG_PIN_SD_NF_POWER_ON == CONFIG_PIN_CT32B6_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD_NF_POWER_ON and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SD_NF_POWER_ON and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_AIN04) && defined(CONFIG_PIN_CT32B6_PWM0) && (CONFIG_PIN_AIN04 == CONFIG_PIN_CT32B6_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_AIN04 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_AIN04 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIFC_CS1) && defined(CONFIG_PIN_SPI0_DMA_CS) && (CONFIG_PIN_SPIFC_CS1 == CONFIG_PIN_SPI0_DMA_CS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIFC_CS1 and CONFIG_PIN_SPI0_DMA_CS conflict !"
	#else
		#error "CONFIG_PIN_SPIFC_CS1 and CONFIG_PIN_SPI0_DMA_CS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIFC_ED2_WP) && defined(CONFIG_PIN_CT32B0_PWM1) && (CONFIG_PIN_SPIFC_ED2_WP == CONFIG_PIN_CT32B0_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIFC_ED2_WP and CONFIG_PIN_CT32B0_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SPIFC_ED2_WP and CONFIG_PIN_CT32B0_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIFC_ED3) && defined(CONFIG_PIN_CT32B0_PWM0) && (CONFIG_PIN_SPIFC_ED3 == CONFIG_PIN_CT32B0_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIFC_ED3 and CONFIG_PIN_CT32B0_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_SPIFC_ED3 and CONFIG_PIN_CT32B0_PWM0 conflict !"
	#endif
#endif
#if defined(HOST_PPC) && defined(CLKOUT) && (HOST_PPC == CLKOUT)
	#if IO_SHARE_PIN
		#warning "HOST_PPC and CLKOUT conflict !"
	#else
		#error "HOST_PPC and CLKOUT conflict !"
	#endif
#endif
#if defined(HOST_PPC) && defined(CONFIG_PIN_SPIIF_0_CS) && (HOST_PPC == CONFIG_PIN_SPIIF_0_CS)
	#if IO_SHARE_PIN
		#warning "HOST_PPC and CONFIG_PIN_SPIIF_0_CS conflict !"
	#else
		#error "HOST_PPC and CONFIG_PIN_SPIIF_0_CS conflict !"
	#endif	
#endif
#if defined(CLKOUT) && defined(CONFIG_PIN_SPIIF_0_CS) && (CLKOUT == CONFIG_PIN_SPIIF_0_CS)
	#if IO_SHARE_PIN
		#warning "CLKOUT and CONFIG_PIN_SPIIF_0_CS conflict !"
	#else
		#error "CLKOUT and CONFIG_PIN_SPIIF_0_CS conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_NF_CS) && defined(CONFIG_PIN_SPIIF_0_SCK) && (CONFIG_PIN_NF_CS == CONFIG_PIN_SPIIF_0_SCK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_CS and CONFIG_PIN_SPIIF_0_SCK conflict !"
	#else
		#error "CONFIG_PIN_NF_CS and CONFIG_PIN_SPIIF_0_SCK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_RB) && defined(CONFIG_PIN_SPIIF_0_MISO) && (CONFIG_PIN_NF_RB == CONFIG_PIN_SPIIF_0_MISO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_RB and CONFIG_PIN_SPIIF_0_MISO conflict !"
	#else
		#error "CONFIG_PIN_NF_RB and CONFIG_PIN_SPIIF_0_MISO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_ALE) && defined(CONFIG_PIN_SPIIF_0_MOSI) && (CONFIG_PIN_NF_ALE == CONFIG_PIN_SPIIF_0_MOSI)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_ALE and CONFIG_PIN_SPIIF_0_MOSI conflict !"
	#else
		#error "CONFIG_PIN_NF_ALE and CONFIG_PIN_SPIIF_0_MOSI conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_CIS_VSYNC) && defined(CONFIG_PIN_NF_WE) && (CONFIG_PIN_CIS_VSYNC == CONFIG_PIN_NF_WE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_VSYNC and CONFIG_PIN_NF_WE conflict !"
	#else
		#error "CONFIG_PIN_CIS_VSYNC and CONFIG_PIN_NF_WE conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_VSYNC) && defined(CONFIG_PIN_HSYNC) && (CONFIG_PIN_CIS_VSYNC == CONFIG_PIN_HSYNC)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_VSYNC and CONFIG_PIN_HSYNC conflict !"
	#else
		#error "CONFIG_PIN_CIS_VSYNC and CONFIG_PIN_HSYNC conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_NF_WE) && defined(CONFIG_PIN_HSYNC) && (CONFIG_PIN_NF_WE == CONFIG_PIN_HSYNC)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WE and CONFIG_PIN_HSYNC conflict !"
	#else
		#error "CONFIG_PIN_NF_WE and CONFIG_PIN_HSYNC conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_CIS_HSYNC) && defined(CONFIG_PIN_NF_RE) && (CONFIG_PIN_CIS_HSYNC == CONFIG_PIN_NF_RE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_HSYNC and CONFIG_PIN_NF_RE conflict !"
	#else
		#error "CONFIG_PIN_CIS_HSYNC and CONFIG_PIN_NF_RE conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_HSYNC) && defined(CONFIG_PIN_VSYNC) && (CONFIG_PIN_CIS_HSYNC == CONFIG_PIN_VSYNC)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_HSYNC and CONFIG_PIN_VSYNC conflict !"
	#else
		#error "CONFIG_PIN_CIS_HSYNC and CONFIG_PIN_VSYNC conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_NF_RE) && defined(CONFIG_PIN_VSYNC) && (CONFIG_PIN_NF_RE == CONFIG_PIN_VSYNC)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_RE and CONFIG_PIN_VSYNC conflict !"
	#else
		#error "CONFIG_PIN_NF_RE and CONFIG_PIN_VSYNC conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CLK) && defined(CONFIG_PIN_CIS_MLCK) && (CONFIG_PIN_SD0_CLK == CONFIG_PIN_CIS_MLCK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CLK and CONFIG_PIN_CIS_MLCK conflict !"
	#else
		#error "CONFIG_PIN_SD0_CLK and CONFIG_PIN_CIS_MLCK conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SD0_CLK) && defined(CONFIG_PIN_NF_WP) && (CONFIG_PIN_SD0_CLK == CONFIG_PIN_NF_WP)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CLK and CONFIG_PIN_NF_WP conflict !"
	#else
		#error "CONFIG_PIN_SD0_CLK and CONFIG_PIN_NF_WP conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SD0_CLK) && defined(CONFIG_PIN_DE) && (CONFIG_PIN_SD0_CLK == CONFIG_PIN_DE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CLK and CONFIG_PIN_DE conflict !"
	#else
		#error "CONFIG_PIN_SD0_CLK and CONFIG_PIN_DE conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CLK) && defined(CONFIG_PIN_SPI0_DMA_CS) && (CONFIG_PIN_SD0_CLK == CONFIG_PIN_SPI0_DMA_CS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CLK and CONFIG_PIN_SPI0_DMA_CS conflict !"
	#else
		#error "CONFIG_PIN_SD0_CLK and CONFIG_PIN_SPI0_DMA_CS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CLK) && defined(CONFIG_PIN_OID_CLK) && (CONFIG_PIN_SD0_CLK == CONFIG_PIN_OID_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CLK and CONFIG_PIN_OID_CLK conflict !"
	#else
		#error "CONFIG_PIN_SD0_CLK and CONFIG_PIN_OID_CLK conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SD0_CLK) && defined(CONFIG_PIN_CT32B7_PWM2) && (CONFIG_PIN_SD0_CLK == CONFIG_PIN_CT32B7_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CLK and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_SD0_CLK and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_MLCK) && defined(CONFIG_PIN_NF_WP) && (CONFIG_PIN_CIS_MLCK == CONFIG_PIN_NF_WP)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_MLCK and CONFIG_PIN_NF_WP conflict !"
	#else
		#error "CONFIG_PIN_CIS_MLCK and CONFIG_PIN_NF_WP conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_MLCK) && defined(CONFIG_PIN_DE) && (CONFIG_PIN_CIS_MLCK == CONFIG_PIN_DE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_MLCK and CONFIG_PIN_DE conflict !"
	#else
		#error "CONFIG_PIN_CIS_MLCK and CONFIG_PIN_DE conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_CIS_MLCK) && defined(CONFIG_PIN_SPI0_DMA_CS) && (CONFIG_PIN_CIS_MLCK == CONFIG_PIN_SPI0_DMA_CS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_MLCK and CONFIG_PIN_SPI0_DMA_CS conflict !"
	#else
		#error "CONFIG_PIN_CIS_MLCK and CONFIG_PIN_SPI0_DMA_CS conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_CIS_MLCK) && defined(CONFIG_PIN_OID_CLK) && (CONFIG_PIN_CIS_MLCK == CONFIG_PIN_OID_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_MLCK and CONFIG_PIN_OID_CLK conflict !"
	#else
		#error "CONFIG_PIN_CIS_MLCK and CONFIG_PIN_OID_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_MLCK) && defined(CONFIG_PIN_CT32B7_PWM2) && (CONFIG_PIN_CIS_MLCK == CONFIG_PIN_CT32B7_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_MLCK and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_CIS_MLCK and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_WP) && defined(CONFIG_PIN_DE) && (CONFIG_PIN_NF_WP == CONFIG_PIN_DE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WP and CONFIG_PIN_DE conflict !"
	#else
		#error "CONFIG_PIN_NF_WP and CONFIG_PIN_DE conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_WP) && defined(CONFIG_PIN_SPI0_DMA_CS) && (CONFIG_PIN_NF_WP == CONFIG_PIN_SPI0_DMA_CS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WP and CONFIG_PIN_SPI0_DMA_CS conflict !"
	#else
		#error "CONFIG_PIN_NF_WP and CONFIG_PIN_SPI0_DMA_CS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_WP) && defined(CONFIG_PIN_OID_CLK) && (CONFIG_PIN_NF_WP == CONFIG_PIN_OID_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WP and CONFIG_PIN_OID_CLK conflict !"
	#else
		#error "CONFIG_PIN_NF_WP and CONFIG_PIN_OID_CLK conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_NF_WP) && defined(CONFIG_PIN_CT32B7_PWM2) && (CONFIG_PIN_NF_WP == CONFIG_PIN_CT32B7_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WP and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_NF_WP and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DE) && defined(CONFIG_PIN_SPI0_DMA_CS) && (CONFIG_PIN_DE == CONFIG_PIN_SPI0_DMA_CS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DE and CONFIG_PIN_SPI0_DMA_CS conflict !"
	#else
		#error "CONFIG_PIN_DE and CONFIG_PIN_SPI0_DMA_CS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DE) && defined(CONFIG_PIN_OID_CLK) && (CONFIG_PIN_DE == CONFIG_PIN_OID_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DE and CONFIG_PIN_OID_CLK conflict !"
	#else
		#error "CONFIG_PIN_DE and CONFIG_PIN_OID_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DE) && defined(CONFIG_PIN_CT32B7_PWM2) && (CONFIG_PIN_DE == CONFIG_PIN_CT32B7_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DE and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_DE and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_CS) && defined(CONFIG_PIN_OID_CLK) && (CONFIG_PIN_SPI0_DMA_CS == CONFIG_PIN_OID_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_CS and CONFIG_PIN_OID_CLK conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_CS and CONFIG_PIN_OID_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_CS) && defined(CONFIG_PIN_CT32B7_PWM2) && (CONFIG_PIN_SPI0_DMA_CS == CONFIG_PIN_CT32B7_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_CS and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_CS and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_OID_CLK) && defined(CONFIG_PIN_CT32B7_PWM2) && (CONFIG_PIN_OID_CLK == CONFIG_PIN_CT32B7_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_CLK and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_OID_CLK and CONFIG_PIN_CT32B7_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CMD) && defined(CONFIG_PIN_CIS_PCLK) && (CONFIG_PIN_SD0_CMD == CONFIG_PIN_CIS_PCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CMD and CONFIG_PIN_CIS_PCLK conflict !"
	#else
		#error "CONFIG_PIN_SD0_CMD and CONFIG_PIN_CIS_PCLK conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SD0_CMD) && defined(CONFIG_PIN_NF_CLE) && (CONFIG_PIN_SD0_CMD == CONFIG_PIN_NF_CLE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CMD and CONFIG_PIN_NF_CLE conflict !"
	#else
		#error "CONFIG_PIN_SD0_CMD and CONFIG_PIN_NF_CLE conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CMD) && defined(CONFIG_PIN_DCLK) && (CONFIG_PIN_SD0_CMD == CONFIG_PIN_DCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CMD and CONFIG_PIN_DCLK conflict !"
	#else
		#error "CONFIG_PIN_SD0_CMD and CONFIG_PIN_DCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CMD) && defined(CONFIG_PIN_SPI0_DMA_SCK) && (CONFIG_PIN_SD0_CMD == CONFIG_PIN_SPI0_DMA_SCK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CMD and CONFIG_PIN_SPI0_DMA_SCK conflict !"
	#else
		#error "CONFIG_PIN_SD0_CMD and CONFIG_PIN_SPI0_DMA_SCK conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SD0_CMD) && defined(CONFIG_PIN_OID_CMD) && (CONFIG_PIN_SD0_CMD == CONFIG_PIN_OID_CMD)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CMD and CONFIG_PIN_OID_CMD conflict !"
	#else
		#error "CONFIG_PIN_SD0_CMD and CONFIG_PIN_OID_CMD conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SD0_CMD) && defined(CONFIG_PIN_CT32B7_PWM1) && (CONFIG_PIN_SD0_CMD == CONFIG_PIN_CT32B7_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CMD and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SD0_CMD and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_PCLK) && defined(CONFIG_PIN_NF_CLE) && (CONFIG_PIN_CIS_PCLK == CONFIG_PIN_NF_CLE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_PCLK and CONFIG_PIN_NF_CLE conflict !"
	#else
		#error "CONFIG_PIN_CIS_PCLK and CONFIG_PIN_NF_CLE conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_PCLK) && defined(CONFIG_PIN_DCLK) && (CONFIG_PIN_CIS_PCLK == CONFIG_PIN_DCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_PCLK and CONFIG_PIN_DCLK conflict !"
	#else
		#error "CONFIG_PIN_CIS_PCLK and CONFIG_PIN_DCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_PCLK) && defined(CONFIG_PIN_SPI0_DMA_SCK) && (CONFIG_PIN_CIS_PCLK == CONFIG_PIN_SPI0_DMA_SCK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_PCLK and CONFIG_PIN_SPI0_DMA_SCK conflict !"
	#else
		#error "CONFIG_PIN_CIS_PCLK and CONFIG_PIN_SPI0_DMA_SCK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_PCLK) && defined(CONFIG_PIN_OID_CMD) && (CONFIG_PIN_CIS_PCLK == CONFIG_PIN_OID_CMD)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_PCLK and CONFIG_PIN_OID_CMD conflict !"
	#else
		#error "CONFIG_PIN_CIS_PCLK and CONFIG_PIN_OID_CMD conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_PCLK) && defined(CONFIG_PIN_CT32B7_PWM1) && (CONFIG_PIN_CIS_PCLK == CONFIG_PIN_CT32B7_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_PCLK and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_CIS_PCLK and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_CLE) && defined(CONFIG_PIN_DCLK) && (CONFIG_PIN_NF_CLE == CONFIG_PIN_DCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_CLE and CONFIG_PIN_DCLK conflict !"
	#else
		#error "CONFIG_PIN_NF_CLE and CONFIG_PIN_DCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_CLE) && defined(CONFIG_PIN_SPI0_DMA_SCK) && (CONFIG_PIN_NF_CLE == CONFIG_PIN_SPI0_DMA_SCK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_CLE and CONFIG_PIN_SPI0_DMA_SCK conflict !"
	#else
		#error "CONFIG_PIN_NF_CLE and CONFIG_PIN_SPI0_DMA_SCK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_CLE) && defined(CONFIG_PIN_OID_CMD) && (CONFIG_PIN_NF_CLE == CONFIG_PIN_OID_CMD)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_CLE and CONFIG_PIN_OID_CMD conflict !"
	#else
		#error "CONFIG_PIN_NF_CLE and CONFIG_PIN_OID_CMD conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_CLE) && defined(CONFIG_PIN_CT32B7_PWM1) && (CONFIG_PIN_NF_CLE == CONFIG_PIN_CT32B7_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_CLE and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_NF_CLE and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DCLK) && defined(CONFIG_PIN_SPI0_DMA_SCK) && (CONFIG_PIN_DCLK == CONFIG_PIN_SPI0_DMA_SCK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DCLK and CONFIG_PIN_SPI0_DMA_SCK conflict !"
	#else
		#error "CONFIG_PIN_DCLK and CONFIG_PIN_SPI0_DMA_SCK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DCLK) && defined(CONFIG_PIN_OID_CMD) && (CONFIG_PIN_DCLK == CONFIG_PIN_OID_CMD)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DCLK and CONFIG_PIN_OID_CMD conflict !"
	#else
		#error "CONFIG_PIN_DCLK and CONFIG_PIN_OID_CMD conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DCLK) && defined(CONFIG_PIN_CT32B7_PWM1) && (CONFIG_PIN_DCLK == CONFIG_PIN_CT32B7_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DCLK and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_DCLK and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_SCK) && defined(CONFIG_PIN_OID_CMD) && (CONFIG_PIN_SPI0_DMA_SCK == CONFIG_PIN_OID_CMD)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_SCK and CONFIG_PIN_OID_CMD conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_SCK and CONFIG_PIN_OID_CMD conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_SCK) && defined(CONFIG_PIN_CT32B7_PWM1) && (CONFIG_PIN_SPI0_DMA_SCK == CONFIG_PIN_CT32B7_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_SCK and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_SCK and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_OID_CMD) && defined(CONFIG_PIN_CT32B7_PWM1) && (CONFIG_PIN_OID_CMD == CONFIG_PIN_CT32B7_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_CMD and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_OID_CMD and CONFIG_PIN_CT32B7_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D0) && defined(CONFIG_PIN_CIS_D0) && (CONFIG_PIN_SD0_D0 == CONFIG_PIN_CIS_D0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D0 and CONFIG_PIN_CIS_D0 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D0 and CONFIG_PIN_CIS_D0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D0) && defined(CONFIG_PIN_NF_D0) && (CONFIG_PIN_SD0_D0 == CONFIG_PIN_NF_D0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D0 and CONFIG_PIN_NF_D0 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D0 and CONFIG_PIN_NF_D0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D0) && defined(CONFIG_PIN_ED0) && (CONFIG_PIN_SD0_D0 == CONFIG_PIN_ED0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D0 and CONFIG_PIN_ED0 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D0 and CONFIG_PIN_ED0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D0) && defined(CONFIG_PIN_SPI0_DMA_SO) && (CONFIG_PIN_SD0_D0 == CONFIG_PIN_SPI0_DMA_SO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D0 and CONFIG_PIN_SPI0_DMA_SO conflict !"
	#else
		#error "CONFIG_PIN_SD0_D0 and CONFIG_PIN_SPI0_DMA_SO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D0) && defined(CONFIG_PIN_OID_DATA_0) && (CONFIG_PIN_SD0_D0 == CONFIG_PIN_OID_DATA_0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D0 and CONFIG_PIN_OID_DATA_0 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D0 and CONFIG_PIN_OID_DATA_0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D0) && defined(CONFIG_PIN_CT32B7_PWM0) && (CONFIG_PIN_SD0_D0 == CONFIG_PIN_CT32B7_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D0 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D0 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D0) && defined(CONFIG_PIN_NF_D0) && (CONFIG_PIN_CIS_D0 == CONFIG_PIN_NF_D0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D0 and CONFIG_PIN_NF_D0 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D0 and CONFIG_PIN_NF_D0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D0) && defined(CONFIG_PIN_ED0) && (CONFIG_PIN_CIS_D0 == CONFIG_PIN_ED0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D0 and CONFIG_PIN_ED0 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D0 and CONFIG_PIN_ED0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D0) && defined(CONFIG_PIN_SPI0_DMA_SO) && (CONFIG_PIN_CIS_D0 == CONFIG_PIN_SPI0_DMA_SO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D0 and CONFIG_PIN_SPI0_DMA_SO conflict !"
	#else
		#error "CONFIG_PIN_CIS_D0 and CONFIG_PIN_SPI0_DMA_SO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D0) && defined(CONFIG_PIN_OID_DATA_0) && (CONFIG_PIN_CIS_D0 == CONFIG_PIN_OID_DATA_0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D0 and CONFIG_PIN_OID_DATA_0 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D0 and CONFIG_PIN_OID_DATA_0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D0) && defined(CONFIG_PIN_CT32B7_PWM0) && (CONFIG_PIN_CIS_D0 == CONFIG_PIN_CT32B7_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D0 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D0 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D0) && defined(CONFIG_PIN_ED0) && (CONFIG_PIN_NF_D0 == CONFIG_PIN_ED0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D0 and CONFIG_PIN_ED0 conflict !"
	#else
		#error "CONFIG_PIN_NF_D0 and CONFIG_PIN_ED0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D0) && defined(CONFIG_PIN_SPI0_DMA_SO) && (CONFIG_PIN_NF_D0 == CONFIG_PIN_SPI0_DMA_SO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D0 and CONFIG_PIN_SPI0_DMA_SO conflict !"
	#else
		#error "CONFIG_PIN_NF_D0 and CONFIG_PIN_SPI0_DMA_SO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D0) && defined(CONFIG_PIN_OID_DATA_0) && (CONFIG_PIN_NF_D0 == CONFIG_PIN_OID_DATA_0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D0 and CONFIG_PIN_OID_DATA_0 conflict !"
	#else
		#error "CONFIG_PIN_NF_D0 and CONFIG_PIN_OID_DATA_0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D0) && defined(CONFIG_PIN_CT32B7_PWM0) && (CONFIG_PIN_NF_D0 == CONFIG_PIN_CT32B7_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D0 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_NF_D0 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED0) && defined(CONFIG_PIN_SPI0_DMA_SO) && (CONFIG_PIN_ED0 == CONFIG_PIN_SPI0_DMA_SO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED0 and CONFIG_PIN_SPI0_DMA_SO conflict !"
	#else
		#error "CONFIG_PIN_ED0 and CONFIG_PIN_SPI0_DMA_SO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED0) && defined(CONFIG_PIN_OID_DATA_0) && (CONFIG_PIN_ED0 == CONFIG_PIN_OID_DATA_0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED0 and CONFIG_PIN_OID_DATA_0 conflict !"
	#else
		#error "CONFIG_PIN_ED0 and CONFIG_PIN_OID_DATA_0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED0) && defined(CONFIG_PIN_CT32B7_PWM0) && (CONFIG_PIN_ED0 == CONFIG_PIN_CT32B7_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED0 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_ED0 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_SO) && defined(CONFIG_PIN_OID_DATA_0) && (CONFIG_PIN_SPI0_DMA_SO == CONFIG_PIN_OID_DATA_0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_SO and CONFIG_PIN_OID_DATA_0 conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_SO and CONFIG_PIN_OID_DATA_0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_SO) && defined(CONFIG_PIN_CT32B7_PWM0) && (CONFIG_PIN_SPI0_DMA_SO == CONFIG_PIN_CT32B7_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_SO and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_SO and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_OID_DATA_0) && defined(CONFIG_PIN_CT32B7_PWM0) && (CONFIG_PIN_OID_DATA_0 == CONFIG_PIN_CT32B7_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_DATA_0 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_OID_DATA_0 and CONFIG_PIN_CT32B7_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D1) && defined(CONFIG_PIN_CIS_D1) && (CONFIG_PIN_SD0_D1 == CONFIG_PIN_CIS_D1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D1 and CONFIG_PIN_CIS_D1 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D1 and CONFIG_PIN_CIS_D1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D1) && defined(CONFIG_PIN_NF_D1) && (CONFIG_PIN_SD0_D1 == CONFIG_PIN_NF_D1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D1 and CONFIG_PIN_NF_D1 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D1 and CONFIG_PIN_NF_D1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D1) && defined(CONFIG_PIN_ED1) && (CONFIG_PIN_SD0_D1 == CONFIG_PIN_ED1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D1 and CONFIG_PIN_ED1 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D1 and CONFIG_PIN_ED1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D1) && defined(CONFIG_PIN_SPI0_DMA_SI) && (CONFIG_PIN_SD0_D1 == CONFIG_PIN_SPI0_DMA_SI)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D1 and CONFIG_PIN_SPI0_DMA_SI conflict !"
	#else
		#error "CONFIG_PIN_SD0_D1 and CONFIG_PIN_SPI0_DMA_SI conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D1) && defined(CONFIG_PIN_OID_DATA_1) && (CONFIG_PIN_SD0_D1 == CONFIG_PIN_OID_DATA_1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D1 and CONFIG_PIN_OID_DATA_1 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D1 and CONFIG_PIN_OID_DATA_1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D1) && defined(CONFIG_PIN_CT32B6_PWM2) && (CONFIG_PIN_SD0_D1 == CONFIG_PIN_CT32B6_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D1 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D1 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D1) && defined(CONFIG_PIN_NF_D1) && (CONFIG_PIN_CIS_D1 == CONFIG_PIN_NF_D1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D1 and CONFIG_PIN_NF_D1 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D1 and CONFIG_PIN_NF_D1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D1) && defined(CONFIG_PIN_ED1) && (CONFIG_PIN_CIS_D1 == CONFIG_PIN_ED1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D1 and CONFIG_PIN_ED1 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D1 and CONFIG_PIN_ED1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D1) && defined(CONFIG_PIN_SPI0_DMA_SI) && (CONFIG_PIN_CIS_D1 == CONFIG_PIN_SPI0_DMA_SI)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D1 and CONFIG_PIN_SPI0_DMA_SI conflict !"
	#else
		#error "CONFIG_PIN_CIS_D1 and CONFIG_PIN_SPI0_DMA_SI conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D1) && defined(CONFIG_PIN_OID_DATA_1) && (CONFIG_PIN_CIS_D1 == CONFIG_PIN_OID_DATA_1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D1 and CONFIG_PIN_OID_DATA_1 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D1 and CONFIG_PIN_OID_DATA_1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D1) && defined(CONFIG_PIN_CT32B6_PWM2) && (CONFIG_PIN_CIS_D1 == CONFIG_PIN_CT32B6_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D1 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D1 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D1) && defined(CONFIG_PIN_ED1) && (CONFIG_PIN_NF_D1 == CONFIG_PIN_ED1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D1 and CONFIG_PIN_ED1 conflict !"
	#else
		#error "CONFIG_PIN_NF_D1 and CONFIG_PIN_ED1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D1) && defined(CONFIG_PIN_SPI0_DMA_SI) && (CONFIG_PIN_NF_D1 == CONFIG_PIN_SPI0_DMA_SI)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D1 and CONFIG_PIN_SPI0_DMA_SI conflict !"
	#else
		#error "CONFIG_PIN_NF_D1 and CONFIG_PIN_SPI0_DMA_SI conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D1) && defined(CONFIG_PIN_OID_DATA_1) && (CONFIG_PIN_NF_D1 == CONFIG_PIN_OID_DATA_1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D1 and CONFIG_PIN_OID_DATA_1 conflict !"
	#else
		#error "CONFIG_PIN_NF_D1 and CONFIG_PIN_OID_DATA_1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D1) && defined(CONFIG_PIN_CT32B6_PWM2) && (CONFIG_PIN_NF_D1 == CONFIG_PIN_CT32B6_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D1 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_NF_D1 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED1) && defined(CONFIG_PIN_SPI0_DMA_SI) && (CONFIG_PIN_ED1 == CONFIG_PIN_SPI0_DMA_SI)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED1 and CONFIG_PIN_SPI0_DMA_SI conflict !"
	#else
		#error "CONFIG_PIN_ED1 and CONFIG_PIN_SPI0_DMA_SI conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED1) && defined(CONFIG_PIN_OID_DATA_1) && (CONFIG_PIN_ED1 == CONFIG_PIN_OID_DATA_1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED1 and CONFIG_PIN_OID_DATA_1 conflict !"
	#else
		#error "CONFIG_PIN_ED1 and CONFIG_PIN_OID_DATA_1 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_ED1) && defined(CONFIG_PIN_CT32B6_PWM2) && (CONFIG_PIN_ED1 == CONFIG_PIN_CT32B6_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED1 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_ED1 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_SI) && defined(CONFIG_PIN_OID_DATA_1) && (CONFIG_PIN_SPI0_DMA_SI == CONFIG_PIN_OID_DATA_1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_SI and CONFIG_PIN_OID_DATA_1 conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_SI and CONFIG_PIN_OID_DATA_1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_SI) && defined(CONFIG_PIN_CT32B6_PWM2) && (CONFIG_PIN_SPI0_DMA_SI == CONFIG_PIN_CT32B6_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_SI and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_SI and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_OID_DATA_1) && defined(CONFIG_PIN_CT32B6_PWM2) && (CONFIG_PIN_OID_DATA_1 == CONFIG_PIN_CT32B6_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_DATA_1 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_OID_DATA_1 and CONFIG_PIN_CT32B6_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D2) && defined(CONFIG_PIN_CIS_D2) && (CONFIG_PIN_SD0_D2 == CONFIG_PIN_CIS_D2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D2 and CONFIG_PIN_CIS_D2 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D2 and CONFIG_PIN_CIS_D2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D2) && defined(CONFIG_PIN_NF_D2) && (CONFIG_PIN_SD0_D2 == CONFIG_PIN_NF_D2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D2 and CONFIG_PIN_NF_D2 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D2 and CONFIG_PIN_NF_D2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D2) && defined(CONFIG_PIN_ED2) && (CONFIG_PIN_SD0_D2 == CONFIG_PIN_ED2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D2 and CONFIG_PIN_ED2 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D2 and CONFIG_PIN_ED2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D2) && defined(CONFIG_PIN_SPI0_DMA_SO2) && (CONFIG_PIN_SD0_D2 == CONFIG_PIN_SPI0_DMA_SO2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D2 and CONFIG_PIN_SPI0_DMA_SO2 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D2 and CONFIG_PIN_SPI0_DMA_SO2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D2) && defined(CONFIG_PIN_OID_2WIRE_CLK) && (CONFIG_PIN_SD0_D2 == CONFIG_PIN_OID_2WIRE_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D2 and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#else
		#error "CONFIG_PIN_SD0_D2 and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D2) && defined(CONFIG_PIN_CT32B6_PWM1) && (CONFIG_PIN_SD0_D2 == CONFIG_PIN_CT32B6_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D2 and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D2 and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D2) && defined(CONFIG_PIN_NF_D2) && (CONFIG_PIN_CIS_D2 == CONFIG_PIN_NF_D2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D2 and CONFIG_PIN_NF_D2 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D2 and CONFIG_PIN_NF_D2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D2) && defined(CONFIG_PIN_ED2) && (CONFIG_PIN_CIS_D2 == CONFIG_PIN_ED2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D2 and CONFIG_PIN_NF_D2 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D2 and CONFIG_PIN_ED2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D2) && defined(CONFIG_PIN_SPI0_DMA_SO2) && (CONFIG_PIN_CIS_D2 == CONFIG_PIN_SPI0_DMA_SO2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D2 and CONFIG_PIN_SPI0_DMA_SO2 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D2 and CONFIG_PIN_SPI0_DMA_SO2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D2) && defined(CONFIG_PIN_OID_2WIRE_CLK) && (CONFIG_PIN_CIS_D2 == CONFIG_PIN_OID_2WIRE_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D2 and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#else
		#error "CONFIG_PIN_CIS_D2 and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D2) && defined(CONFIG_PIN_CT32B6_PWM1) && (CONFIG_PIN_CIS_D2 == CONFIG_PIN_CT32B6_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D2 and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D2 and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D2) && defined(CONFIG_PIN_ED2) && (CONFIG_PIN_NF_D2 == CONFIG_PIN_ED2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D2 and CONFIG_PIN_ED2 conflict !"
	#else
		#error "CONFIG_PIN_NF_D2 and CONFIG_PIN_ED2 conflict !"
	#endif			
#endif
#if defined(CONFIG_PIN_NF_D2) && defined(CONFIG_PIN_SPI0_DMA_SO2) && (CONFIG_PIN_NF_D2 == CONFIG_PIN_SPI0_DMA_SO2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D2 and CONFIG_PIN_SPI0_DMA_SO2 conflict !"
	#else
		#error "CONFIG_PIN_NF_D2 and CONFIG_PIN_SPI0_DMA_SO2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D2) && defined(CONFIG_PIN_OID_2WIRE_CLK) && (CONFIG_PIN_NF_D2 == CONFIG_PIN_OID_2WIRE_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D2 and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#else
		#error "CONFIG_PIN_NF_D2 and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D2) && defined(CONFIG_PIN_CT32B6_PWM1) && (CONFIG_PIN_NF_D2 == CONFIG_PIN_CT32B6_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D2 and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_NF_D2 and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED2) && defined(CONFIG_PIN_SPI0_DMA_SO2) && (CONFIG_PIN_ED2 == CONFIG_PIN_SPI0_DMA_SO2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED2 and CONFIG_PIN_SPI0_DMA_SO2 conflict !"
	#else
		#error "CONFIG_PIN_ED2 and CONFIG_PIN_SPI0_DMA_SO2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED2) && defined(CONFIG_PIN_OID_2WIRE_CLK) && (CONFIG_PIN_ED2 == CONFIG_PIN_OID_2WIRE_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED2 and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#else
		#error "CONFIG_PIN_ED2 and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED2) && defined(CONFIG_PIN_CT32B6_PWM1) && (CONFIG_PIN_ED2 == CONFIG_PIN_CT32B6_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED2 and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_ED2 and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_SO2) && defined(CONFIG_PIN_OID_2WIRE_CLK) && (CONFIG_PIN_SPI0_DMA_SO2 == CONFIG_PIN_OID_2WIRE_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_SO2 and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_SO2 and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_SO2) && defined(CONFIG_PIN_CT32B6_PWM1) && (CONFIG_PIN_SPI0_DMA_SO2 == CONFIG_PIN_CT32B6_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_SO2 and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_SO2 and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_OID_2WIRE_CLK) && defined(CONFIG_PIN_CT32B6_PWM1) && (CONFIG_PIN_OID_2WIRE_CLK == CONFIG_PIN_CT32B6_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_2WIRE_CLK and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_OID_2WIRE_CLK and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D3) && defined(CONFIG_PIN_CIS_D3) && (CONFIG_PIN_SD0_D3 == CONFIG_PIN_CIS_D3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_2WIRE_CLK and CONFIG_PIN_CT32B6_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D3 and CONFIG_PIN_CIS_D3 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D3) && defined(CONFIG_PIN_NF_D3) && (CONFIG_PIN_SD0_D3 == CONFIG_PIN_NF_D3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D3 and CONFIG_PIN_NF_D3 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D3 and CONFIG_PIN_NF_D3 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D3) && defined(CONFIG_PIN_ED3) && (CONFIG_PIN_SD0_D3 == CONFIG_PIN_ED3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D3 and CONFIG_PIN_ED3 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D3 and CONFIG_PIN_ED3 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D3) && defined(CONFIG_PIN_SPI0_DMA_SO3) && (CONFIG_PIN_SD0_D3 == CONFIG_PIN_SPI0_DMA_SO3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D3 and CONFIG_PIN_SPI0_DMA_SO3 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D3 and CONFIG_PIN_SPI0_DMA_SO3 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D3) && defined(CONFIG_PIN_OID_2WIRE_DATA) && (CONFIG_PIN_SD0_D3 == CONFIG_PIN_OID_2WIRE_DATA)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D3 and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#else
		#error "CONFIG_PIN_SD0_D3 and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D3) && defined(CONFIG_PIN_CT32B6_PWM0) && (CONFIG_PIN_SD0_D3 == CONFIG_PIN_CT32B6_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D3 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D3 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D3) && defined(CONFIG_PIN_NF_D3) && (CONFIG_PIN_CIS_D3 == CONFIG_PIN_NF_D3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D3 and CONFIG_PIN_NF_D3 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D3 and CONFIG_PIN_NF_D3 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D3) && defined(CONFIG_PIN_ED3) && (CONFIG_PIN_CIS_D3 == CONFIG_PIN_ED3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D3 and CONFIG_PIN_ED3 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D3 and CONFIG_PIN_ED3 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D3) && defined(CONFIG_PIN_SPI0_DMA_SO3) && (CONFIG_PIN_CIS_D3 == CONFIG_PIN_SPI0_DMA_SO3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D3 and CONFIG_PIN_SPI0_DMA_SO3 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D3 and CONFIG_PIN_SPI0_DMA_SO3 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D3) && defined(CONFIG_PIN_OID_2WIRE_DATA) && (CONFIG_PIN_CIS_D3 == CONFIG_PIN_OID_2WIRE_DATA)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D3 and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#else
		#error "CONFIG_PIN_CIS_D3 and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D3) && defined(CONFIG_PIN_CT32B6_PWM0) && (CONFIG_PIN_CIS_D3 == CONFIG_PIN_CT32B6_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D3 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D3 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D3) && defined(CONFIG_PIN_ED3) && (CONFIG_PIN_NF_D3 == CONFIG_PIN_ED3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D3 and CONFIG_PIN_ED3 conflict !"
	#else
		#error "CONFIG_PIN_NF_D3 and CONFIG_PIN_ED3 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_NF_D3) && defined(CONFIG_PIN_SPI0_DMA_SO3) && (CONFIG_PIN_NF_D3 == CONFIG_PIN_SPI0_DMA_SO3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D3 and CONFIG_PIN_SPI0_DMA_SO3 conflict !"
	#else
		#error "CONFIG_PIN_NF_D3 and CONFIG_PIN_SPI0_DMA_SO3 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D3) && defined(CONFIG_PIN_OID_2WIRE_DATA) && (CONFIG_PIN_NF_D3 == CONFIG_PIN_OID_2WIRE_DATA)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D3 and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#else
		#error "CONFIG_PIN_NF_D3 and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D3) && defined(CONFIG_PIN_CT32B6_PWM0) && (CONFIG_PIN_NF_D3 == CONFIG_PIN_CT32B6_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D3 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_NF_D3 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED3) && defined(CONFIG_PIN_SPI0_DMA_SO3) && (CONFIG_PIN_ED3 == CONFIG_PIN_SPI0_DMA_SO3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED3 and CONFIG_PIN_SPI0_DMA_SO3 conflict !"
	#else
		#error "CONFIG_PIN_ED3 and CONFIG_PIN_SPI0_DMA_SO3 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_ED3) && defined(CONFIG_PIN_OID_2WIRE_DATA) && (CONFIG_PIN_ED3 == CONFIG_PIN_OID_2WIRE_DATA)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED3 and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#else
		#error "CONFIG_PIN_ED3 and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED3) && defined(CONFIG_PIN_CT32B6_PWM0) && (CONFIG_PIN_ED3 == CONFIG_PIN_CT32B6_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED3 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_ED3 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_SO3) && defined(CONFIG_PIN_OID_2WIRE_DATA) && (CONFIG_PIN_SPI0_DMA_SO3 == CONFIG_PIN_OID_2WIRE_DATA)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_SO3 and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_SO3 and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI0_DMA_SO3) && defined(CONFIG_PIN_CT32B6_PWM0) && (CONFIG_PIN_SPI0_DMA_SO3 == CONFIG_PIN_CT32B6_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI0_DMA_SO3 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_SPI0_DMA_SO3 and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_OID_2WIRE_DATA) && defined(CONFIG_PIN_CT32B6_PWM0) && (CONFIG_PIN_OID_2WIRE_DATA == CONFIG_PIN_CT32B6_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_2WIRE_DATA and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_OID_2WIRE_DATA and CONFIG_PIN_CT32B6_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D4) && defined(CONFIG_PIN_NF_D4) && (CONFIG_PIN_CIS_D4 == CONFIG_PIN_NF_D4)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D4 and CONFIG_PIN_NF_D4 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D4 and CONFIG_PIN_NF_D4 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D4) && defined(CONFIG_PIN_ED4) && (CONFIG_PIN_CIS_D4 == CONFIG_PIN_ED4)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D4 and CONFIG_PIN_ED4 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D4 and CONFIG_PIN_ED4 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D4) && defined(CONFIG_PIN_ED4) && (CONFIG_PIN_NF_D4 == CONFIG_PIN_ED4)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D4 and CONFIG_PIN_ED4 conflict !"
	#else
		#error "CONFIG_PIN_NF_D4 and CONFIG_PIN_ED4 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D5) && defined(CONFIG_PIN_NF_D5) && (CONFIG_PIN_CIS_D5 == CONFIG_PIN_NF_D5)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D5 and CONFIG_PIN_NF_D5 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D5 and CONFIG_PIN_NF_D5 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_CIS_D5) && defined(CONFIG_PIN_ED5) && (CONFIG_PIN_CIS_D5 == CONFIG_PIN_ED5)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D5 and CONFIG_PIN_ED5 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D5 and CONFIG_PIN_ED5 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D5) && defined(CONFIG_PIN_ED5) && (CONFIG_PIN_NF_D5 == CONFIG_PIN_ED5)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D5 and CONFIG_PIN_ED5 conflict !"
	#else
		#error "CONFIG_PIN_NF_D5 and CONFIG_PIN_ED5 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D6) && defined(CONFIG_PIN_NF_D6) && (CONFIG_PIN_CIS_D6 == CONFIG_PIN_NF_D6)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D6 and CONFIG_PIN_NF_D6 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D6 and CONFIG_PIN_NF_D6 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D6) && defined(CONFIG_PIN_ED6) && (CONFIG_PIN_CIS_D6 == CONFIG_PIN_ED6)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D6 and CONFIG_PIN_ED6 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D6 and CONFIG_PIN_ED6 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D6) && defined(CONFIG_PIN_ED6) && (CONFIG_PIN_NF_D6 == CONFIG_PIN_ED6)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D6 and CONFIG_PIN_ED6 conflict !"
	#else
		#error "CONFIG_PIN_NF_D6 and CONFIG_PIN_ED6 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D7) && defined(CONFIG_PIN_NF_D7) && (CONFIG_PIN_CIS_D7 == CONFIG_PIN_NF_D7)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D7 and CONFIG_PIN_NF_D7 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D7 and CONFIG_PIN_NF_D7 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CIS_D7) && defined(CONFIG_PIN_ED7) && (CONFIG_PIN_CIS_D7 == CONFIG_PIN_ED7)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CIS_D7 and CONFIG_PIN_ED7 conflict !"
	#else
		#error "CONFIG_PIN_CIS_D7 and CONFIG_PIN_ED7 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D7) && defined(CONFIG_PIN_ED7) && (CONFIG_PIN_NF_D7 == CONFIG_PIN_ED7)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D7 and CONFIG_PIN_ED7 conflict !"
	#else
		#error "CONFIG_PIN_NF_D7 and CONFIG_PIN_ED7 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD1_CLK) && defined(CONFIG_PIN_I2S_4_MCLK) && (CONFIG_PIN_SD1_CLK == CONFIG_PIN_I2S_4_MCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD1_CLK and CONFIG_PIN_I2S_4_MCLK conflict !"
	#else
		#error "CONFIG_PIN_SD1_CLK and CONFIG_PIN_I2S_4_MCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD1_CMD) && defined(CONFIG_PIN_I2S_4_SDIN) && (CONFIG_PIN_SD1_CMD == CONFIG_PIN_I2S_4_SDIN)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD1_CMD and CONFIG_PIN_I2S_4_SDIN conflict !"
	#else
		#error "CONFIG_PIN_SD1_CMD and CONFIG_PIN_I2S_4_SDIN conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD1_D0) && defined(CONFIG_PIN_I2S_4_SDOUT) && (CONFIG_PIN_SD1_D0 == CONFIG_PIN_I2S_4_SDOUT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD1_D0 and CONFIG_PIN_I2S_4_SDOUT conflict !"
	#else
		#error "CONFIG_PIN_SD1_D0 and CONFIG_PIN_I2S_4_SDOUT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD1_D1) && defined(CONFIG_PIN_I2S_4_BCLK) && (CONFIG_PIN_SD1_D1 == CONFIG_PIN_I2S_4_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD1_D1 and CONFIG_PIN_I2S_4_BCLK conflict !"
	#else
		#error "CONFIG_PIN_SD1_D1 and CONFIG_PIN_I2S_4_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD1_D2) && defined(CONFIG_PIN_I2S_4_WS) && (CONFIG_PIN_SD1_D2 == CONFIG_PIN_I2S_4_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD1_D2 and CONFIG_PIN_I2S_4_WS conflict !"
	#else
		#error "CONFIG_PIN_SD1_D2 and CONFIG_PIN_I2S_4_WS conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_HSYNC) && defined(CONFIG_PIN_I2S_4_MCLK) && (CONFIG_PIN_HSYNC == CONFIG_PIN_I2S_4_MCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_HSYNC and CONFIG_PIN_I2S_4_MCLK conflict !"
	#else
		#error "CONFIG_PIN_HSYNC and CONFIG_PIN_I2S_4_MCLK conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_HSYNC) && defined(CONFIG_PIN_OID_CLK) && (CONFIG_PIN_HSYNC == CONFIG_PIN_OID_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_HSYNC and CONFIG_PIN_OID_CLK conflict !"
	#else
		#error "CONFIG_PIN_HSYNC and CONFIG_PIN_OID_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_HSYNC) && defined(CONFIG_PIN_CT32B5_PWM2) && (CONFIG_PIN_HSYNC == CONFIG_PIN_CT32B5_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_HSYNC and CONFIG_PIN_CT32B5_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_HSYNC and CONFIG_PIN_CT32B5_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_MCLK) && defined(CONFIG_PIN_OID_CLK) && (CONFIG_PIN_I2S_4_MCLK == CONFIG_PIN_OID_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_MCLK and CONFIG_PIN_OID_CLK conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_MCLK and CONFIG_PIN_OID_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_MCLK) && defined(CONFIG_PIN_CT32B5_PWM2) && (CONFIG_PIN_I2S_4_MCLK == CONFIG_PIN_CT32B5_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_MCLK and CONFIG_PIN_CT32B5_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_MCLK and CONFIG_PIN_CT32B5_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_OID_CLK) && defined(CONFIG_PIN_CT32B5_PWM2) && (CONFIG_PIN_OID_CLK == CONFIG_PIN_CT32B5_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_CLK and CONFIG_PIN_CT32B5_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_OID_CLK and CONFIG_PIN_CT32B5_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_VSYNC) && defined(CONFIG_PIN_I2S_4_SDOUT) && (CONFIG_PIN_VSYNC == CONFIG_PIN_I2S_4_SDOUT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_VSYNC and CONFIG_PIN_I2S_4_SDOUT conflict !"
	#else
		#error "CONFIG_PIN_VSYNC and CONFIG_PIN_I2S_4_SDOUT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_VSYNC) && defined(CONFIG_PIN_OID_CMD) && (CONFIG_PIN_VSYNC == CONFIG_PIN_OID_CMD)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_VSYNC and CONFIG_PIN_OID_CMD conflict !"
	#else
		#error "CONFIG_PIN_VSYNC and CONFIG_PIN_OID_CMD conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_VSYNC) && defined(CONFIG_PIN_CT32B5_PWM1) && (CONFIG_PIN_VSYNC == CONFIG_PIN_CT32B5_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_VSYNC and CONFIG_PIN_CT32B5_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_VSYNC and CONFIG_PIN_CT32B5_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_SDOUT) && defined(CONFIG_PIN_OID_CMD) && (CONFIG_PIN_I2S_4_SDOUT == CONFIG_PIN_OID_CMD)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_SDOUT and CONFIG_PIN_OID_CMD conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_SDOUT and CONFIG_PIN_OID_CMD conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_SDOUT) && defined(CONFIG_PIN_CT32B5_PWM1) && (CONFIG_PIN_I2S_4_SDOUT == CONFIG_PIN_CT32B5_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_SDOUT and CONFIG_PIN_CT32B5_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_SDOUT and CONFIG_PIN_CT32B5_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_OID_CMD) && defined(CONFIG_PIN_CT32B5_PWM1) && (CONFIG_PIN_OID_CMD == CONFIG_PIN_CT32B5_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_CMD and CONFIG_PIN_CT32B5_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_OID_CMD and CONFIG_PIN_CT32B5_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DE) && defined(CONFIG_PIN_I2S_4_BCLK) && (CONFIG_PIN_DE == CONFIG_PIN_I2S_4_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DE and CONFIG_PIN_I2S_4_BCLK conflict !"
	#else
		#error "CONFIG_PIN_DE and CONFIG_PIN_I2S_4_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DE) && defined(CONFIG_PIN_OID_DATA_0) && (CONFIG_PIN_DE == CONFIG_PIN_OID_DATA_0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DE and CONFIG_PIN_OID_DATA_0 conflict !"
	#else
		#error "CONFIG_PIN_DE and CONFIG_PIN_OID_DATA_0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DE) && defined(CONFIG_PIN_CT32B5_PWM0) && (CONFIG_PIN_DE == CONFIG_PIN_CT32B5_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DE and CONFIG_PIN_CT32B5_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_DE and CONFIG_PIN_CT32B5_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_BCLK) && defined(CONFIG_PIN_OID_DATA_0) && (CONFIG_PIN_I2S_4_BCLK == CONFIG_PIN_OID_DATA_0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_BCLK and CONFIG_PIN_OID_DATA_0 conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_BCLK and CONFIG_PIN_OID_DATA_0 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_I2S_4_BCLK) && defined(CONFIG_PIN_CT32B5_PWM0) && (CONFIG_PIN_I2S_4_BCLK == CONFIG_PIN_CT32B5_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_BCLK and CONFIG_PIN_CT32B5_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_BCLK and CONFIG_PIN_CT32B5_PWM0 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_OID_DATA_0) && defined(CONFIG_PIN_CT32B5_PWM0) && (CONFIG_PIN_OID_DATA_0 == CONFIG_PIN_CT32B5_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_DATA_0 and CONFIG_PIN_CT32B5_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_OID_DATA_0 and CONFIG_PIN_CT32B5_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DCLK) && defined(CONFIG_PIN_I2S_4_WS) && (CONFIG_PIN_DCLK == CONFIG_PIN_I2S_4_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DCLK and CONFIG_PIN_I2S_4_WS conflict !"
	#else
		#error "CONFIG_PIN_DCLK and CONFIG_PIN_I2S_4_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DCLK) && defined(CONFIG_PIN_OID_DATA_1) && (CONFIG_PIN_DCLK == CONFIG_PIN_OID_DATA_1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DCLK and CONFIG_PIN_OID_DATA_1 conflict !"
	#else
		#error "CONFIG_PIN_DCLK and CONFIG_PIN_OID_DATA_1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_DCLK) && defined(CONFIG_PIN_CT32B4_PWM2) && (CONFIG_PIN_DCLK == CONFIG_PIN_CT32B4_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_DCLK and CONFIG_PIN_CT32B4_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_DCLK and CONFIG_PIN_CT32B4_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_WS) && defined(CONFIG_PIN_OID_DATA_1) && (CONFIG_PIN_I2S_4_WS == CONFIG_PIN_OID_DATA_1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_WS and CONFIG_PIN_OID_DATA_1 conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_WS and CONFIG_PIN_OID_DATA_1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_WS) && defined(CONFIG_PIN_CT32B4_PWM2) && (CONFIG_PIN_I2S_4_WS == CONFIG_PIN_CT32B4_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_WS and CONFIG_PIN_CT32B4_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_WS and CONFIG_PIN_CT32B4_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_OID_DATA_1) && defined(CONFIG_PIN_CT32B4_PWM2) && (CONFIG_PIN_OID_DATA_1 == CONFIG_PIN_CT32B4_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_DATA_1 and CONFIG_PIN_CT32B4_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_OID_DATA_1 and CONFIG_PIN_CT32B4_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED0) && defined(CONFIG_PIN_I2S_4_SDIN) && (CONFIG_PIN_ED0 == CONFIG_PIN_I2S_4_SDIN)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED0 and CONFIG_PIN_I2S_4_SDIN conflict !"
	#else
		#error "CONFIG_PIN_ED0 and CONFIG_PIN_I2S_4_SDIN conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED8) && defined(CONFIG_PIN_NF_CS) && (CONFIG_PIN_ED8 == CONFIG_PIN_NF_CS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED8 and CONFIG_PIN_NF_CS conflict !"
	#else
		#error "CONFIG_PIN_ED8 and CONFIG_PIN_NF_CS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED9) && defined(CONFIG_PIN_NF_RB) && (CONFIG_PIN_ED9 == CONFIG_PIN_NF_RB)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED9 and CONFIG_PIN_NF_RB conflict !"
	#else
		#error "CONFIG_PIN_ED9 and CONFIG_PIN_NF_RB conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED10) && defined(CONFIG_PIN_NF_ALE) && (CONFIG_PIN_ED10 == CONFIG_PIN_NF_ALE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED10 and CONFIG_PIN_NF_ALE conflict !"
	#else
		#error "CONFIG_PIN_ED10 and CONFIG_PIN_NF_ALE conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED10) && defined(CONFIG_PIN_I2S_4_MCLK) && (CONFIG_PIN_ED10 == CONFIG_PIN_I2S_4_MCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED10 and CONFIG_PIN_I2S_4_MCLK conflict !"
	#else
		#error "CONFIG_PIN_ED10 and CONFIG_PIN_I2S_4_MCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED10) && defined(CONFIG_PIN_I2C_2_CLK) && (CONFIG_PIN_ED10 == CONFIG_PIN_I2C_2_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED10 and CONFIG_PIN_I2C_2_CLK conflict !"
	#else
		#error "CONFIG_PIN_ED10 and CONFIG_PIN_I2C_2_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_ALE) && defined(CONFIG_PIN_I2S_4_MCLK) && (CONFIG_PIN_NF_ALE == CONFIG_PIN_I2S_4_MCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_ALE and CONFIG_PIN_I2S_4_MCLK conflict !"
	#else
		#error "CONFIG_PIN_NF_ALE and CONFIG_PIN_I2S_4_MCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_ALE) && defined(CONFIG_PIN_I2C_2_CLK) && (CONFIG_PIN_NF_ALE == CONFIG_PIN_I2C_2_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_ALE and CONFIG_PIN_I2C_2_CLK conflict !"
	#else
		#error "CONFIG_PIN_NF_ALE and CONFIG_PIN_I2C_2_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_MCLK) && defined(CONFIG_PIN_I2C_2_CLK) && (CONFIG_PIN_I2S_4_MCLK == CONFIG_PIN_I2C_2_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_MCLK and CONFIG_PIN_I2C_2_CLK conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_MCLK and CONFIG_PIN_I2C_2_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED11) && defined(CONFIG_PIN_NF_WE) && (CONFIG_PIN_ED11 == CONFIG_PIN_NF_WE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED11 and CONFIG_PIN_NF_WE conflict !"
	#else
		#error "CONFIG_PIN_ED11 and CONFIG_PIN_NF_WE conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED11) && defined(CONFIG_PIN_I2S_4_SDIN) && (CONFIG_PIN_ED11 == CONFIG_PIN_I2S_4_SDIN)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED11 and CONFIG_PIN_I2S_4_SDIN conflict !"
	#else
		#error "CONFIG_PIN_ED11 and CONFIG_PIN_I2S_4_SDIN conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED11) && defined(CONFIG_PIN_I2C_2_DAT) && (CONFIG_PIN_ED11 == CONFIG_PIN_I2C_2_DAT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED11 and CONFIG_PIN_I2C_2_DAT conflict !"
	#else
		#error "CONFIG_PIN_ED11 and CONFIG_PIN_I2C_2_DAT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED11) && defined(CONFIG_PIN_I2S_1_SDIO) && (CONFIG_PIN_ED11 == CONFIG_PIN_I2S_1_SDIO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED11 and CONFIG_PIN_I2S_1_SDIO conflict !"
	#else
		#error "CONFIG_PIN_ED11 and CONFIG_PIN_I2S_1_SDIO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_WE) && defined(CONFIG_PIN_I2S_4_SDIN) && (CONFIG_PIN_NF_WE == CONFIG_PIN_I2S_4_SDIN)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WE and CONFIG_PIN_I2S_4_SDIN conflict !"
	#else
		#error "CONFIG_PIN_NF_WE and CONFIG_PIN_I2S_4_SDIN conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_WE) && defined(CONFIG_PIN_I2C_2_DAT) && (CONFIG_PIN_NF_WE == CONFIG_PIN_I2C_2_DAT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WE and CONFIG_PIN_I2C_2_DAT conflict !"
	#else
		#error "CONFIG_PIN_NF_WE and CONFIG_PIN_I2C_2_DAT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_WE) && defined(CONFIG_PIN_I2S_1_SDIO) && (CONFIG_PIN_NF_WE == CONFIG_PIN_I2S_1_SDIO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WE and CONFIG_PIN_I2S_1_SDIO conflict !"
	#else
		#error "CONFIG_PIN_NF_WE and CONFIG_PIN_I2S_1_SDIO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_SDIN) && defined(CONFIG_PIN_I2C_2_DAT) && (CONFIG_PIN_I2S_4_SDIN == CONFIG_PIN_I2C_2_DAT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_SDIN and CONFIG_PIN_I2C_2_DAT conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_SDIN and CONFIG_PIN_I2C_2_DAT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_SDIN) && defined(CONFIG_PIN_I2S_1_SDIO) && (CONFIG_PIN_I2S_4_SDIN == CONFIG_PIN_I2S_1_SDIO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_SDIN and CONFIG_PIN_I2S_1_SDIO conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_SDIN and CONFIG_PIN_I2S_1_SDIO conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_I2C_2_DAT) && defined(CONFIG_PIN_I2S_1_SDIO) && (CONFIG_PIN_I2C_2_DAT == CONFIG_PIN_I2S_1_SDIO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2C_2_DAT and CONFIG_PIN_I2S_1_SDIO conflict !"
	#else
		#error "CONFIG_PIN_I2C_2_DAT and CONFIG_PIN_I2S_1_SDIO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED12) && defined(CONFIG_PIN_NF_RE) && (CONFIG_PIN_ED12 == CONFIG_PIN_NF_RE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED12 and CONFIG_PIN_NF_RE conflict !"
	#else
		#error "CONFIG_PIN_ED12 and CONFIG_PIN_NF_RE conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED12) && defined(CONFIG_PIN_I2S_4_SDOUT) && (CONFIG_PIN_ED12 == CONFIG_PIN_I2S_4_SDOUT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED12 and CONFIG_PIN_I2S_4_SDOUT conflict !"
	#else
		#error "CONFIG_PIN_ED12 and CONFIG_PIN_I2S_4_SDOUT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED12) && defined(CONFIG_PIN_I2S_1_SDIO) && (CONFIG_PIN_ED12 == CONFIG_PIN_I2S_1_SDIO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED12 and CONFIG_PIN_I2S_1_SDIO conflict !"
	#else
		#error "CONFIG_PIN_ED12 and CONFIG_PIN_I2S_1_SDIO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED12) && defined(CONFIG_PIN_I2S_1_WS) && (CONFIG_PIN_ED12 == CONFIG_PIN_I2S_1_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED12 and CONFIG_PIN_I2S_1_WS conflict !"
	#else
		#error "CONFIG_PIN_ED12 and CONFIG_PIN_I2S_1_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_RE) && defined(CONFIG_PIN_I2S_4_SDOUT) && (CONFIG_PIN_NF_RE == CONFIG_PIN_I2S_4_SDOUT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_RE and CONFIG_PIN_I2S_4_SDOUT conflict !"
	#else
		#error "CONFIG_PIN_NF_RE and CONFIG_PIN_I2S_4_SDOUT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_RE) && defined(CONFIG_PIN_I2S_1_SDIO) && (CONFIG_PIN_NF_RE == CONFIG_PIN_I2S_1_SDIO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_RE and CONFIG_PIN_I2S_1_SDIO conflict !"
	#else
		#error "CONFIG_PIN_NF_RE and CONFIG_PIN_I2S_1_SDIO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_RE) && defined(CONFIG_PIN_I2S_1_WS) && (CONFIG_PIN_NF_RE == CONFIG_PIN_I2S_1_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_RE and CONFIG_PIN_I2S_1_WS conflict !"
	#else
		#error "CONFIG_PIN_NF_RE and CONFIG_PIN_I2S_1_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_SDOUT) && defined(CONFIG_PIN_I2S_1_SDIO) && (CONFIG_PIN_I2S_4_SDOUT == CONFIG_PIN_I2S_1_SDIO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_SDOUT and CONFIG_PIN_I2S_1_SDIO conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_SDOUT and CONFIG_PIN_I2S_1_SDIO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_SDOUT) && defined(CONFIG_PIN_I2S_1_WS) && (CONFIG_PIN_I2S_4_SDOUT == CONFIG_PIN_I2S_1_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_SDOUT and CONFIG_PIN_I2S_1_WS conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_SDOUT and CONFIG_PIN_I2S_1_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_1_SDIO) && defined(CONFIG_PIN_I2S_1_WS) && (CONFIG_PIN_I2S_1_SDIO == CONFIG_PIN_I2S_1_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_1_SDIO and CONFIG_PIN_I2S_1_WS conflict !"
	#else
		#error "CONFIG_PIN_I2S_1_SDIO and CONFIG_PIN_I2S_1_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED13) && defined(CONFIG_PIN_NF_WP) && (CONFIG_PIN_ED13 == CONFIG_PIN_NF_WP)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED13 and CONFIG_PIN_NF_WP conflict !"
	#else
		#error "CONFIG_PIN_ED13 and CONFIG_PIN_NF_WP conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED13) && defined(CONFIG_PIN_SD0_CLK) && (CONFIG_PIN_ED13 == CONFIG_PIN_SD0_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED13 and CONFIG_PIN_SD0_CLK conflict !"
	#else
		#error "CONFIG_PIN_ED13 and CONFIG_PIN_SD0_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED13) && defined(CONFIG_PIN_I2S_4_BCLK) && (CONFIG_PIN_ED13 == CONFIG_PIN_I2S_4_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED13 and CONFIG_PIN_I2S_4_BCLK conflict !"
	#else
		#error "CONFIG_PIN_ED13 and CONFIG_PIN_I2S_4_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED13) && defined(CONFIG_PIN_I2S_1_BCLK) && (CONFIG_PIN_ED13 == CONFIG_PIN_I2S_1_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED13 and CONFIG_PIN_I2S_1_BCLK conflict !"
	#else
		#error "CONFIG_PIN_ED13 and CONFIG_PIN_I2S_1_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED13) && defined(CONFIG_PIN_I2C_2_DAT) && (CONFIG_PIN_ED13 == CONFIG_PIN_I2C_2_DAT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED13 and CONFIG_PIN_I2C_2_DAT conflict !"
	#else
		#error "CONFIG_PIN_ED13 and CONFIG_PIN_I2C_2_DAT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_WP) && defined(CONFIG_PIN_SD0_CLK) && (CONFIG_PIN_NF_WP == CONFIG_PIN_SD0_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WP and CONFIG_PIN_SD0_CLK conflict !"
	#else
		#error "CONFIG_PIN_NF_WP and CONFIG_PIN_SD0_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_WP) && defined(CONFIG_PIN_I2S_4_BCLK) && (CONFIG_PIN_NF_WP == CONFIG_PIN_I2S_4_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WP and CONFIG_PIN_I2S_4_BCLK conflict !"
	#else
		#error "CONFIG_PIN_NF_WP and CONFIG_PIN_I2S_4_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_WP) && defined(CONFIG_PIN_I2S_1_BCLK) && (CONFIG_PIN_NF_WP == CONFIG_PIN_I2S_1_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WP and CONFIG_PIN_I2S_1_BCLK conflict !"
	#else
		#error "CONFIG_PIN_NF_WP and CONFIG_PIN_I2S_1_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_WP) && defined(CONFIG_PIN_I2C_2_DAT) && (CONFIG_PIN_NF_WP == CONFIG_PIN_I2C_2_DAT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_WP and CONFIG_PIN_I2C_2_DAT conflict !"
	#else
		#error "CONFIG_PIN_NF_WP and CONFIG_PIN_I2C_2_DAT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CLK) && defined(CONFIG_PIN_I2S_4_BCLK) && (CONFIG_PIN_SD0_CLK == CONFIG_PIN_I2S_4_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CLK and CONFIG_PIN_I2S_4_BCLK conflict !"
	#else
		#error "CONFIG_PIN_SD0_CLK and CONFIG_PIN_I2S_4_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CLK) && defined(CONFIG_PIN_I2S_1_BCLK) && (CONFIG_PIN_SD0_CLK == CONFIG_PIN_I2S_1_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CLK and CONFIG_PIN_I2S_1_BCLK conflict !"
	#else
		#error "CONFIG_PIN_SD0_CLK and CONFIG_PIN_I2S_1_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CLK) && defined(CONFIG_PIN_I2C_2_DAT) && (CONFIG_PIN_SD0_CLK == CONFIG_PIN_I2C_2_DAT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CLK and CONFIG_PIN_I2C_2_DAT conflict !"
	#else
		#error "CONFIG_PIN_SD0_CLK and CONFIG_PIN_I2C_2_DAT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_BCLK) && defined(CONFIG_PIN_I2S_1_BCLK) && (CONFIG_PIN_I2S_4_BCLK == CONFIG_PIN_I2S_1_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_BCLK and CONFIG_PIN_I2S_1_BCLK conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_BCLK and CONFIG_PIN_I2S_1_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_BCLK) && defined(CONFIG_PIN_I2C_2_DAT) && (CONFIG_PIN_I2S_4_BCLK == CONFIG_PIN_I2C_2_DAT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_BCLK and CONFIG_PIN_I2C_2_DAT conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_BCLK and CONFIG_PIN_I2C_2_DAT conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_I2S_1_BCLK) && defined(CONFIG_PIN_I2C_2_DAT) && (CONFIG_PIN_I2S_1_BCLK == CONFIG_PIN_I2C_2_DAT)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_1_BCLK and CONFIG_PIN_I2C_2_DAT conflict !"
	#else
		#error "CONFIG_PIN_I2S_1_BCLK and CONFIG_PIN_I2C_2_DAT conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED14) && defined(CONFIG_PIN_NF_CLE) && (CONFIG_PIN_ED14 == CONFIG_PIN_NF_CLE)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED14 and CONFIG_PIN_NF_CLE conflict !"
	#else
		#error "CONFIG_PIN_ED14 and CONFIG_PIN_NF_CLE conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED14) && defined(CONFIG_PIN_SD0_CMD) && (CONFIG_PIN_ED14 == CONFIG_PIN_SD0_CMD)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED14 and CONFIG_PIN_SD0_CMD conflict !"
	#else
		#error "CONFIG_PIN_ED14 and CONFIG_PIN_SD0_CMD conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED14) && defined(CONFIG_PIN_I2S_4_WS) && (CONFIG_PIN_ED14 == CONFIG_PIN_I2S_4_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED14 and CONFIG_PIN_I2S_4_WS conflict !"
	#else
		#error "CONFIG_PIN_ED14 and CONFIG_PIN_I2S_4_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED14) && defined(CONFIG_PIN_I2S_1_WS) && (CONFIG_PIN_ED14 == CONFIG_PIN_I2S_1_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED14 and CONFIG_PIN_I2S_1_WS conflict !"
	#else
		#error "CONFIG_PIN_ED14 and CONFIG_PIN_I2S_1_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED14) && defined(CONFIG_PIN_I2S_1_BCLK) && (CONFIG_PIN_ED14 == CONFIG_PIN_I2S_1_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED14 and CONFIG_PIN_I2S_1_BCLK conflict !"
	#else
		#error "CONFIG_PIN_ED14 and CONFIG_PIN_I2S_1_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_CLE) && defined(CONFIG_PIN_SD0_CMD) && (CONFIG_PIN_NF_CLE == CONFIG_PIN_SD0_CMD)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_CLE and CONFIG_PIN_SD0_CMD conflict !"
	#else
		#error "CONFIG_PIN_NF_CLE and CONFIG_PIN_SD0_CMD conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_CLE) && defined(CONFIG_PIN_I2S_4_WS) && (CONFIG_PIN_NF_CLE == CONFIG_PIN_I2S_4_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_CLE and CONFIG_PIN_I2S_4_WS conflict !"
	#else
		#error "CONFIG_PIN_NF_CLE and CONFIG_PIN_I2S_4_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_CLE) && defined(CONFIG_PIN_I2S_1_WS) && (CONFIG_PIN_NF_CLE == CONFIG_PIN_I2S_1_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_CLE and CONFIG_PIN_I2S_1_WS conflict !"
	#else
		#error "CONFIG_PIN_NF_CLE and CONFIG_PIN_I2S_1_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_CLE) && defined(CONFIG_PIN_I2S_1_BCLK) && (CONFIG_PIN_NF_CLE == CONFIG_PIN_I2S_1_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_CLE and CONFIG_PIN_I2S_1_BCLK conflict !"
	#else
		#error "CONFIG_PIN_NF_CLE and CONFIG_PIN_I2S_1_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CMD) && defined(CONFIG_PIN_I2S_4_WS) && (CONFIG_PIN_SD0_CMD == CONFIG_PIN_I2S_4_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CMD and CONFIG_PIN_I2S_4_WS conflict !"
	#else
		#error "CONFIG_PIN_SD0_CMD and CONFIG_PIN_I2S_4_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_CMD) && defined(CONFIG_PIN_I2S_1_WS) && (CONFIG_PIN_SD0_CMD == CONFIG_PIN_I2S_1_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_CMD and CONFIG_PIN_I2S_1_WS conflict !"
	#else
		#error "CONFIG_PIN_SD0_CMD and CONFIG_PIN_I2S_1_WS conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SD0_CMD) && defined(CONFIG_PIN_I2S_1_BCLK) && (CONFIG_PIN_SD0_CMD == CONFIG_PIN_I2S_1_BCLK)
	#if IO_SHARE_PIN
		#warning CONFIG_PIN_SD0_CMD and CONFIG_PIN_I2S_1_BCLK conflict !"
	#else
		#error "CONFIG_PIN_SD0_CMD and CONFIG_PIN_I2S_1_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_4_WS) && defined(CONFIG_PIN_I2S_1_WS) && (CONFIG_PIN_I2S_4_WS == CONFIG_PIN_I2S_1_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_WS and CONFIG_PIN_I2S_1_WS conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_WS and CONFIG_PIN_I2S_1_WS conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_I2S_4_WS) && defined(CONFIG_PIN_I2S_1_BCLK) && (CONFIG_PIN_I2S_4_WS == CONFIG_PIN_I2S_1_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_4_WS and CONFIG_PIN_I2S_1_BCLK conflict !"
	#else
		#error "CONFIG_PIN_I2S_4_WS and CONFIG_PIN_I2S_1_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_1_WS) && defined(CONFIG_PIN_I2S_1_BCLK) && (CONFIG_PIN_I2S_1_WS == CONFIG_PIN_I2S_1_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_1_WS and CONFIG_PIN_I2S_1_BCLK conflict !"
	#else
		#error "CONFIG_PIN_I2S_1_WS and CONFIG_PIN_I2S_1_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED15) && defined(CONFIG_PIN_NF_D0) && (CONFIG_PIN_ED15 == CONFIG_PIN_NF_D0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED15 and CONFIG_PIN_NF_D0 conflict !"
	#else
		#error "CONFIG_PIN_ED15 and CONFIG_PIN_NF_D0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED15) && defined(CONFIG_PIN_SD0_D0) && (CONFIG_PIN_ED15 == CONFIG_PIN_SD0_D0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED15 and CONFIG_PIN_SD0_D0 conflict !"
	#else
		#error "CONFIG_PIN_ED15 and CONFIG_PIN_SD0_D0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED15) && defined(CONFIG_PIN_I2S_3_SDIO) && (CONFIG_PIN_ED15 == CONFIG_PIN_I2S_3_SDIO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED15 and CONFIG_PIN_I2S_3_SDIO conflict !"
	#else
		#error "CONFIG_PIN_ED15 and CONFIG_PIN_I2S_3_SDIO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D0) && defined(CONFIG_PIN_SD0_D0) && (CONFIG_PIN_NF_D0 == CONFIG_PIN_SD0_D0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D0 and CONFIG_PIN_SD0_D0 conflict !"
	#else
		#error "CONFIG_PIN_NF_D0 and CONFIG_PIN_SD0_D0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D0) && defined(CONFIG_PIN_I2S_3_SDIO) && (CONFIG_PIN_NF_D0 == CONFIG_PIN_I2S_3_SDIO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D0 and CONFIG_PIN_I2S_3_SDIO conflict !"
	#else
		#error "CONFIG_PIN_NF_D0 and CONFIG_PIN_I2S_3_SDIO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D0) && defined(CONFIG_PIN_I2S_3_SDIO) && (CONFIG_PIN_SD0_D0 == CONFIG_PIN_I2S_3_SDIO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D0 and CONFIG_PIN_I2S_3_SDIO conflict !"
	#else
		#error "CONFIG_PIN_SD0_D0 and CONFIG_PIN_I2S_3_SDIO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED16) && defined(CONFIG_PIN_NF_D1) && (CONFIG_PIN_ED16 == CONFIG_PIN_NF_D1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED16 and CONFIG_PIN_NF_D1 conflict !"
	#else
		#error "CONFIG_PIN_ED16 and CONFIG_PIN_NF_D1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED16) && defined(CONFIG_PIN_SD0_D1) && (CONFIG_PIN_ED16 == CONFIG_PIN_SD0_D1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED16 and CONFIG_PIN_SD0_D1 conflict !"
	#else
		#error "CONFIG_PIN_ED16 and CONFIG_PIN_SD0_D1 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_ED16) && defined(CONFIG_PIN_I2S_3_BCLK) && (CONFIG_PIN_ED16 == CONFIG_PIN_I2S_3_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED16 and CONFIG_PIN_I2S_3_BCLK conflict !"
	#else
		#error "CONFIG_PIN_ED16 and CONFIG_PIN_I2S_3_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D1) && defined(CONFIG_PIN_SD0_D1) && (CONFIG_PIN_NF_D1 == CONFIG_PIN_SD0_D1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D1 and CONFIG_PIN_SD0_D1 conflict !"
	#else
		#error "CONFIG_PIN_NF_D1 and CONFIG_PIN_SD0_D1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D1) && defined(CONFIG_PIN_I2S_3_BCLK) && (CONFIG_PIN_NF_D1 == CONFIG_PIN_I2S_3_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D1 and CONFIG_PIN_I2S_3_BCLK conflict !"
	#else
		#error "CONFIG_PIN_NF_D1 and CONFIG_PIN_I2S_3_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D1) && defined(CONFIG_PIN_I2S_3_BCLK) && (CONFIG_PIN_SD0_D1 == CONFIG_PIN_I2S_3_BCLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D1 and CONFIG_PIN_I2S_3_BCLK conflict !"
	#else
		#error "CONFIG_PIN_SD0_D1 and CONFIG_PIN_I2S_3_BCLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED17) && defined(CONFIG_PIN_NF_D2) && (CONFIG_PIN_ED17 == CONFIG_PIN_NF_D2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED17 and CONFIG_PIN_NF_D2 conflict !"
	#else
		#error "CONFIG_PIN_ED17 and CONFIG_PIN_NF_D2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED17) && defined(CONFIG_PIN_SD0_D2) && (CONFIG_PIN_ED17 == CONFIG_PIN_SD0_D2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED17 and CONFIG_PIN_SD0_D2 conflict !"
	#else
		#error "CONFIG_PIN_ED17 and CONFIG_PIN_SD0_D2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_ED17) && defined(CONFIG_PIN_I2S_3_WS) && (CONFIG_PIN_ED17 == CONFIG_PIN_I2S_3_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_ED17 and CONFIG_PIN_I2S_3_WS conflict !"
	#else
		#error "CONFIG_PIN_ED17 and CONFIG_PIN_I2S_3_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D2) && defined(CONFIG_PIN_SD0_D2) && (CONFIG_PIN_NF_D2 == CONFIG_PIN_SD0_D2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D2 and CONFIG_PIN_SD0_D2 conflict !"
	#else
		#error "CONFIG_PIN_NF_D2 and CONFIG_PIN_SD0_D2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D2) && defined(CONFIG_PIN_I2S_3_WS) && (CONFIG_PIN_NF_D2 == CONFIG_PIN_I2S_3_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D2 and CONFIG_PIN_I2S_3_WS conflict !"
	#else
		#error "CONFIG_PIN_NF_D2 and CONFIG_PIN_I2S_3_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D2) && defined(CONFIG_PIN_I2S_3_WS) && (CONFIG_PIN_SD0_D2 == CONFIG_PIN_I2S_3_WS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D2 and CONFIG_PIN_I2S_3_WS conflict !"
	#else
		#error "CONFIG_PIN_SD0_D2 and CONFIG_PIN_I2S_3_WS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D3) && defined(CONFIG_PIN_SD0_D3) && (CONFIG_PIN_NF_D3 == CONFIG_PIN_SD0_D3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D3 and CONFIG_PIN_SD0_D3 conflict !"
	#else
		#error "CONFIG_PIN_NF_D3 and CONFIG_PIN_SD0_D3 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D3) && defined(CONFIG_PIN_CT32B4_PWM1) && (CONFIG_PIN_NF_D3 == CONFIG_PIN_CT32B4_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D3 and CONFIG_PIN_CT32B4_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_NF_D3 and CONFIG_PIN_CT32B4_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SD0_D3) && defined(CONFIG_PIN_CT32B4_PWM1) && (CONFIG_PIN_SD0_D3 == CONFIG_PIN_CT32B4_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SD0_D3 and CONFIG_PIN_CT32B4_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SD0_D3 and CONFIG_PIN_CT32B4_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D4) && defined(CONFIG_PIN_CT32B4_PWM0) && (CONFIG_PIN_NF_D4 == CONFIG_PIN_CT32B4_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D4 and CONFIG_PIN_CT32B4_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_NF_D4 and CONFIG_PIN_CT32B4_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_CS) && defined(CONFIG_PIN_NF_D5) && (CONFIG_PIN_SPIIF_1_CS == CONFIG_PIN_NF_D5)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_CS and CONFIG_PIN_NF_D5 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_CS and CONFIG_PIN_NF_D5 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_CS) && defined(CONFIG_PIN_SPI1_DMA_CS) && (CONFIG_PIN_SPIIF_1_CS == CONFIG_PIN_SPI1_DMA_CS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_CS and CONFIG_PIN_SPI1_DMA_CS conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_CS and CONFIG_PIN_SPI1_DMA_CS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_CS) && defined(CONFIG_PIN_CT32B0_CAP0) && (CONFIG_PIN_SPIIF_1_CS == CONFIG_PIN_CT32B0_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_CS and CONFIG_PIN_CT32B0_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_CS and CONFIG_PIN_CT32B0_CAP0 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SPIIF_1_CS) && defined(CONFIG_PIN_CT32B3_PWM2) && (CONFIG_PIN_SPIIF_1_CS == CONFIG_PIN_CT32B3_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_CS and CONFIG_PIN_CT32B3_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_CS and CONFIG_PIN_CT32B3_PWM2 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_NF_D5) && defined(CONFIG_PIN_SPI1_DMA_CS) && (CONFIG_PIN_NF_D5 == CONFIG_PIN_SPI1_DMA_CS)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D5 and CONFIG_PIN_SPI1_DMA_CS conflict !"
	#else
		#error "CONFIG_PIN_NF_D5 and CONFIG_PIN_SPI1_DMA_CS conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D5) && defined(CONFIG_PIN_CT32B0_CAP0) && (CONFIG_PIN_NF_D5 == CONFIG_PIN_CT32B0_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D5 and CONFIG_PIN_CT32B0_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_NF_D5 and CONFIG_PIN_CT32B0_CAP0 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_NF_D5) && defined(CONFIG_PIN_CT32B3_PWM2) && (CONFIG_PIN_NF_D5 == CONFIG_PIN_CT32B3_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D5 and CONFIG_PIN_CT32B3_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_NF_D5 and CONFIG_PIN_CT32B3_PWM2 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SPI1_DMA_CS) && defined(CONFIG_PIN_CT32B0_CAP0) && (CONFIG_PIN_SPI1_DMA_CS == CONFIG_PIN_CT32B0_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_CS and CONFIG_PIN_CT32B0_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_CS and CONFIG_PIN_CT32B0_CAP0 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SPI1_DMA_CS) && defined(CONFIG_PIN_CT32B3_PWM2) && (CONFIG_PIN_SPI1_DMA_CS == CONFIG_PIN_CT32B3_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_CS and CONFIG_PIN_CT32B3_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_CS and CONFIG_PIN_CT32B3_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CT32B0_CAP0) && defined(CONFIG_PIN_CT32B3_PWM2) && (CONFIG_PIN_CT32B0_CAP0 == CONFIG_PIN_CT32B3_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CT32B0_CAP0 and CONFIG_PIN_CT32B3_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_CT32B0_CAP0 and CONFIG_PIN_CT32B3_PWM2 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SPIIF_1_SCK) && defined(CONFIG_PIN_NF_D6) && (CONFIG_PIN_SPIIF_1_SCK == CONFIG_PIN_NF_D6)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_SCK and CONFIG_PIN_NF_D6 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_SCK and CONFIG_PIN_NF_D6 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_SCK) && defined(CONFIG_PIN_SPI1_DMA_SCK) && (CONFIG_PIN_SPIIF_1_SCK == CONFIG_PIN_SPI1_DMA_SCK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_SCK and CONFIG_PIN_SPI1_DMA_SCK conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_SCK and CONFIG_PIN_SPI1_DMA_SCK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_SCK) && defined(CONFIG_PIN_CT32B1_CAP0) && (CONFIG_PIN_SPIIF_1_SCK == CONFIG_PIN_CT32B1_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_SCK and CONFIG_PIN_CT32B1_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_SCK and CONFIG_PIN_CT32B1_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_SCK) && defined(CONFIG_PIN_CT32B3_PWM1) && (CONFIG_PIN_SPIIF_1_SCK == CONFIG_PIN_CT32B3_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_SCK and CONFIG_PIN_CT32B3_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_SCK and CONFIG_PIN_CT32B3_PWM1 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_NF_D6) && defined(CONFIG_PIN_SPI1_DMA_SCK) && (CONFIG_PIN_NF_D6 == CONFIG_PIN_SPI1_DMA_SCK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D6 and CONFIG_PIN_SPI1_DMA_SCK conflict !"
	#else
		#error "CONFIG_PIN_NF_D6 and CONFIG_PIN_SPI1_DMA_SCK conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_NF_D6) && defined(CONFIG_PIN_CT32B1_CAP0) && (CONFIG_PIN_NF_D6 == CONFIG_PIN_CT32B1_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D6 and CONFIG_PIN_CT32B1_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_NF_D6 and CONFIG_PIN_CT32B1_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D6) && defined(CONFIG_PIN_CT32B3_PWM1) && (CONFIG_PIN_NF_D6 == CONFIG_PIN_CT32B3_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D6 and CONFIG_PIN_CT32B3_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_NF_D6 and CONFIG_PIN_CT32B3_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI1_DMA_SCK) && defined(CONFIG_PIN_CT32B1_CAP0) && (CONFIG_PIN_SPI1_DMA_SCK == CONFIG_PIN_CT32B1_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_SCK and CONFIG_PIN_CT32B1_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_SCK and CONFIG_PIN_CT32B1_CAP0 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SPI1_DMA_SCK) && defined(CONFIG_PIN_CT32B3_PWM1) && (CONFIG_PIN_SPI1_DMA_SCK == CONFIG_PIN_CT32B3_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_SCK and CONFIG_PIN_CT32B3_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_SCK and CONFIG_PIN_CT32B3_PWM1 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_CT32B1_CAP0) && defined(CONFIG_PIN_CT32B3_PWM1) && (CONFIG_PIN_CT32B1_CAP0 == CONFIG_PIN_CT32B3_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CT32B1_CAP0 and CONFIG_PIN_CT32B3_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_CT32B1_CAP0 and CONFIG_PIN_CT32B3_PWM1 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SPIIF_1_MISO) && defined(CONFIG_PIN_NF_D7) && (CONFIG_PIN_SPIIF_1_MISO == CONFIG_PIN_NF_D7)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_MISO and CONFIG_PIN_NF_D7 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_MISO and CONFIG_PIN_NF_D7 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SPIIF_1_MISO) && defined(CONFIG_PIN_SPI1_DMA_SO) && (CONFIG_PIN_SPIIF_1_MISO == CONFIG_PIN_SPI1_DMA_SO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_MISO and CONFIG_PIN_SPI1_DMA_SO conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_MISO and CONFIG_PIN_SPI1_DMA_SO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_MISO) && defined(CONFIG_PIN_CT32B2_CAP0) && (CONFIG_PIN_SPIIF_1_MISO == CONFIG_PIN_CT32B2_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_MISO and CONFIG_PIN_CT32B2_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_MISO and CONFIG_PIN_CT32B2_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_MISO) && defined(CONFIG_PIN_CT32B3_PWM0) && (CONFIG_PIN_SPIIF_1_MISO == CONFIG_PIN_CT32B3_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_MISO and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_MISO and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D7) && defined(CONFIG_PIN_SPI1_DMA_SO) && (CONFIG_PIN_NF_D7 == CONFIG_PIN_SPI1_DMA_SO)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D7 and CONFIG_PIN_SPI1_DMA_SO conflict !"
	#else
		#error "CONFIG_PIN_NF_D7 and CONFIG_PIN_SPI1_DMA_SO conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D7) && defined(CONFIG_PIN_CT32B2_CAP0) && (CONFIG_PIN_NF_D7 == CONFIG_PIN_CT32B2_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_NF_D7 and CONFIG_PIN_CT32B2_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_NF_D7 and CONFIG_PIN_CT32B2_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_NF_D7) && defined(CONFIG_PIN_CT32B3_PWM0) && (CONFIG_PIN_NF_D7 == CONFIG_PIN_CT32B3_PWM0)
	#if IO_SHARE_PIN
		#warning  "CONFIG_PIN_NF_D7 and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_NF_D7 and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI1_DMA_SO) && defined(CONFIG_PIN_CT32B2_CAP0) && (CONFIG_PIN_SPI1_DMA_SO == CONFIG_PIN_CT32B2_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_SO and CONFIG_PIN_CT32B2_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_SO and CONFIG_PIN_CT32B2_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI1_DMA_SO) && defined(CONFIG_PIN_CT32B3_PWM0) && (CONFIG_PIN_SPI1_DMA_SO == CONFIG_PIN_CT32B3_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_SO and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_SO and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CT32B2_CAP0) && defined(CONFIG_PIN_CT32B3_PWM0) && (CONFIG_PIN_CT32B2_CAP0 == CONFIG_PIN_CT32B3_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CT32B2_CAP0 and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_CT32B2_CAP0 and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_MOSI) && defined(CONFIG_PIN_SPI1_DMA_SI) && (CONFIG_PIN_SPIIF_1_MOSI == CONFIG_PIN_SPI1_DMA_SI)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_MOSI and CONFIG_PIN_SPI1_DMA_SI conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_MOSI and CONFIG_PIN_SPI1_DMA_SI conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_MOSI) && defined(CONFIG_PIN_CT32B3_CAP0) && (CONFIG_PIN_SPIIF_1_MOSI == CONFIG_PIN_CT32B3_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_MOSI and CONFIG_PIN_CT32B3_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_MOSI and CONFIG_PIN_CT32B3_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPIIF_1_MOSI) && defined(CONFIG_PIN_CT32B2_PWM2) && (CONFIG_PIN_SPIIF_1_MOSI == CONFIG_PIN_CT32B2_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPIIF_1_MOSI and CONFIG_PIN_CT32B2_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_SPIIF_1_MOSI and CONFIG_PIN_CT32B2_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI1_DMA_SI) && defined(CONFIG_PIN_CT32B3_CAP0) && (CONFIG_PIN_SPI1_DMA_SI == CONFIG_PIN_CT32B3_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_SI and CONFIG_PIN_CT32B3_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_SI and CONFIG_PIN_CT32B3_CAP0 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SPI1_DMA_SI) && defined(CONFIG_PIN_CT32B2_PWM2) && (CONFIG_PIN_SPI1_DMA_SI == CONFIG_PIN_CT32B2_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_SI and CONFIG_PIN_CT32B2_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_SI and CONFIG_PIN_CT32B2_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CT32B3_CAP0) && defined(CONFIG_PIN_CT32B2_PWM2) && (CONFIG_PIN_CT32B3_CAP0 == CONFIG_PIN_CT32B2_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CT32B3_CAP0 and CONFIG_PIN_CT32B2_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_CT32B3_CAP0 and CONFIG_PIN_CT32B2_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_UART_1_TXD) && defined(CONFIG_PIN_SPI1_DMA_SO2) && (CONFIG_PIN_UART_1_TXD == CONFIG_PIN_SPI1_DMA_SO2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_UART_1_TXD and CONFIG_PIN_SPI1_DMA_SO2 conflict !"
	#else
		#error "CONFIG_PIN_UART_1_TXD and CONFIG_PIN_SPI1_DMA_SO2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_UART_1_TXD) && defined(CONFIG_PIN_CT32B4_CAP0) && (CONFIG_PIN_UART_1_TXD == CONFIG_PIN_CT32B4_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_UART_1_TXD and CONFIG_PIN_CT32B4_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_UART_1_TXD and CONFIG_PIN_CT32B4_CAP0 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_UART_1_TXD) && defined(CONFIG_PIN_CT32B2_PWM1) && (CONFIG_PIN_UART_1_TXD == CONFIG_PIN_CT32B2_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_UART_1_TXD and CONFIG_PIN_CT32B2_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_UART_1_TXD and CONFIG_PIN_CT32B2_PWM1 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_SPI1_DMA_SO2) && defined(CONFIG_PIN_CT32B4_CAP0) && (CONFIG_PIN_SPI1_DMA_SO2 == CONFIG_PIN_CT32B4_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_SO2 and CONFIG_PIN_CT32B4_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_SO2 and CONFIG_PIN_CT32B4_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI1_DMA_SO2) && defined(CONFIG_PIN_CT32B2_PWM1) && (CONFIG_PIN_SPI1_DMA_SO2 == CONFIG_PIN_CT32B2_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_SO2 and CONFIG_PIN_CT32B2_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_SO2 and CONFIG_PIN_CT32B2_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CT32B4_CAP0) && defined(CONFIG_PIN_CT32B2_PWM1) && (CONFIG_PIN_CT32B4_CAP0 == CONFIG_PIN_CT32B2_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CT32B4_CAP0 and CONFIG_PIN_CT32B2_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_CT32B4_CAP0 and CONFIG_PIN_CT32B2_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_UART_1_RXD) && defined(CONFIG_PIN_SPI1_DMA_SO3) && (CONFIG_PIN_UART_1_RXD == CONFIG_PIN_SPI1_DMA_SO3)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_UART_1_RXD and CONFIG_PIN_SPI1_DMA_SO3 conflict !"
	#else
		#error "CONFIG_PIN_UART_1_RXD and CONFIG_PIN_SPI1_DMA_SO3 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_UART_1_RXD) && defined(CONFIG_PIN_CT32B5_CAP0) && (CONFIG_PIN_UART_1_RXD == CONFIG_PIN_CT32B5_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_UART_1_RXD and CONFIG_PIN_CT32B5_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_UART_1_RXD and CONFIG_PIN_CT32B5_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_UART_1_RXD) && defined(CONFIG_PIN_CT32B2_PWM0) && (CONFIG_PIN_UART_1_RXD == CONFIG_PIN_CT32B2_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_UART_1_RXD and CONFIG_PIN_CT32B2_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_UART_1_RXD and CONFIG_PIN_CT32B2_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI1_DMA_SO3) && defined(CONFIG_PIN_CT32B5_CAP0) && (CONFIG_PIN_SPI1_DMA_SO3 == CONFIG_PIN_CT32B5_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_SO3 and CONFIG_PIN_CT32B5_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_SO3 and CONFIG_PIN_CT32B5_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SPI1_DMA_SO3) && defined(CONFIG_PIN_CT32B2_PWM0) && (CONFIG_PIN_SPI1_DMA_SO3 == CONFIG_PIN_CT32B2_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SPI1_DMA_SO3 and CONFIG_PIN_CT32B2_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_SPI1_DMA_SO3 and CONFIG_PIN_CT32B2_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CT32B5_CAP0) && defined(CONFIG_PIN_CT32B2_PWM0) && (CONFIG_PIN_CT32B5_CAP0 == CONFIG_PIN_CT32B2_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CT32B5_CAP0 and CONFIG_PIN_CT32B2_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_CT32B5_CAP0 and CONFIG_PIN_CT32B2_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_AIN05) && defined(CONFIG_PIN_CT32B6_CAP0) && (CONFIG_PIN_AIN05 == CONFIG_PIN_CT32B6_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_AIN05 and CONFIG_PIN_CT32B6_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_AIN05 and CONFIG_PIN_CT32B6_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_AIN05) && defined(CONFIG_PIN_CT32B1_PWM2) && (CONFIG_PIN_AIN05 == CONFIG_PIN_CT32B1_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_AIN05 and CONFIG_PIN_CT32B1_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_AIN05 and CONFIG_PIN_CT32B1_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CT32B6_CAP0) && defined(CONFIG_PIN_CT32B1_PWM2) && (CONFIG_PIN_CT32B6_CAP0 == CONFIG_PIN_CT32B1_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CT32B6_CAP0 and CONFIG_PIN_CT32B1_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_CT32B6_CAP0 and CONFIG_PIN_CT32B1_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_AIN06) && defined(CONFIG_PIN_CT32B7_CAP0) && (CONFIG_PIN_AIN06 == CONFIG_PIN_CT32B7_CAP0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_AIN06 and CONFIG_PIN_CT32B7_CAP0 conflict !"
	#else
		#error "CONFIG_PIN_AIN06 and CONFIG_PIN_CT32B7_CAP0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_AIN06) && defined(CONFIG_PIN_CT32B1_PWM1) && (CONFIG_PIN_AIN06 == CONFIG_PIN_CT32B1_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_AIN06 and CONFIG_PIN_CT32B1_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_AIN06 and CONFIG_PIN_CT32B1_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_CT32B7_CAP0) && defined(CONFIG_PIN_CT32B1_PWM1) && (CONFIG_PIN_CT32B7_CAP0 == CONFIG_PIN_CT32B1_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_CT32B7_CAP0 and CONFIG_PIN_CT32B1_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_CT32B7_CAP0 and CONFIG_PIN_CT32B1_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SWO) && defined(CONFIG_PIN_CT32B0_PWM2) && (CONFIG_PIN_SWO == CONFIG_PIN_CT32B0_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SWO and CONFIG_PIN_CT32B0_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_SWO and CONFIG_PIN_CT32B0_PWM2 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SWD_CLK) && defined(CONFIG_PIN_CT32B0_PWM1) && (CONFIG_PIN_SWD_CLK == CONFIG_PIN_CT32B0_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SWD_CLK and CONFIG_PIN_CT32B0_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_SWD_CLK and CONFIG_PIN_CT32B0_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_SWD_DAT) && defined(CONFIG_PIN_CT32B0_PWM0) && (CONFIG_PIN_SWD_DAT == CONFIG_PIN_CT32B0_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_SWD_DAT and CONFIG_PIN_CT32B0_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_SWD_DAT and CONFIG_PIN_CT32B0_PWM0 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_I2C_1_CLK) && defined(CONFIG_PIN_CT32B3_PWM2) && (CONFIG_PIN_I2C_1_CLK == CONFIG_PIN_CT32B3_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2C_1_CLK and CONFIG_PIN_CT32B3_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_I2C_1_CLK and CONFIG_PIN_CT32B3_PWM2 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_I2C_1_DAT) && defined(CONFIG_PIN_CT32B3_PWM1) && (CONFIG_PIN_I2C_1_DAT == CONFIG_PIN_CT32B3_PWM1)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2C_1_DAT and CONFIG_PIN_CT32B3_PWM1 conflict !"
	#else
		#error "CONFIG_PIN_I2C_1_DAT and CONFIG_PIN_CT32B3_PWM1 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_0_SDIO) && defined(CONFIG_PIN_OID_2WIRE_CLK) && (CONFIG_PIN_I2S_0_SDIO == CONFIG_PIN_OID_2WIRE_CLK)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_0_SDIO and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#else
		#error "CONFIG_PIN_I2S_0_SDIO and CONFIG_PIN_OID_2WIRE_CLK conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_0_SDIO) && defined(CONFIG_PIN_CT32B3_PWM0) && (CONFIG_PIN_I2S_0_SDIO == CONFIG_PIN_CT32B3_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_0_SDIO and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_I2S_0_SDIO and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_OID_2WIRE_CLK) && defined(CONFIG_PIN_CT32B3_PWM0) && (CONFIG_PIN_OID_2WIRE_CLK == CONFIG_PIN_CT32B3_PWM0)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_2WIRE_CLK and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#else
		#error "CONFIG_PIN_OID_2WIRE_CLK and CONFIG_PIN_CT32B3_PWM0 conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_0_BCLK) && defined(CONFIG_PIN_OID_2WIRE_DATA) && (CONFIG_PIN_I2S_0_BCLK == CONFIG_PIN_OID_2WIRE_DATA)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_0_BCLK and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#else
		#error "CONFIG_PIN_I2S_0_BCLK and CONFIG_PIN_OID_2WIRE_DATA conflict !"
	#endif	
#endif
#if defined(CONFIG_PIN_I2S_0_BCLK) && defined(CONFIG_PIN_CT32B2_PWM2) && (CONFIG_PIN_I2S_0_BCLK == CONFIG_PIN_CT32B2_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_I2S_0_BCLK and CONFIG_PIN_CT32B2_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_I2S_0_BCLK and CONFIG_PIN_CT32B2_PWM2 conflict !"
	#endif
#endif
#if defined(CONFIG_PIN_OID_2WIRE_DATA) && defined(CONFIG_PIN_CT32B2_PWM2) && (CONFIG_PIN_OID_2WIRE_DATA == CONFIG_PIN_CT32B2_PWM2)
	#if IO_SHARE_PIN
		#warning "CONFIG_PIN_OID_2WIRE_DATA and CONFIG_PIN_CT32B2_PWM2 conflict !"
	#else
		#error "CONFIG_PIN_OID_2WIRE_DATA and CONFIG_PIN_CT32B2_PWM2 conflict !"
	#endif
#endif
