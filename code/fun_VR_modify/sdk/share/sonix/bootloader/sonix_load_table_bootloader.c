
#include <stdint.h>

/* Please maske sure  */
#define LOAD_TABLE_ADDR         0x60000000

typedef struct
{
    uint32_t MARK[2];           /* Offset: 0x00 - Fixed */
    uint32_t LOAD_CFG;          /* Offset: 0x08 - Fixed */
    uint32_t DEVICE_CFG;        /* Offset: 0x0C - Add */
    uint32_t ADDR_USERCODE;     /* Offset: 0x10 - Fixed */
    uint32_t SIZE_USERCODE;     /* Offset: 0x14 */
    uint32_t RESERVED1[4];      /* Offset: 0x18 */
    uint32_t AES_KEY[8];        /* Offset: 0x28 - Fixed */
    
}load_table_t;

//;*** <<< Use Configuration Wizard in Context Menu >>> ***
//;<h> SONiX SNC7320 Load Table Configuration


const load_table_t __attribute((at(LOAD_TABLE_ADDR))) 
sonix_load_table = 
{
// * -------------------------------------------------------------------
// *   <o2> PRAM image address
// *       <i>Address of the first load program
// *   <o3> PRAM image size
// *       <0x10000=> 64 kBytes
// *       <0x08000=> 32 kBytes
// *       <0x04000=> 16 kBytes
// *       <0x02000=>  8 kBytes
// *       <i>Size of the first load program
// *   <o0.3> Accelerate booting

// * -------------------------------------------------------------------
// *   <e0.0> AES Decryption
// *       <i> Decrypt PRAM data before load.
// *       <o5>AES Key: Byte 0-3
// *       <o7>AES Key: Byte 4-7
// *       <o9>AES Key: Byte 8-11
// *       <o11>AES Key: Byte 12-15
// *       <o13>AES Key: Byte 16-19
// *       <o15>AES Key: Byte 20-23
// *       <o17>AES Key: Byte 24-27
// *       <o19>AES Key: Byte 28-31
// *   </e>
// * -------------------------------------------------------------------
// *   <e0.16> Boot device selection
// *       <i> Select the first priority device
// *       <o1.16..19> Boot From Device:
// *           <0x00=> SPI NOR Flash
// *           <0x05=> SD Card#0 (SD)
// *           <0x02=> SD Card#1 (SDIO)
// *           <0x03=> Parallel NAND Flash 
// *           <0x04=> Serial NAND Flash 
// *           <0x0A=> ISP Mode (USB)
// *
// *   </e>
// **/
    
    
// * Load configuration ---------------------------------------------------
     .LOAD_CFG          = 0x0FFE8008,          //Boot configuration
     .DEVICE_CFG        = 0x00020000,          //Boot redirect setting

// * --- PRAM Info --------------------------------------------------------
     .ADDR_USERCODE     = 0x60001000,          //Pram binary file start address
     .SIZE_USERCODE     = 0x00010000,          //Pram binary file maximun size

// * --- AES Key ----------------------------------------------------------
     .AES_KEY[0]        = 0x11111111,          //AES Key 0
     .AES_KEY[1]        = 0x22222222,          //AES Key 1          
     .AES_KEY[2]        = 0x33333333,          //AES Key 2
     .AES_KEY[3]        = 0x44444444,          //AES Key 3
     .AES_KEY[4]        = 0x55555555,          //AES Key 4
     .AES_KEY[5]        = 0x66666666,          //AES Key 5
     .AES_KEY[6]        = 0x77777777,          //AES Key 6
     .AES_KEY[7]        = 0x88888888,          //AES Key 7
     
         
// * Sonix 7320 identify tag-----------------------------------------------
     .MARK[0]           = 0x37434E53,          //"SNC7"
     .MARK[1]           = 0x41303233,          //"320A"
};



//; </h>
//;*** <<< end of configuration section >>>    ***




