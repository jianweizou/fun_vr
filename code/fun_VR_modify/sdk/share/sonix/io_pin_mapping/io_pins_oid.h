// <<< Use Configuration Wizard in Context Menu >>>




#if (CONFIG_ENABLE_OID)

    #if (CONFIG_PINS_GROUP_ID_OID == 1)

        #define CONFIG_PIN_OID_CLK                  IO_PIN_PORT_1_P04
        #define CONFIG_PIN_OID_CMD                  IO_PIN_PORT_1_P05
        #define CONFIG_PIN_OID_DATA_0               IO_PIN_PORT_1_P06
        #define CONFIG_PIN_OID_DATA_1               IO_PIN_PORT_1_P07
        #define CONFIG_PIN_OID_2WIRE_CLK            IO_PIN_PORT_1_P08
        #define CONFIG_PIN_OID_2WIRE_DATA           IO_PIN_PORT_1_P09

    #elif (CONFIG_PINS_GROUP_ID_OID == 2)

        #define CONFIG_PIN_OID_CLK                  IO_PIN_PORT_2_P04
        #define CONFIG_PIN_OID_CMD                  IO_PIN_PORT_2_P05
        #define CONFIG_PIN_OID_DATA_0               IO_PIN_PORT_2_P06
        #define CONFIG_PIN_OID_DATA_1               IO_PIN_PORT_2_P07
        #define CONFIG_PIN_OID_2WIRE_CLK            IO_PIN_PORT_4_P10
        #define CONFIG_PIN_OID_2WIRE_DATA           IO_PIN_PORT_4_P11

    #endif

#endif



// <<< end of configuration section >>>
