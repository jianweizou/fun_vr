#include "bootloader.h"

void memcpy_reload(void *, void *, uint32_t);

void Reload_PRAM(stFwHeader_t *pHeader, stProjectInfo_t *pMode)
{
    uint32_t source;
    uint32_t dest;
    uint32_t size;

    /* Disable All Interrupts before reload Pram */
    __set_PRIMASK(1);

    for(int i=0; i<pMode->RomNum;i++)
    {
        
        source 	= pMode->Rom[i].AddrOffset + (uint32_t)pHeader; 
        dest 	= pMode->Rom[i].DestAddr;
        size 	= pMode->Rom[i].Size;
        
        if(pMode->Rom[i].DestAddr >= _IC_FLASH_REGION_BASE)
            continue;
        
        memcpy_reload((void*)dest ,(void*)source,size/4);
    }

  NVIC_SystemReset();                                 

  while(1);
}

void memcpy_reload(void *dest, void *sour, uint32_t Length)
{
   uint32_t Cnt;
   uint32_t *Dest = (uint32_t *)dest;
   uint32_t *Sour = (uint32_t *)sour;
   
   for(Cnt = 0; Cnt < Length; Cnt++)
      *(Dest+Cnt) = *(Sour+Cnt);
}

