


;/**************************************************************************//**
; * @file     startup_ARMCM3.s
; * @brief    CMSIS Core Device Startup File for
; *           ARMCM3 Device Series
; * @version  V1.08
; * @date     23. November 2012
; *
; * @note
; *
; ******************************************************************************/
;/* Copyright (c) 2011 - 2012 ARM LIMITED
;
;   All rights reserved.
;   Redistribution and use in source and binary forms, with or without
;   modification, are permitted provided that the following conditions are met:
;   - Redistributions of source code must retain the above copyright
;     notice, this list of conditions and the following disclaimer.
;   - Redistributions in binary form must reproduce the above copyright
;     notice, this list of conditions and the following disclaimer in the
;     documentation and/or other materials provided with the distribution.
;   - Neither the name of ARM nor the names of its contributors may be used
;     to endorse or promote products derived from this software without
;     specific prior written permission.
;   *
;   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
;   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
;   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
;   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
;   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
;   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
;   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
;   POSSIBILITY OF SUCH DAMAGE.
;   ---------------------------------------------------------------------------*/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>
Stack_Size      EQU     0x00001000
                EXPORT  Stack_Size

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>
Heap_Size       EQU     0x00009000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler/WDT0/WDT1
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
				DCD		Core0_Issue_IRQHandler		; 16+ 0: M3_0 dual core
				DCD		Core1_Issue_IRQHandler		; 16+ 1: M3_1 dual core
				DCD		RTC_IRQHandler				; 16+ 2: RTC
				DCD		WKP_IRQHandler			    ; 16+ 3: WKP
				DCD		PPU_VBLK_IRQHandler			; 16+ 4: PPU_VBLK
				DCD		PPU_HBLK_IRQHandler			; 16+ 5: PPU_HBLK
				DCD		USBDEV_IRQHandler			; 16+ 6: USBDEV
				DCD		USB_HOST0_IRQHandler		; 16+ 7: USB_HOST0
				DCD		USB_HOST1_IRQHandler		; 16+ 8: USB_HOST1
				DCD		SAR_ADC_IRQHandler			; 16+ 9: SAR_ADC
				DCD		CIS_VSYNC_IRQHandler		; 16+10: CIS_VSYNC
				DCD		CIS_DMA_IRQHandler			; 16+11: CIS_DMA
				DCD		JPEG_ENC_END_IRQHandler		; 16+12: JPEG_ENC_DMA
				DCD		JPEG_DEC_DMA_IN_IRQHandler	; 16+13: JPEG_DEC_IN_DMA
				DCD		JPEG_DEC_DMA_OUT_IRQHandler	; 16+14: JPEG_DEC_OUT_DMA
				DCD		SDIO_IRQHandler				; 16+15: SDIO
				DCD     SDIO_DMA_IRQHandler			; 16+16: SDIO_DMA
				DCD     NFECC_IRQHandler			; 16+17: NF ECC
				DCD     SDNF_DMA_IRQHandler			; 16+18: DMA SDCARD | NF
				DCD     IDMA0_IRQHandler			; 16+19: IDMA0
				DCD     IDMA1_IRQHandler			; 16+20: IDMA1
				DCD     I2S0_DMA_IRQHandler			; 16+21: I2S0_DMA
				DCD		I2S1_DMA_IRQHandler			; 16+22: I2S1_DMA
				DCD		I2S2_DMA_IRQHandler			; 16+23: I2S2_DMA
				DCD		I2S3_DMA_IRQHandler			; 16+24: I2S3_DMA
				DCD		I2S4_IRQHandler				; 16+25: I2S4
				DCD		GPIO0_IRQHandler			; 16+26: GPIO0
				DCD		GPIO1_IRQHandler			; 16+27: GPIO1
				DCD		GPIO2_IRQHandler			; 16+28: GPIO2
				DCD		GPIO3_IRQHandler			; 16+29: GPIO3
				DCD		GPIO4_IRQHandler			; 16+30: GPIO4
				DCD		I2C0_IRQHandler				; 16+31: I2C0
				DCD		I2C1_IRQHandler				; 16+32: I2C1
				DCD		I2C2_IRQHandler				; 16+33: I2C2
				DCD		SPI0_IRQHandler				; 16+34: SPI0
				DCD		SPI1_IRQHandler				; 16+35: SPI1
				DCD		UART0_IRQHandler			; 16+36: UART0
				DCD		UART1_IRQHandler			; 16+37: UART1
				DCD		CT32B0_IRQHandler			; 16+38: CT32B0
				DCD		CT32B1_IRQHandler			; 16+39: CT32B1
				DCD		CT32B2_IRQHandler			; 16+40: CT32B2
				DCD		CT32B3_IRQHandler			; 16+41: CT32B3
				DCD		CT32B4_IRQHandler			; 16+42: CT32B4
				DCD		CT32B5_IRQHandler			; 16+43: CT32B5
				DCD		CT32B6_IRQHandler			; 16+44: CT32B6
				DCD		CT32B7_IRQHandler			; 16+45: CT32B7
				DCD		SPI0_DMA_IRQHandler			; 16+46: SPI0_DMA
				DCD		SPI0_ECC_IRQHandler			; 16+47: SPI0_ECC
				DCD		SPI1_DMA_IRQHandler			; 16+48: SPI1_DMA
				DCD		SPI1_ECC_IRQHandler			; 16+49: SPI1_ECC
				DCD		CSC_DMA_IN_IRQHandler		; 16+50: CSC_DMA_IN
				DCD		CSC_DMA_OUT_IRQHandler		; 16+51: CSC_DMA_OUT
				DCD		SCRIPT_IRQHandler			; 16+52: SCRIPT
				DCD		CRC16_IRQHandler			; 16+53: CRC16
				DCD		USB_suspend_IRQHandler			; 16+54: USB suspend
				DCD		JPEG_ENC_DMA_IN_IRQHandler			; 16+55: JPEG_ENC_IN_DMA
				DCD		JPEG_ENC_DMA_OUT_IRQHandler			; 16+56: JPEG_ENC_OUT_DMA
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  System_Init
                IMPORT  __main

                ;Reload SP(R13) to correct jump info
                MOV32   R0,#0xE000ED08
                LDR     R0,[R0]
                LDR     R13,[R0]

                LDR     R0, =System_Init
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

                ;Prevent from not implement
WKP_IRQHandler  PROC
                EXPORT  WKP_IRQHandler            [WEAK]
                BX      LR
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC

				EXPORT	Core0_Issue_IRQHandler		 [WEAK]
				EXPORT	Core1_Issue_IRQHandler		 [WEAK]
				EXPORT	RTC_IRQHandler				 [WEAK]
;				EXPORT	WKP_IRQHandler			     [WEAK]
				EXPORT	PPU_VBLK_IRQHandler			 [WEAK]
				EXPORT	PPU_HBLK_IRQHandler			 [WEAK]
				EXPORT	USBDEV_IRQHandler			 [WEAK]
				EXPORT	USB_HOST0_IRQHandler		 [WEAK]
				EXPORT	USB_HOST1_IRQHandler		 [WEAK]
				EXPORT	SAR_ADC_IRQHandler			 [WEAK]
				EXPORT	CIS_VSYNC_IRQHandler		 [WEAK]
				EXPORT	CIS_DMA_IRQHandler			 [WEAK]
				EXPORT	JPEG_ENC_END_IRQHandler		 [WEAK]
				EXPORT	JPEG_DEC_DMA_IN_IRQHandler	 [WEAK]
				EXPORT	JPEG_DEC_DMA_OUT_IRQHandler	 [WEAK]
				EXPORT	SDIO_IRQHandler				 [WEAK]
				EXPORT	SDIO_DMA_IRQHandler			 [WEAK]
				EXPORT	NFECC_IRQHandler			 [WEAK]
				EXPORT	SDNF_DMA_IRQHandler			 [WEAK]
				EXPORT	IDMA0_IRQHandler			 [WEAK]
				EXPORT	IDMA1_IRQHandler			 [WEAK]
				EXPORT	I2S0_DMA_IRQHandler			 [WEAK]
				EXPORT	I2S1_DMA_IRQHandler			 [WEAK]
				EXPORT	I2S2_DMA_IRQHandler			 [WEAK]
				EXPORT	I2S3_DMA_IRQHandler			 [WEAK]
				EXPORT	I2S4_IRQHandler				 [WEAK]
				EXPORT	GPIO0_IRQHandler			 [WEAK]
				EXPORT	GPIO1_IRQHandler			 [WEAK]
				EXPORT	GPIO2_IRQHandler			 [WEAK]
				EXPORT	GPIO3_IRQHandler			 [WEAK]
				EXPORT	GPIO4_IRQHandler			 [WEAK]
				EXPORT	I2C0_IRQHandler				 [WEAK]
				EXPORT	I2C1_IRQHandler				 [WEAK]
				EXPORT	I2C2_IRQHandler				 [WEAK]
				EXPORT	SPI0_IRQHandler				 [WEAK]
				EXPORT	SPI1_IRQHandler				 [WEAK]
				EXPORT	UART0_IRQHandler			 [WEAK]
				EXPORT	UART1_IRQHandler			 [WEAK]
				EXPORT	CT32B0_IRQHandler			 [WEAK]
				EXPORT	CT32B1_IRQHandler			 [WEAK]
				EXPORT	CT32B2_IRQHandler			 [WEAK]
				EXPORT	CT32B3_IRQHandler			 [WEAK]
				EXPORT	CT32B4_IRQHandler			 [WEAK]
				EXPORT	CT32B5_IRQHandler			 [WEAK]
				EXPORT	CT32B6_IRQHandler			 [WEAK]
				EXPORT	CT32B7_IRQHandler			 [WEAK]
				EXPORT	SPI0_DMA_IRQHandler			 [WEAK]
				EXPORT	SPI0_ECC_IRQHandler			 [WEAK]
				EXPORT	SPI1_DMA_IRQHandler			 [WEAK]
				EXPORT	SPI1_ECC_IRQHandler			 [WEAK]
				EXPORT	CSC_DMA_IN_IRQHandler		 [WEAK]
				EXPORT	CSC_DMA_OUT_IRQHandler		 [WEAK]
				EXPORT	SCRIPT_IRQHandler			 [WEAK]
				EXPORT	CRC16_IRQHandler			 [WEAK]
				EXPORT	USB_suspend_IRQHandler		 [WEAK]
				EXPORT	JPEG_ENC_DMA_IN_IRQHandler	 [WEAK]
				EXPORT	JPEG_ENC_DMA_OUT_IRQHandler	 [WEAK]

Core0_Issue_IRQHandler
Core1_Issue_IRQHandler
RTC_IRQHandler
;WKP_IRQHandler
PPU_VBLK_IRQHandler
PPU_HBLK_IRQHandler
USBDEV_IRQHandler
USB_HOST0_IRQHandler
USB_HOST1_IRQHandler
SAR_ADC_IRQHandler
CIS_VSYNC_IRQHandler
CIS_DMA_IRQHandler
JPEG_ENC_END_IRQHandler
JPEG_DEC_DMA_IN_IRQHandler
JPEG_DEC_DMA_OUT_IRQHandler
SDIO_IRQHandler
SDIO_DMA_IRQHandler
NFECC_IRQHandler
SDNF_DMA_IRQHandler
IDMA0_IRQHandler
IDMA1_IRQHandler
I2S0_DMA_IRQHandler
I2S1_DMA_IRQHandler
I2S2_DMA_IRQHandler
I2S3_DMA_IRQHandler
I2S4_IRQHandler
GPIO0_IRQHandler
GPIO1_IRQHandler
GPIO2_IRQHandler
GPIO3_IRQHandler
GPIO4_IRQHandler
I2C0_IRQHandler
I2C1_IRQHandler
I2C2_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
CT32B0_IRQHandler
CT32B1_IRQHandler
CT32B2_IRQHandler
CT32B3_IRQHandler
CT32B4_IRQHandler
CT32B5_IRQHandler
CT32B6_IRQHandler
CT32B7_IRQHandler
SPI0_DMA_IRQHandler
SPI0_ECC_IRQHandler
SPI1_DMA_IRQHandler
SPI1_ECC_IRQHandler
CSC_DMA_IN_IRQHandler
CSC_DMA_OUT_IRQHandler
SCRIPT_IRQHandler
CRC16_IRQHandler
USB_suspend_IRQHandler
JPEG_ENC_DMA_IN_IRQHandler
JPEG_ENC_DMA_OUT_IRQHandler

                B       .

                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                END
