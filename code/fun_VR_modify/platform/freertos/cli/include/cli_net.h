void vRegisterNetCLICommands( void );

/* Struct with settings for each throughput task */
typedef struct _paramtp
{
    char ip_addr[16];               /* ip address */
    short  port;                    /* port number */
    unsigned int testinsec;         /* total test time in seconds */
} paramtpStruct;


