#ifndef __WIFI_STREAM_H__
#define __WIFI_STREAM_H__

#define MAX_BUF_LEN 1024*24
#define WIFI_STREAM_IPADDR "192.168.99.2"
#define WIFI_STREAM_PORT 9963
 
/* Extern declaration */
int wifi_stream_init(char *, int);
int wifi_stream_send(int , unsigned char *, unsigned int);
int wifi_stream_destory(int);
void WIFI_STREAM_VERIFY_TASK(void *);
#endif
