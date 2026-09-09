/*
 * Copyright (c) 2001-2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOCONFIG_H_
#define NEOCONFIG_H_

#include "platform.h"

#if PL_CONFIG_USE_NEO_PIXEL_HW

#ifndef NEOC_LANE_START
  #if PL_CONFIG_USE_NEO_PIXEL_FRONT
    #define NEOC_LANE_START        (3)  /* PTE3 (modulo 8!, because we use 8bit in the lane), this is the first GPIO pin used */
  #else
    #define NEOC_LANE_START        (0)  /* PTB16 (modulo 8!, because we use 8bit in the lane), this is the first GPIO pin used */
  #endif
#endif
#ifndef NEOC_LANE_END
  #if PL_CONFIG_USE_NEO_PIXEL_FRONT
    #define NEOC_LANE_END          (3)  /* PTE3 (modulo 8!, because we use 8bit in the lane), this is the last GPIO pin used */
  #else
    #define NEOC_LANE_END          (0)  /* PTB16 (modulo 8!, because we use 8bit in the lane), this is the last GPIO pin used */
  #endif
#endif
#ifndef NEOC_NOF_LEDS_IN_LANE
  #if PL_CONFIG_USE_NEO_PIXEL_FRONT
    #define NEOC_NOF_LEDS_IN_LANE  (8) /* number of LEDs in a lane */
  #else
    #define NEOC_NOF_LEDS_IN_LANE  (2) /* number of LEDs in a lane */
  #endif
#endif

#ifndef NEOC_NOF_COLORS
  #define NEOC_NOF_COLORS (3)  /* 3 for RGB, 4 for RGBW */
#endif

#if (NEOC_NOF_LANES>8)
  #error "can only handle up to 8 lanes (8bit port)"
#endif

#define NEOC_NOF_PIXEL   ((NEOC_LANE_END+1-NEOC_LANE_START)*(NEOC_NOF_LEDS_IN_LANE)) /* number of pixels */

#define NEOC_USE_DMA     (1)

#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

#endif /* NEOCONFIG_H_ */
