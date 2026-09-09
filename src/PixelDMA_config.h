/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PIXEL_DMA_CONFIG_H_
#define PIXEL_DMA_CONFIG_H_

#ifndef PIXEL_DMA_CONFIG_GPIO_PORT
  #define PIXEL_DMA_CONFIG_GPIO_PORT     (GPIOB)
  //#define PIXEL_DMA_CONFIG_GPIO_PORT     (GPIOD)
#endif

#ifndef PIXEL_DMA_CONFIG_CLOCK_PORT
  #define PIXEL_DMA_CONFIG_CLOCK_PORT     (kCLOCK_PortB)
  //#define PIXEL_DMA_CONFIG_CLOCK_PORT     (kCLOCK_PortD)
#endif

/* Byte offset of the pin inside the 32bit PSOR/PDOR/PCOR registers: the DMA only writes a single
 * byte (8bit), so the destination address has to point to the byte lane that contains the used pin
 * (pin/8), e.g. offset 2 for PTB16 (bits 16..23) or offset 0 for PTD2 (bits 0..7). */
#ifndef PIXEL_DMA_CONFIG_GPIO_PIN_BYTE_OFFSET
  #define PIXEL_DMA_CONFIG_GPIO_PIN_BYTE_OFFSET     (2) /* PTB16 */
  //#define PIXEL_DMA_CONFIG_GPIO_PIN_BYTE_OFFSET     (0) /* PTD2 */
#endif

#endif /* PIXEL_DMA_CONFIG_H_ */
