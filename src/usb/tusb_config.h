/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2022 Stefan Odermatt
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/**
 * \file
 * \brief TinyUSB stack configuration for this project.
 * \author Ha Thach, Stefan Odermatt
 * \note Licensed under MIT License, see copyright block above.
 */

/*---------------------------------------------------------------------------/
/  Configurations of TinyUSB Module
/---------------------------------------------------------------------------*/

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------
// Common Configuration
//--------------------------------------------------------------------
#define CFG_TUD_ENABLED 		(1)

#ifndef CFG_TUSB_DEBUG
  #define CFG_TUSB_DEBUG        	0  /* 0: no logging (default), 1: TU_LOG1() errors, 2: TU_LOG1()/TU_LOG2() warnings, 3: TU_LOG1()/TU_LOG2()/TU_LOG3() verbose */
#endif

/*!
 * \brief RTT-backed printf used by TinyUSB debug logging.
 * \param format Printf format string.
 * \return Number of characters written.
 */
extern int tinyusb_rtt_printf(const char *format, ...);
#define CFG_TUSB_DEBUG_PRINTF tinyusb_rtt_printf

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED     	OPT_MODE_DEFAULT_SPEED

#define CFG_TUH_MAX_SPEED 		OPT_MODE_DEFAULT_SPEED

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
    #define CFG_TUSB_MEM_SECTION     /*__attribute__ (( section("m_usb_bdt") ))*/
#endif

#ifndef CFG_TUSB_MEM_ALIGN
    #define CFG_TUSB_MEM_ALIGN    __attribute__ ((aligned(4)))
#endif

 //--------------------------------------------------------------------
 // Device Specific Configuration
 //--------------------------------------------------------------------

#define CFG_TUSB_MCU 		OPT_MCU_MK22FXX
#define CFG_TUSB_OS			OPT_OS_FREERTOS
#define TUP_DCD_ENDPOINT_MAX 16  /* Kinetis USB hardware (16 endpoint numbers: EP0...EP15) */

 // Define destination of FreeRTOS source code, used in tinyusb osal to find freeRTOS headers
 #define CFG_TUSB_OS_INC_PATH ../McuLib/FreeRTOS/Source/include/
 // Define Static allocation which is not enabled by default on RTOS
 #ifndef configSUPPORT_STATIC_ALLOCATION
    #define configSUPPORT_STATIC_ALLOCATION (1)
 #endif

#define CFG_TUD_CDC  		(1)
#define CFG_TUD_MSC  		(0)

#if CFG_TUD_CDC
  // CDC FIFO size of TX and RX
  #define CFG_TUD_CDC_RX_BUFSIZE   (1*1024)
  #define CFG_TUD_CDC_TX_BUFSIZE   (1*1024)

  // CDC Endpoint transfer buffer size, more is faster
  #define CFG_TUD_CDC_EP_BUFSIZE   (1*1024)

  #define CFG_TUD_TASK_QUEUE_SZ    (1024) /* default 16 */
#endif

#if CFG_TUD_MSC
  // MSC Buffer size of Device Mass storage
  #define CFG_TUD_MSC_EP_BUFSIZE   4096
#endif

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */
