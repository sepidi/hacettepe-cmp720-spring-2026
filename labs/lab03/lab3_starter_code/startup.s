/* ============================================================
 * startup.s - ARM Cortex-M4 Startup Code for STM32F4
 * SafePulse Lab Series
 * 
 * DO NOT MODIFY - This file is provided for all labs.
 * ============================================================ */

    .syntax unified
    .cpu cortex-m4
    .thumb

    .global _start
    .global Reset_Handler
    .global Default_Handler

/* ============================================================
 * Vector Table
 * ============================================================ */
    .section .isr_vector, "a", %progbits
    .type _start, %object

_start:
    .word _estack           /* Initial stack pointer */
    .word Reset_Handler     /* Reset handler */
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0                 /* Reserved */
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0                 /* Reserved */
    .word PendSV_Handler
    .word SysTick_Handler

    /* External interrupts (IRQ 0-81) */
    .word Default_Handler   /* IRQ 0:  WWDG */
    .word Default_Handler   /* IRQ 1:  PVD */
    .word Default_Handler   /* IRQ 2:  TAMP_STAMP */
    .word Default_Handler   /* IRQ 3:  RTC_WKUP */
    .word Default_Handler   /* IRQ 4:  FLASH */
    .word Default_Handler   /* IRQ 5:  RCC */
    .word Default_Handler   /* IRQ 6:  EXTI0 */
    .word Default_Handler   /* IRQ 7:  EXTI1 */
    .word Default_Handler   /* IRQ 8:  EXTI2 */
    .word Default_Handler   /* IRQ 9:  EXTI3 */
    .word Default_Handler   /* IRQ 10: EXTI4 */
    .word Default_Handler   /* IRQ 11: DMA1_Stream0 */
    .word Default_Handler   /* IRQ 12: DMA1_Stream1 */
    .word Default_Handler   /* IRQ 13: DMA1_Stream2 */
    .word Default_Handler   /* IRQ 14: DMA1_Stream3 */
    .word Default_Handler   /* IRQ 15: DMA1_Stream4 */
    .word Default_Handler   /* IRQ 16: DMA1_Stream5 */
    .word Default_Handler   /* IRQ 17: DMA1_Stream6 */
    .word Default_Handler   /* IRQ 18: ADC */
    .word Default_Handler   /* IRQ 19: CAN1_TX */
    .word Default_Handler   /* IRQ 20: CAN1_RX0 */
    .word Default_Handler   /* IRQ 21: CAN1_RX1 */
    .word Default_Handler   /* IRQ 22: CAN1_SCE */
    .word Default_Handler   /* IRQ 23: EXTI9_5 */
    .word Default_Handler   /* IRQ 24: TIM1_BRK_TIM9 */
    .word Default_Handler   /* IRQ 25: TIM1_UP_TIM10 */
    .word Default_Handler   /* IRQ 26: TIM1_TRG_COM_TIM11 */
    .word Default_Handler   /* IRQ 27: TIM1_CC */
    .word TIM2_IRQHandler   /* IRQ 28: TIM2 */
    .word Default_Handler   /* IRQ 29: TIM3 */
    .word Default_Handler   /* IRQ 30: TIM4 */
    .word Default_Handler   /* IRQ 31: I2C1_EV */
    .word Default_Handler   /* IRQ 32: I2C1_ER */
    .word Default_Handler   /* IRQ 33: I2C2_EV */
    .word Default_Handler   /* IRQ 34: I2C2_ER */
    .word Default_Handler   /* IRQ 35: SPI1 */
    .word Default_Handler   /* IRQ 36: SPI2 */
    .word Default_Handler   /* IRQ 37: USART1 */
    .word Default_Handler   /* IRQ 38: USART2 */

    .size _start, .-_start

/* ============================================================
 * Reset Handler
 * ============================================================ */
    .section .text.Reset_Handler
    .type Reset_Handler, %function
    .weak Reset_Handler

Reset_Handler:
    /* Set stack pointer */
    ldr r0, =_estack
    mov sp, r0

    /* Copy .data section from flash to SRAM */
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata
    movs r3, #0
    b .Ldata_loop_check

.Ldata_copy:
    ldr r4, [r2, r3]
    str r4, [r0, r3]
    adds r3, r3, #4

.Ldata_loop_check:
    adds r4, r0, r3
    cmp r4, r1
    bcc .Ldata_copy

    /* Zero .bss section */
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0
    b .Lbss_loop_check

.Lbss_zero:
    str r2, [r0]
    adds r0, r0, #4

.Lbss_loop_check:
    cmp r0, r1
    bcc .Lbss_zero

    /* Call main */
    bl main

    /* If main returns, loop forever */
.Lhalt:
    b .Lhalt

    .size Reset_Handler, .-Reset_Handler

/* ============================================================
 * Default Handler (infinite loop)
 * ============================================================ */
    .section .text.Default_Handler
    .type Default_Handler, %function
    .weak Default_Handler

Default_Handler:
    b Default_Handler

    .size Default_Handler, .-Default_Handler

/* ============================================================
 * Weak aliases for interrupt handlers
 * Override these in C by defining functions with these names.
 * ============================================================ */
    .weak NMI_Handler
    .thumb_set NMI_Handler, Default_Handler

    .weak HardFault_Handler
    .thumb_set HardFault_Handler, Default_Handler

    .weak MemManage_Handler
    .thumb_set MemManage_Handler, Default_Handler

    .weak BusFault_Handler
    .thumb_set BusFault_Handler, Default_Handler

    .weak UsageFault_Handler
    .thumb_set UsageFault_Handler, Default_Handler

    .weak SVC_Handler
    .thumb_set SVC_Handler, Default_Handler

    .weak DebugMon_Handler
    .thumb_set DebugMon_Handler, Default_Handler

    .weak PendSV_Handler
    .thumb_set PendSV_Handler, Default_Handler

    .weak SysTick_Handler
    .thumb_set SysTick_Handler, Default_Handler

    .weak TIM2_IRQHandler
    .thumb_set TIM2_IRQHandler, Default_Handler
