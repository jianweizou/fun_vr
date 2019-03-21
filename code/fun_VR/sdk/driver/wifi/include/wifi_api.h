#if CONFIG_MODULE_WIFI_BCM43362
int bcm43362_init(void);
int bcm43362_sta_join(char *ssid, char *key);
int bcm43362_sta_leave(void);
int bcm43362_ap_start(char *ssid, char *security, char* key);
int bcm43362_ap_stop(void);
void bcm43362_scan(void);
#endif
#if CONFIG_MODULE_WIFI_MT7601
int mt7601_stajoin(char* ssid, char* pwd);
int mt7601_apstart(char* ssid, char* pwd, int security);
#endif
