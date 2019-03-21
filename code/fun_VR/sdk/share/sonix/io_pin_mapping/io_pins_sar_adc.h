// <<< Use Configuration Wizard in Context Menu >>>




#if (CONFIG_ENABLE_SAR_ADC)

    #if (CONFIG_PINS_GROUP_ID_SAR_ADC_AIN01)
        #define CONFIG_PIN_AIN01           IO_PIN_PORT_0_P04
    #else
        #define CONFIG_PIN_AIN01           -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_SAR_ADC_AIN02)
        #define CONFIG_PIN_AIN02           IO_PIN_PORT_0_P05
    #else
        #define CONFIG_PIN_AIN02           -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_SAR_ADC_AIN03)
        #define CONFIG_PIN_AIN03           IO_PIN_PORT_0_P06
    #else
        #define CONFIG_PIN_AIN03           -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_SAR_ADC_AIN04)
        #define CONFIG_PIN_AIN04           IO_PIN_PORT_0_P07
    #else
        #define CONFIG_PIN_AIN04           -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_SAR_ADC_AIN05)
        #define CONFIG_PIN_AIN05           IO_PIN_PORT_4_P02
    #else
        #define CONFIG_PIN_AIN05           -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_SAR_ADC_AIN06)
        #define CONFIG_PIN_AIN06           IO_PIN_PORT_4_P03
    #else
        #define CONFIG_PIN_AIN06           -1
    #endif

#endif


// <<< end of configuration section >>>
