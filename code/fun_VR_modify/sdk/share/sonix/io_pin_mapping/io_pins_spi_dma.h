// <<< Use Configuration Wizard in Context Menu >>>



#if (CONFIG_ENABLE_SPI_DMA_0)

    #if (CONFIG_PINS_GROUP_ID_SPI_DMA_0 == 1)

        #define CONFIG_PIN_SPI0_DMA_CS            IO_PIN_PORT_0_P08
        #define CONFIG_PIN_SPI0_DMA_SCK           IO_PIN_PORT_0_P09
        #define CONFIG_PIN_SPI0_DMA_SO            IO_PIN_PORT_0_P10
        #define CONFIG_PIN_SPI0_DMA_SI            IO_PIN_PORT_0_P11
        #define CONFIG_PIN_SPI0_DMA_SO2           IO_PIN_PORT_0_P12
        #define CONFIG_PIN_SPI0_DMA_SO3           IO_PIN_PORT_0_P13


    #elif (CONFIG_PINS_GROUP_ID_SPI_DMA_0 == 2)

        #define CONFIG_PIN_SPI0_DMA_CS            IO_PIN_PORT_1_P04
        #define CONFIG_PIN_SPI0_DMA_SCK           IO_PIN_PORT_1_P05
        #define CONFIG_PIN_SPI0_DMA_SO            IO_PIN_PORT_1_P06
        #define CONFIG_PIN_SPI0_DMA_SI            IO_PIN_PORT_1_P07
        #define CONFIG_PIN_SPI0_DMA_SO2           IO_PIN_PORT_1_P08
        #define CONFIG_PIN_SPI0_DMA_SO3           IO_PIN_PORT_1_P09

    #endif

#endif


#if (CONFIG_ENABLE_SPI_DMA_1)

    #define CONFIG_PIN_SPI1_DMA_CS            IO_PIN_PORT_3_P12
    #define CONFIG_PIN_SPI1_DMA_SCK           IO_PIN_PORT_3_P13
    #define CONFIG_PIN_SPI1_DMA_SO            IO_PIN_PORT_3_P14
    #define CONFIG_PIN_SPI1_DMA_SI            IO_PIN_PORT_3_P15
    #define CONFIG_PIN_SPI1_DMA_SO2           IO_PIN_PORT_4_P00
    #define CONFIG_PIN_SPI1_DMA_SO3           IO_PIN_PORT_4_P01
#endif

// <<< end of configuration section >>>
