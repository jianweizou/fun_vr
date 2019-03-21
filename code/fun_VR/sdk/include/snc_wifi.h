/* BCM43362 API */
int bcm43362_init(void);
int bcm43362_sta_join(char *ssid, char *key);
int bcm43362_sta_leave(void);
int bcm43362_ap_start(char *ssid, char *security, char* key);
int bcm43362_ap_stop(void);
void bcm43362_scan(void);
/* MT7601 API */
int mt7601_stajoin(char* ssid, char* pwd);
int mt7601_apstart(char* ssid, char* pwd, int security);

