// <<< Use Configuration Wizard in Context Menu >>>



#if (CONFIG_ENABLE_I2S_0)

    #define CONFIG_PIN_I2S_0_SDIO          IO_PIN_PORT_4_P10
    #define CONFIG_PIN_I2S_0_BCLK          IO_PIN_PORT_4_P11
    #define CONFIG_PIN_I2S_0_WS            IO_PIN_PORT_4_P12

#endif


#if (CONFIG_ENABLE_I2S_1)

	#define CONFIG_PIN_I2S_1_SDIO          IO_PIN_PORT_3_P04
	#define CONFIG_PIN_I2S_1_BCLK          IO_PIN_PORT_3_P05
	#define CONFIG_PIN_I2S_1_WS            IO_PIN_PORT_3_P06

#endif


#if (CONFIG_ENABLE_I2S_2)

    #define CONFIG_PIN_I2S_2_SDIO          IO_PIN_PORT_4_P13
    #define CONFIG_PIN_I2S_2_BCLK          IO_PIN_PORT_4_P14
    #define CONFIG_PIN_I2S_2_WS            IO_PIN_PORT_4_P15
#endif

#if (CONFIG_ENABLE_I2S_3)

    #define CONFIG_PIN_I2S_3_SDIO          IO_PIN_PORT_3_P07
    #define CONFIG_PIN_I2S_3_BCLK          IO_PIN_PORT_3_P08
    #define CONFIG_PIN_I2S_3_WS            IO_PIN_PORT_3_P09
#endif

#if (CONFIG_ENABLE_I2S_4)

    #if (CONFIG_PINS_GROUP_ID_I2S_4 == 1)

        #define CONFIG_PIN_I2S_4_MCLK              IO_PIN_PORT_1_P14
        #define CONFIG_PIN_I2S_4_SDIN              IO_PIN_PORT_1_P15
        #define CONFIG_PIN_I2S_4_SDOUT             IO_PIN_PORT_2_P00
        #define CONFIG_PIN_I2S_4_BCLK              IO_PIN_PORT_2_P01
        #define CONFIG_PIN_I2S_4_WS                IO_PIN_PORT_2_P02

    #elif (CONFIG_PINS_GROUP_ID_I2S_4 == 2)

        #define CONFIG_PIN_I2S_4_MCLK              IO_PIN_PORT_3_P02
        #define CONFIG_PIN_I2S_4_SDIN              IO_PIN_PORT_3_P03
        #define CONFIG_PIN_I2S_4_SDOUT             IO_PIN_PORT_3_P04
        #define CONFIG_PIN_I2S_4_BCLK              IO_PIN_PORT_3_P05
        #define CONFIG_PIN_I2S_4_WS                IO_PIN_PORT_3_P06
    #elif (CONFIG_PINS_GROUP_ID_I2S_4 == 3)

        #define CONFIG_PIN_I2S_4_MCLK              IO_PIN_PORT_2_P04
        #define CONFIG_PIN_I2S_4_SDIN              IO_PIN_PORT_2_P08
        #define CONFIG_PIN_I2S_4_SDOUT             IO_PIN_PORT_2_P05
        #define CONFIG_PIN_I2S_4_BCLK              IO_PIN_PORT_2_P06
        #define CONFIG_PIN_I2S_4_WS                IO_PIN_PORT_2_P07
    #endif

#endif

// <<< end of configuration section >>>
