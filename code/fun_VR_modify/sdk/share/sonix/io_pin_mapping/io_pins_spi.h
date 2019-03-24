// <<< Use Configuration Wizard in Context Menu >>>



#if (CONFIG_ENABLE_SPI_0)

    #define CONFIG_PIN_SPIIF_0_CS               IO_PIN_PORT_0_P14
    #define CONFIG_PIN_SPIIF_0_SCK              IO_PIN_PORT_0_P15
    #define CONFIG_PIN_SPIIF_0_MISO             IO_PIN_PORT_1_P00
    #define CONFIG_PIN_SPIIF_0_MOSI             IO_PIN_PORT_1_P01

#endif


#if (CONFIG_ENABLE_SPI_1)

    #define CONFIG_PIN_SPIIF_1_CS               IO_PIN_PORT_3_P12
    #define CONFIG_PIN_SPIIF_1_SCK              IO_PIN_PORT_3_P13
    #define CONFIG_PIN_SPIIF_1_MISO             IO_PIN_PORT_3_P14
    #define CONFIG_PIN_SPIIF_1_MOSI             IO_PIN_PORT_3_P15
#endif

// <<< end of configuration section >>>
