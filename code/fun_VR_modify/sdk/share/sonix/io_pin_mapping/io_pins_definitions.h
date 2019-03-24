#define Stringize(L)          #L
#define MakeString(M, L)      M(L)
#define $PIN_NUM(x)           MakeString(Stringize, x)
#define CONFLICT(a)           "IO Pin Number(" $PIN_NUM(a) ")"
//#pragma message(CONFLICT(CONFIG_PIN_SD0_CLK))


#define IO_PIN_PORT_0_P00            0
#define IO_PIN_PORT_0_P01            1
#define IO_PIN_PORT_0_P02            2
#define IO_PIN_PORT_0_P03            3
#define IO_PIN_PORT_0_P04            4
#define IO_PIN_PORT_0_P05            5
#define IO_PIN_PORT_0_P06            6
#define IO_PIN_PORT_0_P07            7
#define IO_PIN_PORT_0_P08            8
#define IO_PIN_PORT_0_P09            9
#define IO_PIN_PORT_0_P10            10
#define IO_PIN_PORT_0_P11            11
#define IO_PIN_PORT_0_P12            12
#define IO_PIN_PORT_0_P13            13
#define IO_PIN_PORT_0_P14            14
#define IO_PIN_PORT_0_P15            15

#define IO_PIN_PORT_1_P00            16
#define IO_PIN_PORT_1_P01            17
#define IO_PIN_PORT_1_P02            18
#define IO_PIN_PORT_1_P03            19
#define IO_PIN_PORT_1_P04            20
#define IO_PIN_PORT_1_P05            21
#define IO_PIN_PORT_1_P06            22
#define IO_PIN_PORT_1_P07            23
#define IO_PIN_PORT_1_P08            24
#define IO_PIN_PORT_1_P09            25
#define IO_PIN_PORT_1_P10            26
#define IO_PIN_PORT_1_P11            27
#define IO_PIN_PORT_1_P12            28
#define IO_PIN_PORT_1_P13            29
#define IO_PIN_PORT_1_P14            30
#define IO_PIN_PORT_1_P15            31

#define IO_PIN_PORT_2_P00            32
#define IO_PIN_PORT_2_P01            33
#define IO_PIN_PORT_2_P02            34
#define IO_PIN_PORT_2_P03            35
#define IO_PIN_PORT_2_P04            36
#define IO_PIN_PORT_2_P05            37
#define IO_PIN_PORT_2_P06            38
#define IO_PIN_PORT_2_P07            39
#define IO_PIN_PORT_2_P08            40
#define IO_PIN_PORT_2_P09            41
#define IO_PIN_PORT_2_P10            42
#define IO_PIN_PORT_2_P11            43
#define IO_PIN_PORT_2_P12            44
#define IO_PIN_PORT_2_P13            45
#define IO_PIN_PORT_2_P14            46
#define IO_PIN_PORT_2_P15            47

#define IO_PIN_PORT_3_P00            48
#define IO_PIN_PORT_3_P01            49
#define IO_PIN_PORT_3_P02            50
#define IO_PIN_PORT_3_P03            51
#define IO_PIN_PORT_3_P04            52
#define IO_PIN_PORT_3_P05            53
#define IO_PIN_PORT_3_P06            54
#define IO_PIN_PORT_3_P07            55
#define IO_PIN_PORT_3_P08            56
#define IO_PIN_PORT_3_P09            57
#define IO_PIN_PORT_3_P10            58
#define IO_PIN_PORT_3_P11            59
#define IO_PIN_PORT_3_P12            60
#define IO_PIN_PORT_3_P13            61
#define IO_PIN_PORT_3_P14            62
#define IO_PIN_PORT_3_P15            63

#define IO_PIN_PORT_4_P00            64
#define IO_PIN_PORT_4_P01            65
#define IO_PIN_PORT_4_P02            66
#define IO_PIN_PORT_4_P03            67
#define IO_PIN_PORT_4_P04            68
#define IO_PIN_PORT_4_P05            69
#define IO_PIN_PORT_4_P06            70
#define IO_PIN_PORT_4_P07            71
#define IO_PIN_PORT_4_P08            72
#define IO_PIN_PORT_4_P09            73
#define IO_PIN_PORT_4_P10            74
#define IO_PIN_PORT_4_P11            75
#define IO_PIN_PORT_4_P12            76
#define IO_PIN_PORT_4_P13            77
#define IO_PIN_PORT_4_P14            78
#define IO_PIN_PORT_4_P15            79
