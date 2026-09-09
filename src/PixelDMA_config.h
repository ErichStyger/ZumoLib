/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PIXEL_DMA_CONFIG_H_
#define PIXEL_DMA_CONFIG_H_

/* GPIO information, needed for muxing the lane pins as output pins */
#if PL_CONFIG_USE_NEO_PIXEL_FRONT
  #define PIXEL_DMA_CONFIG_NEOPIXEL_FRONT_GPIO      GPIOE /*!< GPIO peripheral for lane pin */
  #define PIXEL_DMA_CONFIG_NEOPIXEL_FRONT_PORT      PORTE /*!< PORT peripheral for lane pin */
  #define PIXEL_DMA_CONFIG_NEOPIXEL_FRONT_PIN       3u    /*!< Pin number for lane pin */
#endif
#if PL_CONFIG_USE_NEO_PIXEL_BACK
  #define PIXEL_DMA_CONFIG_NEOPIXEL_BACK_GPIO       GPIOB /*!< GPIO peripheral for lane pin */
  #define PIXEL_DMA_CONFIG_NEOPIXEL_BACK_PORT       PORTB /*!< PORT peripheral for lane pin */
  #define PIXEL_DMA_CONFIG_NEOPIXEL_BACK_PIN        16u   /*!< Pin number for lane */
#endif

#include "platform.h"
#if PL_CONFIG_USE_PIXEL_LANE_CHAINING
  #include "NeoConfig.h"

  #define PIXEL_DMA_CONFIG_NOF_BYTES_FIRST                (NEOC_NOF_LEDS_FIRST*(NEOC_NOF_COLORS*8))
  #define PIXEL_DMA_CONFIG_NOF_BYTES_SECOND               (NEOC_NOF_LEDS_SECOND*(NEOC_NOF_COLORS*8))

  #define PIXEL_DMA_CONFIG_LANE_GPIO_FIRST                (PIXEL_DMA_CONFIG_NEOPIXEL_FRONT_GPIO)
  #define PIXEL_DMA_CONFIG_LANE_GPIO_SECOND               (PIXEL_DMA_CONFIG_NEOPIXEL_BACK_GPIO)

  #define PIXEL_DMA_CONFIG_CLOCK_PORT_FIRST               (kCLOCK_PortE)
  #define PIXEL_DMA_CONFIG_CLOCK_PORT_SECOND              (kCLOCK_PortB)

  #define PIXEL_DMA_CONFIG_GPIO_PIN_BYTE_OFFSET_FIRST     (PIXEL_DMA_CONFIG_NEOPIXEL_FRONT_PIN/8) /* PTE3: (3/8) */
  #define PIXEL_DMA_CONFIG_GPIO_PIN_BYTE_OFFSET_SECOND    (PIXEL_DMA_CONFIG_NEOPIXEL_BACK_PIN/8) /* PTB16 (16/8) */
#else

  #ifndef PIXEL_DMA_CONFIG_LANE_GPIO
    #if PL_CONFIG_USE_NEO_PIXEL_FRONT
      #define PIXEL_DMA_CONFIG_LANE_GPIO     (GPIOE)
    #else
      #define PIXEL_DMA_CONFIG_LANE_GPIO     (GPIOB)
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
#endif

#ifndef PIXEL_DMA_CONFIG_INVERT_WAVEFORM
  #define PIXEL_DMA_CONFIG_INVERT_WAVEFORM   (1)  /* if the wave form needs to be inverted because of an inverting level shifter */
#endif

#endif /* PIXEL_DMA_CONFIG_H_ */
