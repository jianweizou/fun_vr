// <<< Use Configuration Wizard in Context Menu >>>


#if (CONFIG_ENABLE_CT32B0_PWM)
    #if (CONFIG_PINS_GROUP_ID_CT32B0_PWM0 == 1)
        #define CONFIG_PIN_CT32B0_PWM0         IO_PIN_PORT_4_P07
    #elif (CONFIG_PINS_GROUP_ID_CT32B0_PWM0 == 2)
        #define CONFIG_PIN_CT32B0_PWM0         IO_PIN_PORT_0_P13
    #else
        #define CONFIG_PIN_CT32B0_PWM0         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B0_PWM1 == 1)
        #define CONFIG_PIN_CT32B0_PWM1         IO_PIN_PORT_4_P06
    #elif (CONFIG_PINS_GROUP_ID_CT32B0_PWM1 == 2)
        #define CONFIG_PIN_CT32B0_PWM1         IO_PIN_PORT_0_P12
    #else
        #define CONFIG_PIN_CT32B0_PWM1         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B0_PWM2)
        #define CONFIG_PIN_CT32B0_PWM2         IO_PIN_PORT_4_P05
    #else
        #define CONFIG_PIN_CT32B0_PWM2         -1
    #endif
#endif

#if (CONFIG_ENABLE_CT32B1_PWM)
    #if (CONFIG_PINS_GROUP_ID_CT32B1_PWM0)
        #define CONFIG_PIN_CT32B1_PWM0         IO_PIN_PORT_4_P04
    #else
        #define CONFIG_PIN_CT32B1_PWM0         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B1_PWM1)
        #define CONFIG_PIN_CT32B1_PWM1         IO_PIN_PORT_4_P03
    #else
        #define CONFIG_PIN_CT32B1_PWM1         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B1_PWM2)
        #define CONFIG_PIN_CT32B1_PWM2         IO_PIN_PORT_4_P02
    #else
        #define CONFIG_PIN_CT32B1_PWM2         -1
    #endif
#endif

#if (CONFIG_ENABLE_CT32B2_PWM)
    #if (CONFIG_PINS_GROUP_ID_CT32B2_PWM0)
        #define CONFIG_PIN_CT32B2_PWM0         IO_PIN_PORT_4_P01
    #else
        #define CONFIG_PIN_CT32B2_PWM0         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B2_PWM0)
        #define CONFIG_PIN_CT32B2_PWM1         IO_PIN_PORT_4_P00
    #else
        #define CONFIG_PIN_CT32B2_PWM1         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B2_PWM2 == 1)
        #define CONFIG_PIN_CT32B2_PWM2         IO_PIN_PORT_3_P15
    #elif (CONFIG_PINS_GROUP_ID_CT32B2_PWM2 == 2)
        #define CONFIG_PIN_CT32B2_PWM2         IO_PIN_PORT_4_P11
    #else
        #define CONFIG_PIN_CT32B2_PWM2         -1
    #endif

#endif

#if (CONFIG_ENABLE_CT32B3_PWM)

    #if (CONFIG_PINS_GROUP_ID_CT32B3_PWM0 == 1)
        #define CONFIG_PIN_CT32B3_PWM0         IO_PIN_PORT_3_P14
    #elif (CONFIG_PINS_GROUP_ID_CT32B3_PWM0 == 2)
        #define CONFIG_PIN_CT32B3_PWM0         IO_PIN_PORT_4_P10
    #else
        #define CONFIG_PIN_CT32B3_PWM0         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B3_PWM1 == 1)
        #define CONFIG_PIN_CT32B3_PWM1         IO_PIN_PORT_3_P13
    #elif (CONFIG_PINS_GROUP_ID_CT32B3_PWM1 == 2)
        #define CONFIG_PIN_CT32B3_PWM1         IO_PIN_PORT_4_P09
    #else
        #define CONFIG_PIN_CT32B3_PWM1         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B3_PWM2 == 1)
        #define CONFIG_PIN_CT32B3_PWM2         IO_PIN_PORT_3_P12
    #elif (CONFIG_PINS_GROUP_ID_CT32B3_PWM2 == 2)
        #define CONFIG_PIN_CT32B3_PWM2         IO_PIN_PORT_4_P08
    #else
        #define CONFIG_PIN_CT32B3_PWM2         -1
    #endif

#endif

#if (CONFIG_ENABLE_CT32B4_PWM)
    #if (CONFIG_PINS_GROUP_ID_CT32B4_PWM0)
        #define CONFIG_PIN_CT32B4_PWM0         IO_PIN_PORT_3_P11
    #else
        #define CONFIG_PIN_CT32B4_PWM0         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B4_PWM1)
        #define CONFIG_PIN_CT32B4_PWM1         IO_PIN_PORT_3_P10
    #else
        #define CONFIG_PIN_CT32B4_PWM1         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B4_PWM2)
        #define CONFIG_PIN_CT32B4_PWM2         IO_PIN_PORT_2_P07
    #else
        #define CONFIG_PIN_CT32B4_PWM2         -1
    #endif
#endif

#if (CONFIG_ENABLE_CT32B5_PWM)
    #if (CONFIG_PINS_GROUP_ID_CT32B5_PWM0)
        #define CONFIG_PIN_CT32B5_PWM0         IO_PIN_PORT_2_P06
    #else
        #define CONFIG_PIN_CT32B5_PWM0         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B5_PWM1)
        #define CONFIG_PIN_CT32B5_PWM1         IO_PIN_PORT_2_P05
    #else
        #define CONFIG_PIN_CT32B5_PWM1         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B5_PWM0)
        #define CONFIG_PIN_CT32B5_PWM2         IO_PIN_PORT_2_P04
    #else
        #define CONFIG_PIN_CT32B5_PWM2         -1
    #endif
#endif

#if (CONFIG_ENABLE_CT32B6_PWM)

    #if (CONFIG_PINS_GROUP_ID_CT32B6_PWM0 == 1)
        #define CONFIG_PIN_CT32B6_PWM0         IO_PIN_PORT_1_P09
    #elif (CONFIG_PINS_GROUP_ID_CT32B6_PWM0 == 2)
        #define CONFIG_PIN_CT32B6_PWM0         IO_PIN_PORT_0_P07
    #else
        #define CONFIG_PIN_CT32B6_PWM0         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B6_PWM1 == 1)
        #define CONFIG_PIN_CT32B6_PWM1         IO_PIN_PORT_1_P08
    #elif (CONFIG_PINS_GROUP_ID_CT32B6_PWM1 == 2)
        #define CONFIG_PIN_CT32B6_PWM1         IO_PIN_PORT_0_P06
    #else
        #define CONFIG_PIN_CT32B6_PWM1         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B6_PWM2 == 1)
        #define CONFIG_PIN_CT32B6_PWM2         IO_PIN_PORT_1_P07
    #elif (CONFIG_PINS_GROUP_ID_CT32B6_PWM2 == 2)
        #define CONFIG_PIN_CT32B6_PWM2         IO_PIN_PORT_0_P05
    #else
        #define CONFIG_PIN_CT32B6_PWM2         -1
    #endif

#endif

#if (CONFIG_ENABLE_CT32B7_PWM)

    #if (CONFIG_PINS_GROUP_ID_CT32B7_PWM0 == 1)
        #define CONFIG_PIN_CT32B7_PWM0         IO_PIN_PORT_1_P06
    #elif (CONFIG_PINS_GROUP_ID_CT32B7_PWM0 == 2)
        #define CONFIG_PIN_CT32B7_PWM0         IO_PIN_PORT_0_P04
    #else
        #define CONFIG_PIN_CT32B7_PWM0         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B7_PWM1 == 1)
        #define CONFIG_PIN_CT32B7_PWM1         IO_PIN_PORT_1_P05
    #elif (CONFIG_PINS_GROUP_ID_CT32B7_PWM1 == 2)
        #define CONFIG_PIN_CT32B7_PWM1         IO_PIN_PORT_0_P03
    #else
        #define CONFIG_PIN_CT32B7_PWM1         -1
    #endif

    #if (CONFIG_PINS_GROUP_ID_CT32B7_PWM2 == 1)
        #define CONFIG_PIN_CT32B7_PWM2         IO_PIN_PORT_1_P04
    #elif (CONFIG_PINS_GROUP_ID_CT32B7_PWM2 == 2)
        #define CONFIG_PIN_CT32B7_PWM2         IO_PIN_PORT_0_P02
    #else
        #define CONFIG_PIN_CT32B7_PWM2         -1
    #endif

#endif


#if (CONFIG_ENABLE_CT32B0_CAP)
    #define CONFIG_PIN_CT32B0_CAP0      IO_PIN_PORT_3_P12
#endif

#if (CONFIG_ENABLE_CT32B1_CAP)
    #define CONFIG_PIN_CT32B1_CAP0      IO_PIN_PORT_3_P13
#endif

#if (CONFIG_ENABLE_CT32B2_CAP)
    #define CONFIG_PIN_CT32B2_CAP0      IO_PIN_PORT_3_P14
#endif

#if (CONFIG_ENABLE_CT32B3_CAP)
    #define CONFIG_PIN_CT32B3_CAP0      IO_PIN_PORT_3_P15
#endif

#if (CONFIG_ENABLE_CT32B4_CAP)
    #define CONFIG_PIN_CT32B4_CAP0      IO_PIN_PORT_4_P00
#endif

#if (CONFIG_ENABLE_CT32B5_CAP)
    #define CONFIG_PIN_CT32B5_CAP0      IO_PIN_PORT_4_P01
#endif

#if (CONFIG_ENABLE_CT32B6_CAP)
    #define CONFIG_PIN_CT32B6_CAP0      IO_PIN_PORT_4_P02
#endif

#if (CONFIG_ENABLE_CT32B7_CAP)
    #define CONFIG_PIN_CT32B7_CAP0      IO_PIN_PORT_4_P03
#endif


// <<< end of configuration section >>>
