#ifndef     __INIT_CLK_TABLE__
#define     __INIT_CLK_TABLE__

static init_sequence_t     init_clk_table[] =
{
    {INI_SEQ_CMD_START, 0, 0},
    {0x0C, 0x22, 0xFFFFFFFF},               /* Set HXTL as system clock */
    {INI_SEQ_CMD_WAIT_SET, 0x0C, 0x22},     /* Wait for switching */
    {INI_SEQ_CMD_FINISH, 0, 0},
};

#endif
