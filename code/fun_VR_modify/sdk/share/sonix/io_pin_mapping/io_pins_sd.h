// <<< Use Configuration Wizard in Context Menu >>>





#if (CONFIG_ENABLE_SD_0)

    #ifndef CONFIG_PIN_SD_NF_POWER_ON
        #define CONFIG_PIN_SD_NF_POWER_ON       IO_PIN_PORT_0_P06
    #endif

    #if (CONFIG_PINS_GROUP_ID_SD0 == 1)

        #define CONFIG_PIN_SD0_CLK          IO_PIN_PORT_1_P04
        #define CONFIG_PIN_SD0_CMD          IO_PIN_PORT_1_P05
        #define CONFIG_PIN_SD0_D0           IO_PIN_PORT_1_P06
        #define CONFIG_PIN_SD0_D1           IO_PIN_PORT_1_P07
        #define CONFIG_PIN_SD0_D2           IO_PIN_PORT_1_P08
        #define CONFIG_PIN_SD0_D3           IO_PIN_PORT_1_P09

    #elif (CONFIG_PINS_GROUP_ID_SD0 == 2)

        #define CONFIG_PIN_SD0_CLK          IO_PIN_PORT_3_P05
        #define CONFIG_PIN_SD0_CMD          IO_PIN_PORT_3_P06
        #define CONFIG_PIN_SD0_D0           IO_PIN_PORT_3_P07
        #define CONFIG_PIN_SD0_D1           IO_PIN_PORT_3_P08
        #define CONFIG_PIN_SD0_D2           IO_PIN_PORT_3_P09
        #define CONFIG_PIN_SD0_D3           IO_PIN_PORT_3_P10
    #endif

#endif


#if (CONFIG_ENABLE_SDIO)

    #ifndef CONFIG_PIN_SD_NF_POWER_ON
        #define CONFIG_PIN_SD_NF_POWER_ON       IO_PIN_PORT_0_P06
    #endif

    #define CONFIG_PIN_SD1_CLK          IO_PIN_PORT_1_P14
    #define CONFIG_PIN_SD1_CMD          IO_PIN_PORT_1_P15
    #define CONFIG_PIN_SD1_D0           IO_PIN_PORT_2_P00
    #define CONFIG_PIN_SD1_D1           IO_PIN_PORT_2_P01
    #define CONFIG_PIN_SD1_D2           IO_PIN_PORT_2_P02
    #define CONFIG_PIN_SD1_D3           IO_PIN_PORT_2_P03

#endif

// <<< end of configuration section >>>
