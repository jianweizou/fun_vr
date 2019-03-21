/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file system.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/01/14
 * @license
 * @description
 */


#include "snc_types.h"
#include "snc_system.h"
#include "reg_util.h"


#include "register_7320.h"
#define SYSTEM_VERSION        0x73200000
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static struct sn_sys_ctrl_0     *g_pSys_ctrl_0 = (struct sn_sys_ctrl_0*)SN_SYS_CTRL_0_BASE;
static struct sn_sys_ctrl_1     *g_pSys_ctrl_1 = (struct sn_sys_ctrl_1*)SN_SYS_CTRL_1_BASE;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

int 
System_Clock_Enable(system_clock_t clk)
{
    switch(clk)
    {
        case SYSTEM_CLOCK_IHRC:
            g_pSys_ctrl_0->OSC_CTRL_b.IHRCEN = 1;
            break;
        
        case SYSTEM_CLOCK_ILRC:
            /* always on */
            break;
        
        case SYSTEM_CLOCK_HXTAL:
            g_pSys_ctrl_0->OSC_CTRL_b.HXTLEN = 1;
            break;
        
        case SYSTEM_CLOCK_LXTAL:
        {
            /* ISO Block */
            const uint32_t BASE_ISO = 0x45000200;
            *(uint32_t*)BASE_ISO |= (1 << 5);
            break;
        }
        case SYSTEM_CLOCK_SPLL:
             g_pSys_ctrl_0->OSC_CTRL_b.PLL_EN = 1;
            break;
    }
    return 0;
}


int 
System_Clock_Disable(system_clock_t clk)
{
    /* current clock. */
    if(g_pSys_ctrl_0->SYS_CLKCFG_b.SYSCLKSEL == clk)
        return -1;
    
    switch(clk)
    {
        case SYSTEM_CLOCK_IHRC:
            g_pSys_ctrl_0->OSC_CTRL_b.IHRCEN = 0;
            break;
        
        case SYSTEM_CLOCK_ILRC:
            /* always on */
            break;
        
        case SYSTEM_CLOCK_HXTAL:
            g_pSys_ctrl_0->OSC_CTRL_b.HXTLEN = 0;
            break;
        
        case SYSTEM_CLOCK_LXTAL:
        {
            /* ISO Block */
            const uint32_t BASE_ISO = 0x45000200;
            *(uint32_t*)BASE_ISO &= ~(1 << 5);
            break;
        }
        case SYSTEM_CLOCK_SPLL:
             g_pSys_ctrl_0->OSC_CTRL_b.PLL_EN = 0;
            break;
    }
    return 0;
}

int
System_PLL_Sel(system_clock_t clk)
{
    switch(clk)
    {
        case SYSTEM_CLOCK_IHRC:
            g_pSys_ctrl_0->OSC_CTRL_b.PLL_CLK_SEL = 0;
            break;
        
        case SYSTEM_CLOCK_HXTAL:
            g_pSys_ctrl_0->OSC_CTRL_b.PLL_CLK_SEL = 1;
            break;
        
        case SYSTEM_CLOCK_ILRC:
        case SYSTEM_CLOCK_LXTAL:
        case SYSTEM_CLOCK_SPLL:
            return -1;
    }
    return 0;
}



int
System_PLL_Ctrl(
    system_pll_ctrl_t   ctrl_type,
    uint32_t            value)
{
    int         rval = 0;
    uint32_t    data = 0;

    switch( ctrl_type )
    {
        case SYSTEM_PLL_LPF_OFF:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->OSC_CTRL, (data << 16), (0x1 << 16));
            break;
        case SYSTEM_PLL_EN:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->OSC_CTRL, (data << 15), (0x1 << 15));
            break;
        case SYSTEM_PLL_CLK_SEL:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->OSC_CTRL, (data << 14), (0x1 << 14));
            break;
        case SYSTEM_PLL_TCK_SEL:
            data = value & 0x3;
            reg_write_mask_bits(&g_pSys_ctrl_0->OSC_CTRL, (data << 5), (0x3 << 5));
            break;
        case SYSTEM_PLL_HXTALEN:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->OSC_CTRL, (data << 4), (0x1 << 4));
            break;
        case SYSTEM_PLL_ICP_SEL:
            data = value & 0x3;
            reg_write_mask_bits(&g_pSys_ctrl_0->OSC_CTRL, (data << 2), (0x3 << 2));
            break;
        case SYSTEM_PLL_DLY_SEL:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->OSC_CTRL, (data << 1), (0x1 << 1));
            break;
        case SYSTEM_PLL_IHRCEN:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->OSC_CTRL, (data << 0), (0x1 << 0));
            break;
        case SYSTEM_PLL_FRA_L:
            data = value & 0xFFFF;
            reg_write_mask_bits(&g_pSys_ctrl_0->FPLL_CTRL, (data << 16), 0xFFFF0000);
            break;
        case SYSTEM_PLL_IVCO_SEL:
            data = value & 0xF;
            reg_write_mask_bits(&g_pSys_ctrl_0->FPLL_CTRL, (data << 11), (0xF << 11));
        case SYSTEM_PLL_SDM_EN:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->FPLL_CTRL, (data << 10), (0x1 << 10));
        case SYSTEM_PLL_VLD_IN:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->FPLL_CTRL, (data << 9), (0x1 << 9));
        case SYSTEM_PLL_INT:
            data = value & 0x1F;
            reg_write_mask_bits(&g_pSys_ctrl_0->FPLL_CTRL, (data << 4), (0x1F << 4));
            break;
        case SYSTEM_PLL_FRA_H:
            data = value & 0xF;
            reg_write_mask_bits(&g_pSys_ctrl_0->FPLL_CTRL, (data << 0), 0xF);
            break;
        default:
            rval = -1;
            break;
    }

    return rval;
}

int
System_Get_Clock_Status(
    system_clock_t  clock_type)
{
    int     rval = 0;

    switch( clock_type )
    {
        case SYSTEM_CLOCK_SPLL:
            rval = reg_read_mask_bits(&g_pSys_ctrl_0->OSC_STAT, (0x1 << 6));
            break;
        case SYSTEM_CLOCK_HXTAL:
            rval = reg_read_mask_bits(&g_pSys_ctrl_0->OSC_STAT, (0x1 << 4));
            break;
        case SYSTEM_CLOCK_LXTAL:
            rval = reg_read_mask_bits(&g_pSys_ctrl_0->OSC_STAT, (0x1 << 2));
            break;
        case SYSTEM_CLOCK_IHRC:
            rval = reg_read_mask_bits(&g_pSys_ctrl_0->OSC_STAT, (0x1 << 0));
            break;

        case SYSTEM_CLOCK_ILRC:
        default:    break;
    }
    return rval;
}


/**
 *  \brief  get system reset state
 *
 *  \param [in] reset_type      enum system_reset_type
 *  \return     true: reset occurred, false: No reset occurred
 *
 *  \details
 */
int
System_Get_Reset_State(
    system_reset_type_t     reset_type)
{
    return (reg_read_mask_bits(&g_pSys_ctrl_0->OSC_STAT, (0x1 << reset_type)))
            ? 1 : 0;
}



/**
 *  \brief  Remap system start memory address
 *
 *  \param [in] remap_type      enum system_remap
 *  \return
 *
 *  \details
 */
int
System_Set_Remap(
    system_remap_t  remap_type)
{
    int     rval = 0;

    reg_write_mask_bits(&g_pSys_ctrl_0->SYS0_REMAP, remap_type, 0x1);

    return rval;
}

int
System_LVD_Ctrl(
    system_lvd_ctrl_t   ctrl_type,
    uint32_t            value)
{
    int         rval = 0;
    uint32_t    data = 0;

    switch( ctrl_type )
    {
        case SYSTEM_LVD_11EN:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->LVD_CTRL, (data << 0), (0x1 << 0));
            break;

        case SYSTEM_LVD_33EN:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->LVD_CTRL, (data << 1), (0x1 << 1));
            break;

        case SYSTEM_LVD_POW_DOWN:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->LVD_CTRL, (data << 2), (0x1 << 2));
            break;

        case SYSTEM_LVD_33_VAL:
            data = (value) & 3;
            reg_write_mask_bits(&g_pSys_ctrl_0->LVD_CTRL, (data << 3), (0x3 << 3));
            break;

        case SYSTEM_LDO_ILRC_SEL:
            data = (value) & 3;
            reg_write_mask_bits(&g_pSys_ctrl_0->LVD_CTRL, (data << 5), (0x3 << 5));
            break;

        default:
            rval = -1;
    }

    return rval;
}

int
System_Pin_Ctrl(
    system_pin_set_t    pin_type,
    uint32_t            value)
{
    int         rval = 0;
    uint32_t    data = 0;

    switch( pin_type )
    {
        case SYSTEM_PIN_SET_USB_TSQU_SEL:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 26), (0x1 << 26));
            break;
        case SYSTEM_PIN_SET_USB_DBG_MODE:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 25), (0x1 << 25));
            break;
        case SYSTEM_PIN_SET_HOST_DEV_SEL:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 24), (0x1 << 24));
            break;
        case SYSTEM_PIN_SET_AUDIO_IF2:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 23), (0x1 << 23));
            break;
        case SYSTEM_PIN_SET_I2C_SYNC_SW:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 22), (0x1 << 22));
            break;
        case SYSTEM_PIN_SET_PWMIO:
           data = value & 0xFFF;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 10), (0xFFF << 10));
            break;
        case SYSTEM_PIN_SET_OID_SENSOR:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 9), (0x1 << 9));
            break;
        case SYSTEM_PIN_SET_SPIDMA0:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 8), (0x1 << 8));
            break;
        case SYSTEM_PIN_SET_SWDV_1:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 7), (0x1 << 7));
            break;
        case SYSTEM_PIN_SET_SWDV_0:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 6), (0x1 << 6));
            break;
        case SYSTEM_PIN_SET_I2S4:
           data = value & 0x3;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 4), (0x3 << 4));
        case SYSTEM_PIN_SET_HOST_PPC:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 3), (0x1 << 3));
            break;
        case SYSTEM_PIN_SET_TFT_8080:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 1), (0x1 << 1));
            break;
        case SYSTEM_PIN_SET_SDNF0:
            data = (value) ? 0x1 : 0x0;
            reg_write_mask_bits(&g_pSys_ctrl_0->PINCTRL, (data << 0), (0x1 << 0));
            break;
        default:
            rval = -1;
            break;
    }
    return rval;
}

int
System_Set_NDT_Time(
    system_ndt_time_t   ndt_time)
{
    int     rval = 0;

    reg_write_mask_bits(&g_pSys_ctrl_0->SYS0_NDTCTRL, ndt_time, 0x3);
    return rval;
}

/**
 *  \brief      switch system clock
 *
 *  \param [in] clock_type      target clock type
 *  \param [in] clock_div       clock divider (pre-scaler)
 *  \return     0: ok, other: fail
 *
 *  \details
 */
int
System_Clock_Switch(
    system_clock_t          clock_type,
    system_clock_div_t      clock_div)
{
    uint32_t    timeout = 0;

    g_pSys_ctrl_0->SYS_CLKCFG = clock_type;
    
    while( (g_pSys_ctrl_0->SYS_CLKCFG >> 4) != clock_type )
    {
        // TODO: times need to re-calculate
        if( timeout++ > 1000 )
        {
            return -1;
        }
    }

    g_pSys_ctrl_0->SYS_CLKDIV = clock_div;

    return 0;
}


//TODO: complete when EFUSE table is comfirmed.
int
System_Get_Efuse()
{
    int     rval = 0;
    return rval;
}

int
System_Ram_Power_Down(uint32_t ram)
{
    int     rval = 0;

    reg_write_mask_bits(&g_pSys_ctrl_0->NAP, ram, 0xFFFF);

    return rval;
}

int
System_DRAM_Channel_Sel(
    system_ddr_user_t       user,
    system_ddr_channel_t    channel)
{
    int         rval = 0;
    uint32_t    shift = user << 1;

    g_pSys_ctrl_0->DMA_CH_SEL &= ~(0x3 << shift);
    g_pSys_ctrl_0->DMA_CH_SEL |= channel << shift;
    return rval;
}

int
System_Tick_Calibration(
    uint32_t    value)
{
    int     rval = 0;
    reg_write_mask_bits(&g_pSys_ctrl_0->M3_TICK, value, 0x3FFFFFF);
    return rval;
}

int
System_Clock_APB_Divide(
    system_apb_div_t    clock_div)
{
    int         rval = 0;

    reg_write_mask_bits(&g_pSys_ctrl_1->PERI_CLKCTL, clock_div, 0x7);
    return rval;
}

int
System_Clock_SD_NF_Divide(
    system_sd_nf_div_t    clock_div)
{
    int         rval = 0;

    reg_write_mask_bits(&g_pSys_ctrl_1->PERI_CLKCTL, (clock_div << 5), (0x3 << 5));
    return rval;
}

int
System_Clock_Peripheral(uint32_t setting)
{
    int     rval = 0;

    g_pSys_ctrl_1->PCLK_EN = setting;
    return rval;
}


uint32_t
System_Get_SysClk(void)
{
    uint32_t clock_hz = 0;

    switch(g_pSys_ctrl_0->SYS_CLKCFG_b.SYSCLKSEL)
    {
        case SYSTEM_CLOCK_IHRC:
        case SYSTEM_CLOCK_HXTAL:
            clock_hz = 12000000;
            break;

        case SYSTEM_CLOCK_ILRC:
        case SYSTEM_CLOCK_LXTAL:
            clock_hz = 32767;
            break;

        case SYSTEM_CLOCK_SPLL:
        {
            /* TODO: Calculate the decimal digit. */
            uint32_t fra_temp = ((  g_pSys_ctrl_0->FPLL_CTRL_b.PLL_FRA_H << 16) |
                                    g_pSys_ctrl_0->FPLL_CTRL_b.PLL_FRA_L
                                );
                        
            float fraction_part = (float)fra_temp / 0xFFFFF;
            
            clock_hz = ((float)g_pSys_ctrl_0->FPLL_CTRL_b.PLL_INT + fraction_part) * 6000000 ;
            
            break;
        }
    }
    
    clock_hz >>= g_pSys_ctrl_0->SYS_CLKDIV;
    
    return clock_hz;
}

int
System_Set_SysClk(uint32_t Clk)
{
    uint32_t timeout = 0;
    uint32_t switch_pll = 0;

    if(Clk > 162000000)   /* Maximun CLK */
        return -1;

    if(Clk < 30000000)   /* Minimun CLK */
        return -1;
    
    /* Check if PLL is used as system clock source. */
    if(g_pSys_ctrl_0->SYS_CLKCFG_b.SYSCLKST == SYSTEM_CLOCK_SPLL)
    {
        switch_pll = 1;
        
        /* Switch System CLK to other source */
        if(g_pSys_ctrl_0->OSC_CTRL_b.PLL_CLK_SEL == SYSTEM_CLOCK_IHRC)
        {
            g_pSys_ctrl_0->SYS_CLKCFG_b.SYSCLKSEL = 0;              /* Set IHRC as Sys clock */
            while(g_pSys_ctrl_0->SYS_CLKCFG != 0)                   /* Wait for Switching */
            {
                // TODO: times need to re-calculate
                if( timeout++ > 1000 )
                {
                    return -1;
                }
            }
        }
        else
        {
            g_pSys_ctrl_0->SYS_CLKCFG_b.SYSCLKSEL = SYSTEM_CLOCK_HXTAL; /* Set HXTAL as Sys clock */
            while(g_pSys_ctrl_0->SYS_CLKCFG != 0x22)                    /* Wait for Switching */
            {
                // TODO: times need to re-calculate
                if( timeout++ > 1000 )
                {
                    return -1;
                }
            }
        }
    }


    /* Find the PLL_INT and PLL_FRA number.
     * float - 23bits mantissa.
     */
    uint32_t quotient   = Clk/6000000;   /* Integer part. */
    float  temp         = (float)Clk/6000000;   /* Fraction part. */
    temp                = temp - quotient;   
    uint32_t  hex_fra   = (uint32_t)(temp * 0xFFFFF);

    g_pSys_ctrl_0->FPLL_CTRL_b.PLL_INT    = quotient;

    /* TODO: Check if there's fractional part */
    if( ((uint32_t)Clk % 6000000) )
    {
        g_pSys_ctrl_0->FPLL_CTRL_b.PLL_FRA_H  = (hex_fra>> 16) & 0xF;             /* MSB 4bit */
        g_pSys_ctrl_0->FPLL_CTRL_b.PLL_FRA_L  = hex_fra & 0xFFFF;      /* LSB 16bit */
    }
    else    /* Integer */
    {
        g_pSys_ctrl_0->FPLL_CTRL_b.PLL_FRA_H = 0;
        g_pSys_ctrl_0->FPLL_CTRL_b.PLL_FRA_L = 0;
    }

    g_pSys_ctrl_0->FPLL_CTRL_b.SDM_EN = 1;
    /* 100ns_delay Sys CLK=12Mhz, NOP = 83ns */
    asm ("nop");
    asm ("nop");

    g_pSys_ctrl_0->FPLL_CTRL_b.VLD_IN = 1;
    /* 100ns_delay Sys CLK=12Mhz, NOP = 83ns */
    asm ("nop");
    asm ("nop");


    /* 200ns_delay Sys CLK=12Mhz, NOP = 83ns */
    asm ("nop");
    asm ("nop");
    asm ("nop");


    g_pSys_ctrl_0->FPLL_CTRL_b.VLD_IN = 0;
    
    if(switch_pll)
    {
        g_pSys_ctrl_0->SYS_CLKCFG_b.SYSCLKSEL = 4;              /* Set PLL as Sys clock */
        while(g_pSys_ctrl_0->SYS_CLKCFG != 0x44)                   /* Wait for Switching */
        {
            // TODO: times need to re-calculate
            if( timeout++ > 1000 )
            {
                return -1;
            }
        }
    }
    
    return 0;
}

int
System_PLL_Set_Freq(uint32_t Clk)
{
    uint32_t timeout = 0;
    uint32_t switch_pll = 0;

    if(Clk > 324000000)   /* Maximun CLK */
        return -1;

    /* Check if PLL is used as system clock source. */
    if(g_pSys_ctrl_0->SYS_CLKCFG_b.SYSCLKST == SYSTEM_CLOCK_SPLL)
    {
        switch_pll = 1;
        
        /* Switch System CLK to other source */
        if(g_pSys_ctrl_0->OSC_CTRL_b.PLL_CLK_SEL == SYSTEM_CLOCK_IHRC)
        {
            g_pSys_ctrl_0->SYS_CLKCFG_b.SYSCLKSEL = 0;              /* Set IHRC as Sys clock */
            while(g_pSys_ctrl_0->SYS_CLKCFG != 0)                   /* Wait for Switching */
            {
                // TODO: times need to re-calculate
                if( timeout++ > 1000 )
                {
                    return -1;
                }
            }
        }
        else
        {
            g_pSys_ctrl_0->SYS_CLKCFG_b.SYSCLKSEL = SYSTEM_CLOCK_HXTAL; /* Set HXTAL as Sys clock */
            while(g_pSys_ctrl_0->SYS_CLKCFG != 0x22)                    /* Wait for Switching */
            {
                // TODO: times need to re-calculate
                if( timeout++ > 1000 )
                {
                    return -1;
                }
            }
        }
    }


    /* Find the PLL_INT and PLL_FRA number.
     * float - 23bits mantissa.
     */
    uint32_t quotient   = Clk/12000000;   /* Integer part. */
    float  temp         = (float)Clk/12000000;   /* Fraction part. */
    temp                = temp - quotient;   
    uint32_t  hex_fra   = (uint32_t)(temp * 0xFFFFF);

    g_pSys_ctrl_0->FPLL_CTRL_b.PLL_INT    = quotient;

    /* TODO: Check if there's fractional part */
    if( ((uint32_t)Clk % 12000000) )
    {
        g_pSys_ctrl_0->FPLL_CTRL_b.PLL_FRA_H  = (hex_fra>> 16) & 0xF;             /* MSB 4bit */
        g_pSys_ctrl_0->FPLL_CTRL_b.PLL_FRA_L  = hex_fra & 0xFFFF;      /* LSB 16bit */
    }

    g_pSys_ctrl_0->FPLL_CTRL_b.SDM_EN = 1;
    /* 100ns_delay Sys CLK=12Mhz, NOP = 83ns */
    asm ("nop");
    asm ("nop");

    g_pSys_ctrl_0->FPLL_CTRL_b.VLD_IN = 1;
    /* 100ns_delay Sys CLK=12Mhz, NOP = 83ns */
    asm ("nop");
    asm ("nop");


    /* 200ns_delay Sys CLK=12Mhz, NOP = 83ns */
    asm ("nop");
    asm ("nop");
    asm ("nop");


    g_pSys_ctrl_0->FPLL_CTRL_b.VLD_IN = 0;
    
    if(switch_pll)
    {
        g_pSys_ctrl_0->SYS_CLKCFG_b.SYSCLKSEL = 4;              /* Set PLL as Sys clock */
        while(g_pSys_ctrl_0->SYS_CLKCFG != 0x44)                   /* Wait for Switching */
        {
            // TODO: times need to re-calculate
            if( timeout++ > 1000 )
            {
                return -1;
            }
        }
    }
    
    return 0;
}

uint32_t
System_Get_Version(void)
{
    return SYSTEM_VERSION;
}
