#ifndef     __INIT_CLK_TABLE__
#define     __INIT_CLK_TABLE__



static const init_sequence_t     init_clk_table[] =
{
    {INI_SEQ_CMD_START, 0, 0},
    {0x00000000, ( 1 << 15), ( 1 << 15)},       /* Enable PLL */
    {INI_SEQ_CMD_WAIT_SET, 0x08, ( 1 << 6)},    /* Wait for system clock stable */
    {0x0000000C, 4, 0xFFFFFFFF},                /* Switch System CLK to PLL */
    {INI_SEQ_CMD_WAIT_SET, 0x0C, 0x44},         /* Wait for system clock stable */
    {INI_SEQ_CMD_FINISH, 0, 0},
};

#if 0
/* Original Function */
static void Init_Clock(void)
{
//    SN_SYS0->OSC_CTRL_b.IHRCEN = 1;                 /* Enable IHRC */
//
//    while(SN_SYS0->SYS0_CSST_b.IHRC_RDY == 0);      /* Wait for IHRC stable. */

    SN_SYS0->OSC_CTRL_b.PLL_CLK_SEL = 0;            /* Select PLL From IHRC */

    /* #.Enable PLL ---------------------------------------------------------- */
    SN_SYS0->OSC_CTRL_b.PLL_EN = 1;                 /* Enable PLL */

    Delay_Tick_us(12, 150);     /* Need to delay 150us. */

    while(SN_SYS0->SYS0_CSST_b.SPLL_RDY == 0);      /* Wait PLL Stable */

    SN_SYS0->SYS_CLKCFG_b.SYSCLKSEL = 4;            /* Set PLL as Sys clock */

    while(SN_SYS0->SYS_CLKCFG != 0x44);             /* Wait for Switching */


    /**
     *  enable all peripheral
     */
    *(volatile uint32_t*)REG_SYS1_PERI_CLK_EN |= 0xFFFF;
    *(volatile uint32_t*)REG_SYS1_CLK_RST     |= 0xFFFF;
}
#endif


#endif
