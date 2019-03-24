
////#AP6181_NVRAM_V1.1_01152013
////#adjuest PA parameter for g/n mode

#if 1
static const char wifi_nvram_image[] =
					"manfid=0x2d0"                                              "\x00"
					"prodid=0x492"                                              "\x00"
					"vendid=0x14e4"                                              "\x00"
					"devid=0x4343"                                              "\x00"
					"boardtype=0x0598"                                              "\x00"

					//# Board Revision is P307, same nvram file can be used for P304, P305, P306 and P307 as the tssi pa params used are same
					//#Please force the automatic RX PER data to the respective board directory if not using P307 board, for e.g. for P305 boards force the data into the following directory /projects/BCM43362/a1_labdata/boardtests/results/sdg_rev0305

					"boardrev=0x1307"                                              "\x00"
					"boardnum=777"                                              "\x00"
					"xtalfreq=26000"                                              "\x00"
					"boardflags=0xa00"                                              "\x00"
					"sromrev=3"                                              "\x00"
					"wl0id=0x431b"                                              "\x00"
					"macaddr=00:90:4c:07:71:12"                                              "\x00"
					"ag0=2"                                              "\x00"
					"maxp2ga0=74"                                              "\x00"
					"cck2gpo=0x2222"                                              "\x00"
					"ofdm2gpo=0x66666666"                                              "\x00"
					"mcs2gpo0=0x7777"                                              "\x00"
					"mcs2gpo1=0x7777"                                              "\x00"
					"pa0maxpwr=56"                                              "\x00"

//					//#P207 PA params
//					//#pa0b0=5447
//					//#pa0b1=-658
//					//#pa0b2=-175<div></div>

//					//#Same PA params for P304,P305, P306, P307

					"pa0b0=5447"                                              "\x00"
					"pa0b1=-607"                                              "\x00"
					"pa0b2=-160"                                              "\x00"
					"pa0itssit=62"                                              "\x00"
					"pa1itssit=62"                                              "\x00"


					"cckPwrOffset=5"                                              "\x00"
					"ccode=0"                                              "\x00"
					"rssismf2g=0xa"                                              "\x00"
					"rssismc2g=0x3"                                              "\x00"
					"rssisav2g=0x7"                                              "\x00"
					"triso2g=0"                                              "\x00"
					"noise_cal_enable_2g=0"                                              "\x00"
					"noise_cal_po_2g=0"                                              "\x00"
					"swctrlmap_2g=0x04040404,0x02020202,0x02020202,0x010101,0x1ff"                                              "\x00"
					"temp_add=29767"                                              "\x00"
					"temp_mult=425"                                              "\x00"
					"\x00\x00";
#else
const __align(32)char wifi_nvram_image[] =
        "manfid=0x2d0"                                              "\x00"
        "prodid=0x492"                                              "\x00"
        "vendid=0x14e4"                                             "\x00"
        "devid=0x4343"                                              "\x00"
        "boardtype=0x0636"                                          "\x00"
        "boardrev=0x1201"                                           "\x00"
        "boardnum=777"                                              "\x00"
        "xtalfreq=26000"                                            "\x00"
        "boardflags=0xa00"                                          "\x00"
        "sromrev=3"                                                 "\x00"
        "wl0id=0x431b"                                              "\x00"
        "macaddr=00:90:4c:07:71:12"                                 "\x00"
        "aa2g=3"                                                    "\x00"
        "ag0=2"                                                     "\x00"
        "maxp2ga0=74"                                               "\x00"
        "ofdm2gpo=0x44111111"                                       "\x00"
        "mcs2gpo0=0x4444"                                           "\x00"
        "mcs2gpo1=0x6444"                                           "\x00"
        "pa0maxpwr=80"                                              "\x00"
        "pa0b0=5031"                                                "\x00"  /*PA params*/
        "pa0b1=-570"                                                "\x00"
        "pa0b2=-107"                                                "\x00"
        "pa0itssit=62"                                              "\x00"
        "pa1itssit=62"                                              "\x00"
        "temp_based_dutycy_en=1"                                    "\x00"
        "tx_duty_cycle_ofdm=100"                                    "\x00"
        "tx_duty_cycle_cck=100"                                     "\x00"
        "tx_ofdm_temp_0=115"                                        "\x00"
        "tx_cck_temp_0=115"                                         "\x00"
        "tx_ofdm_dutycy_0=40"                                       "\x00"
        "tx_cck_dutycy_0=40"                                        "\x00"
        "tx_ofdm_temp_1=255"                                        "\x00"
        "tx_cck_temp_1=255"                                         "\x00"
        "tx_ofdm_dutycy_1=40"                                       "\x00"
        "tx_cck_dutycy_1=40"                                        "\x00"
        "tx_tone_power_index=40"                                    "\x00"
        "tx_tone_power_index.fab.3=48"                              "\x00"
        "cckPwrOffset=0"                                            "\x00"
        "ccode=0"                                                   "\x00"
        "rssismf2g=0xa"                                             "\x00"
        "rssismc2g=0x3"                                             "\x00"
        "rssisav2g=0x7"                                             "\x00"
        "triso2g=0"                                                 "\x00"
        "noise_cal_enable_2g=0"                                     "\x00"
        "noise_cal_po_2g=0"                                         "\x00"
        "noise_cal_po_2g.fab.3=-2"                                  "\x00"
        "swctrlmap_2g=0x030a030a,0x050c050c,0x050c050c,0x0,0x1ff"   "\x00"
        "temp_add=29767"                                            "\x00"
        "temp_mult=425"                                             "\x00"
        "temp_q=10"                                                 "\x00"
        "initxidx2g=45"                                             "\x00"
        "tssitime=1"                                                "\x00"
        "rfreg033=0x19"                                             "\x00"
        "rfreg033_cck=0x1f"                                         "\x00"
        "cckPwrIdxCorr=-8"                                          "\x00"
        "spuravoid_enable2g=1"                                      "\x00"
        "\x00\x00";

#endif
