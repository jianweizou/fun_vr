
#include <stdint.h>

/* DLO Table address */
#define DLO_TABLE_ADDR         0x60000100

typedef struct dlo_table
{
    uint32_t    load_str;
    void*       load_ro_base;
    void*       exec_ro_base;
    void*       ro_length;
    
    void*       load_rw_base;
    void*       exec_rw_base;
    void*       rw_length;
    
    void*       exec_zi_base;
    void*       zi_length;
} dlo_table_t;

extern void Load$$DLO_REGION$$RO$$Base(void);
extern void Load$$DLO_REGION$$RW$$Base(void);

extern void Image$$DLO_REGION$$RO$$Length(void);
extern void Image$$DLO_REGION$$RW$$Length(void);
extern void Image$$DLO_REGION$$ZI$$Length(void);

extern void Image$$DLO_REGION$$RO$$Base(void);
extern void Image$$DLO_REGION$$RW$$Base(void);
extern void Image$$DLO_REGION$$ZI$$Base(void);

//;*** <<< Use Configuration Wizard in Context Menu >>> ***
const dlo_table_t __attribute((at(DLO_TABLE_ADDR))) 
dlo_table = 
{
    /* ASCII string "load"    */
    .load_str           = 0x64616F6C,               
    
    /* RO info */
    .load_ro_base       = Load$$DLO_REGION$$RO$$Base,
    .exec_ro_base       = Image$$DLO_REGION$$RO$$Base,
    .ro_length          = Image$$DLO_REGION$$RO$$Length,
                       
    /* RW info */                   
    .load_rw_base       = Load$$DLO_REGION$$RW$$Base,
    .exec_rw_base       = Image$$DLO_REGION$$RW$$Base,
    .rw_length          = Image$$DLO_REGION$$RW$$Length,
         
    /* ZI info */
    .exec_zi_base       = Image$$DLO_REGION$$ZI$$Base,
    .zi_length          = Image$$DLO_REGION$$ZI$$Length,

};
