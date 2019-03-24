
#ifndef __HARDFAULT_H_98148ede_1c6c_4639_ae4c_0e584dacf83c__
#define __HARDFAULT_H_98148ede_1c6c_4639_ae4c_0e584dacf83c__


/*_____ I N C L U D E S ____________________________________________________*/
#include "snc_types.h"


typedef enum
{
    FAULT_ADDRESS_INVALID,
    FAULT_ADDRESS_VALID
    
}fault_addr_e;


typedef enum
{
    FAULT_NONE,
    
    FAULT_MEMORY_MSTKERR,       /*!< MemManage fault on stacking for exception entry                                    */
    FAULT_MEMORY_MUNSTKERR,     /*!< MemManage fault on unstacking for a return from exception                          */
    FAULT_MEMORY_DACCVIOL,      /*!< Data access violation                                                              */
    FAULT_MEMORY_IACCVIOL,      /*!< Instruction access violation                                                       */
                                
    FAULT_BUS_STKERR,           /*!< BusFault on stacking for exception entry                                           */
    FAULT_BUS_UNSTKERR,         /*!< BusFault on unstacking for a return from exception                                 */
    FAULT_BUS_IMPRECISERR,      /*!< Imprecise data bus error                                                           */
    FAULT_BUS_PRECISERR,        /*!< Precise data bus error                                                             */
    FAULT_BUS_IBUSERR,          /*!< Instruction bus error                                                              */
                                
    FAULT_USAGE_DIVBYZERO,      /*!< Divide by zero UsageFault                                                          */
    FAULT_USAGE_UNALIGNED,      /*!< Unaligned access UsageFault                                                        */
    FAULT_USAGE_NOCP,           /*!< No coprocessor UsageFault. The processor does not support coprocessor instructions */
    FAULT_USAGE_INVPC,          /*!< Invalid PC load UsageFault, caused by an invalid PC load by EXC_RETURN             */
    FAULT_USAGE_INVSTATE,       /*!< Invalid state UsageFault                                                           */
    FAULT_USAGE_UNDEFINSTR,     /*!< Undefined instruction UsageFault                                                   */
    
}cortex_fault_e;


typedef struct
{
    fault_addr_e    addr_valid;
    uint32_t        address;
    uint32_t        CFSR_data;
    cortex_fault_e  item;
    
}cortex_fault_t;


/* Cortex-M3 document: DUI0552A */
/*
 *   Copyright © 2010 ARM. All rights reserved.     
 *   ARM DUI 0552A (ID121610) 

 * [Memory fault]
 ------------------------------------------------------------------------------------------------------
 * MSTKERR 
 *   MemManage fault on stacking for exception entry:
 *   0 = no stacking fault
 *   1 = stacking for an exception entry has caused one or more access violations.
 *   When this bit is 1, the SP is still adjusted but the values in the context area on the stack might
 *   be incorrect. The processor has not written a fault address to the MMAR.
 *   
 *   
 * MUNSTKERR 
 *   MemManage fault on unstacking for a return from exception:
 *   0 = no unstacking fault
 *   1 = unstack for an exception return has caused one or more access violations.
 *   This fault is chained to the handler. This means that when this bit is 1, the original return stack
 *   is still present. The processor has not adjusted the SP from the failing return, and has not
 *   performed a new save. The processor has not written a fault address to the MMAR.
 *   
 * DACCVIOL 
 *   Data access violation flag:
 *   0 = no data access violation fault
 *   1 = the processor attempted a load or store at a location that does not permit the operation.
 *   When this bit is 1, the PC value stacked for the exception return points to the faulting
 *   instruction. The processor has loaded the MMAR with the address of the attempted access.
 *   
 * IACCVIOL
 *   Instruction access violation flag:
 *   0 = no instruction access violation fault
 *   1 = the processor attempted an instruction fetch from a location that does not permit execution.
 *   This fault occurs on any access to an XN region, even when the MPU is disabled or not present.
 *   When this bit is 1, the PC value stacked for the exception return points to the faulting
 *   instruction. The processor has not written a fault address to the MMAR.
 *
 * [BUS fault]
 ------------------------------------------------------------------------------------------------------
 * STKERR 
 *   BusFault on stacking for exception entry:
 *   0 = no stacking fault
 *   1 = stacking for an exception entry has caused one or more BusFaults.
 *   When the processor sets this bit to 1, the SP is still adjusted but the values in the context area on the stack
 *   might be incorrect. The processor does not write a fault address to the BFAR.
 *   
 * UNSTKERR 
 *   BusFault on unstacking for a return from exception:
 *   0 = no unstacking fault
 *   1 = unstack for an exception return has caused one or more BusFaults.
 *   This fault is chained to the handler. This means that when the processor sets this bit to 1, the original return
 *   stack is still present. The processor does not adjust the SP from the failing return, does not performed a
 *   new save, and does not write a fault address to the BFAR.
 *   
 * IMPRECISERR 
 *   Imprecise data bus error:
 *   0 = no imprecise data bus error
 *   1 = a data bus error has occurred, but the return address in the stack frame is not related to the instruction
 *   that caused the error.
 *   When the processor sets this bit to 1, it does not write a fault address to the BFAR.
 *   This is an asynchronous fault. Therefore, if it is detected when the priority of the current process is higher
 *   than the BusFault priority, the BusFault becomes pending and becomes active only when the processor
 *   returns from all higher priority processes. If a precise fault occurs before the processor enters the handler
 *   for the imprecise BusFault, the handler detects both IMPRECISERR set to 1 and one of the precise fault
 *   status bits set to 1.
 *   
 * PRECISERR 
 *   Precise data bus error:
 *   0 = no precise data bus error
 *   1 = a data bus error has occurred, and the PC value stacked for the exception return points to the instruction
 *   that caused the fault.
 *   When the processor sets this bit is 1, it writes the faulting address to the BFAR.
 *   
 * IBUSERR 
 *   Instruction bus error:
 *   0 = no instruction bus error
 *   1 = instruction bus error.
 *   The processor detects the instruction bus error on prefetching an instruction, but it sets the IBUSERR flag
 *   to 1 only if it attempts to issue the faulting instruction.
 *   When the processor sets this bit is 1, it does not write a fault address to the BFAR..
 *  
 * [Usage fault]
 ------------------------------------------------------------------------------------------------------
 * DIVBYZERO 
 *   Divide by zero UsageFault:
 *   0 = no divide by zero fault, or divide by zero trapping not enabled
 *   1 = the processor has executed an SDIV or UDIV instruction with a divisor of 0.
 *   When the processor sets this bit to 1, the PC value stacked for the exception return points to the instruction
 *   that performed the divide by zero.
 *   Enable trapping of divide by zero by setting the DIV_0_TRP bit in the CCR to 1, see Configuration and
 *   Control Register on page 4-19.
 *   
 * UNALIGNED 
 *   Unaligned access UsageFault:
 *   0 = no unaligned access fault, or unaligned access trapping not enabled
 *   1 = the processor has made an unaligned memory access.
 *   Enable trapping of unaligned accesses by setting the UNALIGN_TRP bit in the CCR to 1, see
 *   Configuration and Control Register on page 4-19.
 *   Unaligned LDM, STM, LDRD, and STRD instructions always fault irrespective of the setting of UNALIGN_TRP.
 *  
 * NOCP
 *   No coprocessor UsageFault. The processor does not support coprocessor instructions:
 *   0 = no UsageFault caused by attempting to access a coprocessor
 *   1 = the processor has attempted to access a coprocessor.
 *   
 * INVPC 
 *   Invalid PC load UsageFault, caused by an invalid PC load by EXC_RETURN:
 *   0 = no invalid PC load UsageFault
 *   1 = the processor has attempted an illegal load of EXC_RETURN to the PC, as a result of an invalid
 *   context, or an invalid EXC_RETURN value.
 *   When this bit is set to 1, the PC value stacked for the exception return points to the instruction that tried
 *   to perform the illegal load of the PC.
 *   
 * INVSTATE 
 *   Invalid state UsageFault:
 *   0 = no invalid state UsageFault
 *   1 = the processor has attempted to execute an instruction that makes illegal use of the EPSR.
 *   When this bit is set to 1, the PC value stacked for the exception return points to the instruction that
 *   attempted the illegal use of the EPSR.
 *   This bit is not set to 1 if an undefined instruction uses the EPSR.
 *   
 * UNDEFINSTR 
 *   Undefined instruction UsageFault:
 *   0 = no undefined instruction UsageFault
 *   1 = the processor has attempted to execute an undefined instruction.
 *   When this bit is set to 1, the PC value stacked for the exception return points to the undefined instruction.
 *   An undefined instruction is an instruction that the processor cannot decode.
 *   
 * Note
 *   The UFSR bits are sticky. This means as one or more fault occurs, the associated bits are set to
 *   1. A bit that is set to 1 is cleared to 0 only by writing 1 to that bit, or by a reset.
 *  
 **/

#define MMFSR_ADDR          0xE000ED28
#define BFSR_ADDR           0xE000ED29
#define UFSR_ADDR           0xE000ED2A

/* MemManage Fault Status Register */
#define MEMFAULTSR_MMARVALID_Pos            7       /*!< MemManage Fault Address Register (MMFAR) valid flag                                */  
#define MEMFAULTSR_MMARVALID_Msk            (1 << (MEMFAULTSR_MMARVALID_Pos     + SCB_CFSR_MEMFAULTSR_Pos))

#define MEMFAULTSR_MSTKERR_Pos              4       /*!< MemManage fault on stacking for exception entry                                    */
#define MEMFAULTSR_MSTKERR_Msk              (1 << (MEMFAULTSR_MSTKERR_Pos       + SCB_CFSR_MEMFAULTSR_Pos))

#define MEMFAULTSR_MUNSTKERR_Pos            3       /*!< MemManage fault on unstacking for a return from exception                          */
#define MEMFAULTSR_MUNSTKERR_Msk            (1 << (MEMFAULTSR_MUNSTKERR_Pos     + SCB_CFSR_MEMFAULTSR_Pos))

#define MEMFAULTSR_DACCVIOL_Pos             1       /*!< Data access violation flag                                                         */
#define MEMFAULTSR_DACCVIOL_Msk             (1 << (MEMFAULTSR_DACCVIOL_Pos      + SCB_CFSR_MEMFAULTSR_Pos))

#define MEMFAULTSR_IACCVIOL_Pos             0       /*!< Instruction access violation flag                                                  */
#define MEMFAULTSR_IACCVIOL_Msk             (1 << (MEMFAULTSR_IACCVIOL_Pos      + SCB_CFSR_MEMFAULTSR_Pos))


/* BusFault Status Register */
#define BUSFAULTSR_BFARVALID_Pos            7       /*!< BusFault Address Register (BFAR) valid flag                                        */
#define BUSFAULTSR_BFARVALID_Msk            (1 << (BUSFAULTSR_BFARVALID_Pos     + SCB_CFSR_BUSFAULTSR_Pos))

#define BUSFAULTSR_STKERR_Pos               4       /*!< BusFault on stacking for exception entry                                           */
#define BUSFAULTSR_STKERR_Msk               (1 << (BUSFAULTSR_STKERR_Pos        + SCB_CFSR_BUSFAULTSR_Pos))

#define BUSFAULTSR_UNSTKERR_Pos             3       /*!< BusFault on unstacking for a return from exception                                 */
#define BUSFAULTSR_UNSTKERR_Msk             (1 << (BUSFAULTSR_UNSTKERR_Pos      + SCB_CFSR_BUSFAULTSR_Pos))

#define BUSFAULTSR_IMPRECISERR_Pos          2       /*!< Imprecise data bus error                                                           */
#define BUSFAULTSR_IMPRECISERR_Msk          (1 << (BUSFAULTSR_IMPRECISERR_Pos   + SCB_CFSR_BUSFAULTSR_Pos))

#define BUSFAULTSR_PRECISERR_Pos            0       /*!< Precise data bus error                                                             */
#define BUSFAULTSR_PRECISERR_Msk            (1 << (BUSFAULTSR_PRECISERR_Pos     + SCB_CFSR_BUSFAULTSR_Pos))

#define BUSFAULTSR_IBUSERR_Pos              0       /*!< Instruction bus error                                                              */
#define BUSFAULTSR_IBUSERR_Msk              (1 << (BUSFAULTSR_IBUSERR_Pos       + SCB_CFSR_BUSFAULTSR_Pos))


/* UsageFault Status Register */
#define USGFAULTSR_DIVBYZERO_Pos            9       /*!< Divide by zero UsageFault                                                          */
#define USGFAULTSR_DIVBYZERO_Msk            (1 << (USGFAULTSR_DIVBYZERO_Pos     + SCB_CFSR_USGFAULTSR_Pos))

#define USGFAULTSR_UNALIGNED_Pos            8       /*!< Unaligned access UsageFault                                                        */
#define USGFAULTSR_UNALIGNED_Msk            (1 << (USGFAULTSR_UNALIGNED_Pos     + SCB_CFSR_USGFAULTSR_Pos))

#define USGFAULTSR_NOCP_Pos                 3       /*!< No coprocessor UsageFault. The processor does not support coprocessor instructions */
#define USGFAULTSR_NOCP_Msk                 (1 << (USGFAULTSR_NOCP_Pos          + SCB_CFSR_USGFAULTSR_Pos))

#define USGFAULTSR_INVPC_Pos                2       /*!< Invalid PC load UsageFault, caused by an invalid PC load by EXC_RETURN             */
#define USGFAULTSR_INVPC_Msk                (1 << (USGFAULTSR_INVPC_Pos         + SCB_CFSR_USGFAULTSR_Pos))

#define USGFAULTSR_INVSTATE_Pos             1       /*!< Invalid state UsageFault                                                           */
#define USGFAULTSR_INVSTATE_Msk             (1 << (USGFAULTSR_INVSTATE_Pos      + SCB_CFSR_USGFAULTSR_Pos))

#define USGFAULTSR_UNDEFINSTR_Pos           0       /*!< Undefined instruction UsageFault                                                   */
#define USGFAULTSR_UNDEFINSTR_Msk           (1 << (USGFAULTSR_UNDEFINSTR_Pos    + SCB_CFSR_USGFAULTSR_Pos))


void HardFaultParser(cortex_fault_t*);

uint32_t MPU_ExceptionParser(cortex_fault_t*);

uint32_t BUS_ExceptionParser(cortex_fault_t*);

uint32_t Usage_ExceptionParser(cortex_fault_t*);

void Check_Stack_Overflow(void);

#endif
