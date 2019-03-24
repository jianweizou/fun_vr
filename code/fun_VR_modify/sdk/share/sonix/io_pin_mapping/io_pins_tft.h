// <<< Use Configuration Wizard in Context Menu >>>




#if (CONFIG_ENABLE_TFT)

    #if (CONFIG_PINS_GROUP_ID_TFT == 1)

        #define CONFIG_PIN_HSYNC           IO_PIN_PORT_2_P04
        #define CONFIG_PIN_VSYNC           IO_PIN_PORT_2_P05
        #define CONFIG_PIN_DE              IO_PIN_PORT_2_P06
        #define CONFIG_PIN_DCLK            IO_PIN_PORT_2_P07
        #define CONFIG_PIN_ED0             IO_PIN_PORT_2_P08
        #define CONFIG_PIN_ED1             IO_PIN_PORT_2_P09
        #define CONFIG_PIN_ED2             IO_PIN_PORT_2_P10
        #define CONFIG_PIN_ED3             IO_PIN_PORT_2_P11
        #define CONFIG_PIN_ED4             IO_PIN_PORT_2_P12
        #define CONFIG_PIN_ED5             IO_PIN_PORT_2_P13
        #define CONFIG_PIN_ED6             IO_PIN_PORT_2_P14
        #define CONFIG_PIN_ED7             IO_PIN_PORT_2_P15

    #elif (CONFIG_PINS_GROUP_ID_NAND == 2)

        #define CONFIG_PIN_HSYNC           IO_PIN_PORT_1_P02
        #define CONFIG_PIN_VSYNC           IO_PIN_PORT_1_P03
        #define CONFIG_PIN_DE              IO_PIN_PORT_1_P04
        #define CONFIG_PIN_DCLK            IO_PIN_PORT_1_P05
        #define CONFIG_PIN_ED0             IO_PIN_PORT_1_P06
        #define CONFIG_PIN_ED1             IO_PIN_PORT_1_P07
        #define CONFIG_PIN_ED2             IO_PIN_PORT_1_P08
        #define CONFIG_PIN_ED3             IO_PIN_PORT_1_P09
        #define CONFIG_PIN_ED4             IO_PIN_PORT_1_P10
        #define CONFIG_PIN_ED5             IO_PIN_PORT_1_P11
        #define CONFIG_PIN_ED6             IO_PIN_PORT_1_P12
        #define CONFIG_PIN_ED7             IO_PIN_PORT_1_P13
    #endif

#endif

#if defined(CONFIG_ENABLE_TFT_16_BITS) && (CONFIG_ENABLE_TFT_16_BITS)

    #define CONFIG_PIN_LCM_RE        CONFIG_PIN_HSYNC
    #define CONFIG_PIN_LCM_WE        CONFIG_PIN_VSYNC
    #define CONFIG_PIN_LCM_CS        CONFIG_PIN_DE
    #define CONFIG_PIN_EA0           CONFIG_PIN_DCLK
    #define CONFIG_PIN_ED8           IO_PIN_PORT_3_P00
    #define CONFIG_PIN_ED9           IO_PIN_PORT_3_P01
    #define CONFIG_PIN_ED10          IO_PIN_PORT_3_P02
    #define CONFIG_PIN_ED11          IO_PIN_PORT_3_P03
    #define CONFIG_PIN_ED12          IO_PIN_PORT_3_P04
    #define CONFIG_PIN_ED13          IO_PIN_PORT_3_P05
    #define CONFIG_PIN_ED14          IO_PIN_PORT_3_P06
    #define CONFIG_PIN_ED15          IO_PIN_PORT_3_P07

#endif

#if defined(CONFIG_ENABLE_TFT_16_BITS) && defined(CONFIG_ENABLE_TFT_18_BITS) && \
    (CONFIG_ENABLE_TFT_16_BITS) && (CONFIG_ENABLE_TFT_18_BITS)

    #define CONFIG_PIN_ED16          IO_PIN_PORT_3_P08
    #define CONFIG_PIN_ED17          IO_PIN_PORT_3_P09
#endif



// <<< end of configuration section >>>
