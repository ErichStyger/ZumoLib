/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PIXEL_DMA_CONFIG_H_
#define PIXEL_DMA_CONFIG_H_

#ifndef PIXEL_DMA_CONFIG_GPIO_PORT
  #if PL_CONFIG_USE_NEO_PIXEL_FRONT
    #define PIXEL_DMA_CONFIG_GPIO_PORT     (GPIOE)
  #else
    #define PIXEL_DMA_CONFIG_GPIO_PORT     (GPIOB)
  #endif
#endif

#ifndef PIXEL_DMA_CONFIG_CLOCK_PORT
  #if PL_CONFIG_USE_NEO_PIXEL_FRONT
    #define PIXEL_DMA_CONFIG_CLOCK_PORT     (kCLOCK_PortE)
  #else
    #define PIXEL_DMA_CONFIG_CLOCK_PORT     (kCLOCK_PortB)
  #endif
#endif

/* Byte offset of the pin inside the 32bit PSOR/PDOR/PCOR registers: the DMA only writes a single
 * byte (8bit), so the destination address has to point to the byte lane that contains the used pin
 * (pin/8), e.g. offset 2 for PTB16 (bits 16..23) or offset 0 for PTD2 (bits 0..7). */
#ifndef PIXEL_DMA_CONFIG_GPIO_PIN_BYTE_OFFSET
  #if PL_CONFIG_USE_NEO_PIXEL_FRONT
    #define PIXEL_DMA_CONFIG_GPIO_PIN_BYTE_OFFSET     (0) /* PTE3 */
  #else  
    #define PIXEL_DMA_CONFIG_GPIO_PIN_BYTE_OFFSET     (2) /* PTB16 */
  #endif
#endif

#if PL_CONFIG_USE_NEO_PIXEL_FRONT
  #define PIXEL_DMA_CONFIG_NEOPIXEL_GPIO      GPIOE /*!< GPIO peripheral for lane pin */
  #define PIXEL_DMA_CONFIG_NEOPIXEL_PORT      PORTE /*!< PORT peripheral for lane pin */
  #define PIXEL_DMA_CONFIG_NEOPIXEL_PIN       3u    /*!< Pin number for lane pin */
#else
  #define PIXEL_DMA_CONFIG_NEOPIXEL_GPIO      GPIOB /*!< GPIO peripheral for lane pin */
  #define PIXEL_DMA_CONFIG_NEOPIXEL_PORT      PORTB /*!< PORT peripheral for lane pin */
  #define PIXEL_DMA_CONFIG_NEOPIXEL_PIN       16u   /*!< Pin number for lane */
#endif

#ifndef PIXEL_DMA_CONFIG_INVERT_WAVEFORM
  #define PIXEL_DMA_CONFIG_INVERT_WAVEFORM   (1)  /* if the wave form needs to be inverted because of an inverting level shifter */
#endif

#endif /* PIXEL_DMA_CONFIG_H_ */
