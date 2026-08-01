/*
 * Copyright (c) 2021-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Quadrature encoder counter implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "quadCounter.h"
#include "McuUtility.h"
#include "McuShell.h"
#include "McuGPIO.h"
#include "fsl_pit.h"

#define QuadCounter_GET_C1_PIN_LEFT()      (McuGPIO_GetValue(C1_Left))
#define QuadCounter_GET_C1_PIN_RIGHT()     (McuGPIO_GetValue(C1_Right))
#define QuadCounter_GET_C2_PIN_LEFT()      (McuGPIO_GetValue(C2_Left))
#define QuadCounter_GET_C2_PIN_RIGHT()     (McuGPIO_GetValue(C2_Right))

#define QuadCounter_GET_C1_PIN_LEFT()      (McuGPIO_GetValue(C1_Left))
#define QuadCounter_GET_C1_PIN_RIGHT()     (McuGPIO_GetValue(C1_Right))
#define QuadCounter_GET_C2_PIN_LEFT()      (McuGPIO_GetValue(C2_Left))
#define QuadCounter_GET_C2_PIN_RIGHT()     (McuGPIO_GetValue(C2_Right))

#if QuadCounter_SWAP_PINS
  #define QuadCounter_GET_C1_C2_PINS_LEFT()               ((QuadCounter_GET_C2_PIN_LEFT()?2:0)|(QuadCounter_GET_C1_PIN_LEFT()?1:0))
  #define QuadCounter_GET_C1_C2_PINS_RIGHT()              ((QuadCounter_GET_C2_PIN_RIGHT()?2:0)|(QuadCounter_GET_C1_PIN_RIGHT()?1:0))
  #define QuadCounter_GET_C1_C2_PINS_SWAPPED_LEFT()       ((QuadCounter_GET_C1_PIN_LEFT()?2:0)|(QuadCounter_GET_C2_PIN_LEFT()?1:0))
  #define QuadCounter_GET_C1_C2_PINS_SWAPPED_RIGHT()      ((QuadCounter_GET_C1_PIN_RIGHT()?2:0)|(QuadCounter_GET_C2_PIN_RIGHT()?1:0))
#else
  #define QuadCounter_GET_C1_C2_PINS_LEFT()               ((QuadCounter_GET_C1_PIN_LEFT()?2:0)|(QuadCounter_GET_C2_PIN_LEFT()?1:0))
  #define QuadCounter_GET_C1_C2_PINS_RIGHT()              ((QuadCounter_GET_C1_PIN_RIGHT()?2:0)|(QuadCounter_GET_C2_PIN_RIGHT()?1:0))
  #define QuadCounter_GET_C1_C2_PINS_SWAPPED_LEFT()       ((QuadCounter_GET_C2_PIN_LEFT()?2:0)|(QuadCounter_GET_C1_PIN_LEFT()?1:0))
  #define QuadCounter_GET_C1_C2_PINS_SWAPPED_RIGHT()      ((QuadCounter_GET_C2_PIN_RIGHT()?2:0)|(QuadCounter_GET_C1_PIN_RIGHT()?1:0))
#endif

static McuGPIO_Handle_t C1_Left, C2_Left, C1_Right, C2_Right;
#if QuadCounter_SWAP_PINS_AT_RUNTIME
  static bool QuadCounter_swappedPins_Left = FALSE;
  static bool QuadCounter_swappedPins_Right = FALSE;
#endif

/* The decoder has 4 different states, together with the previous state the table has 16 entries.
   The value in the table (0,1,-1) indicates the steps taken since previous sample. */
#define QUAD_ERROR  3 /*!< Value to indicate an error in impulse detection. Has to be different from 0,1,-1 */

static const signed char QuadCounter_Quad_Table[4][4] =
  {               /* prev   new    */
    {             /* c1 c2  c1 c2  */
     0,           /* 0  0   0  0  no change or missed a step? */
     1,           /* 0  0   0  1   */
     -1,          /* 0  0   1  0   */
     QUAD_ERROR   /* 0  0   1  1  error, lost impulse */
     },
    {             /* c1 c2  c1 c2  */
     -1,          /* 0  1   0  0   */
     0,           /* 0  1   0  1   no change or missed a step? */
     QUAD_ERROR,  /* 0  1   1  0   error, lost impulse */
     1            /* 0  1   1  1   */
     },
    {             /* c1 c2  c1 c2  */
     1,           /* 1  0   0  0   */
     QUAD_ERROR,  /* 1  0   0  1   error, lost impulse */
     0,           /* 1  0   1  0   no change or missed a step? */
     -1           /* 1  0   1  1   */
     },
    {             /* c1 c2  c1 c2  */
     QUAD_ERROR,  /* 1  1   0  0   error, lost impulse */
     -1,          /* 1  1   0  1   */
     1,           /* 1  1   1  0   */
     0            /* 1  1   1  1   no change or missed a step? */
     }
  };

static uint8_t QuadCounter_last_quadrature_value_left; /*! Value of left C1&C2 during last round. */
static uint8_t QuadCounter_last_quadrature_value_right; /*! Value of right C1&C2 during last round. */

static QuadCounter_QuadCntrType QuadCounter_currPos_Left = 0; /*!< Current left position */
static QuadCounter_QuadCntrType QuadCounter_currPos_Right = 0; /*!< Current right position */

static uint32_t QuadCounter_errorsLeft, QuadCounter_errorsRight;

uint32_t QuadCounter_GetErrorsLeft(void) {
  return QuadCounter_errorsLeft;
}

uint32_t QuadCounter_GetErrorsRight(void) {
  return QuadCounter_errorsRight;
}

void QuadCounter_SetPosLeft(QuadCounter_QuadCntrType pos) {
  QuadCounter_currPos_Left = pos;
}

void QuadCounter_SetPosRight(QuadCounter_QuadCntrType pos) {
  QuadCounter_currPos_Right = pos;
}

QuadCounter_QuadCntrType QuadCounter_GetPosLeft(void) {
  return QuadCounter_currPos_Left;
}

QuadCounter_QuadCntrType QuadCounter_GetPosRight(void) {
  return QuadCounter_currPos_Right;
}

#if QuadCounter_SWAP_PINS_AT_RUNTIME
void QuadCounter_SwapPinsLeft(bool swap) {
  QuadCounter_swappedPins_Left = swap;
}

void QuadCounter_SwapPinsRight(bool swap) {
  QuadCounter_swappedPins_Right = swap;
}
#endif

uint8_t QuadCounter_GetValLeft(void) {
#if QuadCounter_SWAP_PINS_AT_RUNTIME
  if (QuadCounter_swappedPins_Left) {
    return QuadCounter_GET_C1_C2_PINS_SWAPPED_LEFT();
  } else {
    return QuadCounter_GET_C1_C2_PINS_LEFT();
  }
#else
  return QuadCounter_GET_C1_C2_PINS_LEFT();
#endif
}

uint8_t QuadCounter_GetValRight(void) {
#if QuadCounter_SWAP_PINS_AT_RUNTIME
  if (QuadCounter_swappedPins_Right) {
    return QuadCounter_GET_C1_C2_PINS_SWAPPED_RIGHT();
  } else {
    return QuadCounter_GET_C1_C2_PINS_RIGHT();
  }
#else
  return QuadCounter_GET_C1_C2_PINS_RIGHT();
#endif
}

void QuadCounter_Sample(void) {
  signed char new_step;
  uint8_t c12; /* value of the two sensor input */

  /* left */
  c12 = QuadCounter_GetValLeft();
  new_step = QuadCounter_Quad_Table[QuadCounter_last_quadrature_value_left][c12];
  QuadCounter_last_quadrature_value_left = c12;
  if (new_step == QUAD_ERROR) {
    QuadCounter_errorsLeft++;
  } else if (new_step != 0) {
    QuadCounter_currPos_Left += new_step;
  }
  /* right */
  c12 = QuadCounter_GetValRight();
  new_step = QuadCounter_Quad_Table[QuadCounter_last_quadrature_value_right][c12];
  QuadCounter_last_quadrature_value_right = c12;
  if (new_step == QUAD_ERROR) {
    QuadCounter_errorsRight++;
  } else if (new_step != 0) {
    QuadCounter_currPos_Right += new_step;
  }
}

uint8_t QuadCounter_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res=ERR_OK;

  if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((const char *)cmd, "quad help")==0) {
    McuShell_SendHelpStr((const unsigned char*)"quad", (const unsigned char*)"Quadrature counter command group\r\n", io->stdOut);
    McuShell_SendHelpStr((const unsigned char*)"  help|status", (const unsigned char*)"Print help or status information\r\n", io->stdOut);
    McuShell_SendHelpStr((const unsigned char*)"  reset", (const unsigned char*)"Reset the current position counter\r\n", io->stdOut);
    *handled = TRUE;
  } else if (McuUtility_strcmp((const char*)cmd, McuShell_CMD_STATUS)==0 || McuUtility_strcmp((const char*)cmd, "quad status")==0) {
    McuShell_SendStatusStr((const unsigned char*)"quad", (const unsigned char*)"Quadrature counter status\r\n", io->stdOut);
    McuShell_SendStatusStr((const unsigned char*)"  pos left", (const unsigned char*)"", io->stdOut);
    McuShell_SendNum32s((int32_t)QuadCounter_currPos_Left, io->stdOut);
    McuShell_SendStr((const unsigned char*)"\r\n", io->stdOut);

    McuShell_SendStatusStr((const unsigned char*)"  error left", (const unsigned char*)"", io->stdOut);
    McuShell_SendNum32s((int32_t)QuadCounter_errorsLeft, io->stdOut);
    McuShell_SendStr((const unsigned char*)"\r\n", io->stdOut);

    McuShell_SendStatusStr((const unsigned char*)"  left C1 C2", (const unsigned char*)"", io->stdOut);
    if (QuadCounter_GET_C1_PIN_LEFT()) {
      McuShell_SendStr((const unsigned char*)"1 ", io->stdOut);
    } else {
      McuShell_SendStr((const unsigned char*)"0 ", io->stdOut);
    }
    if (QuadCounter_GET_C2_PIN_LEFT()) {
      McuShell_SendStr((const unsigned char*)"1\r\n", io->stdOut);
    } else {
      McuShell_SendStr((const unsigned char*)"0\r\n", io->stdOut);
    }
    McuShell_SendStatusStr((const unsigned char*)"  pos left", (const unsigned char*)"", io->stdOut);
    McuShell_SendNum32s((int32_t)QuadCounter_currPos_Left, io->stdOut);
    McuShell_SendStr((const unsigned char*)"\r\n", io->stdOut);

    McuShell_SendStatusStr((const unsigned char*)"  error right", (const unsigned char*)"", io->stdOut);
    McuShell_SendNum32s((int32_t)QuadCounter_errorsRight, io->stdOut);
    McuShell_SendStr((const unsigned char*)"\r\n", io->stdOut);

    McuShell_SendStatusStr((const unsigned char*)"  right C1 C2", (const unsigned char*)"", io->stdOut);
    if (QuadCounter_GET_C1_PIN_RIGHT()) {
      McuShell_SendStr((const unsigned char*)"1 ", io->stdOut);
    } else {
      McuShell_SendStr((const unsigned char*)"0 ", io->stdOut);
    }
    if (QuadCounter_GET_C2_PIN_RIGHT()) {
      McuShell_SendStr((const unsigned char*)"1\r\n", io->stdOut);
    } else {
      McuShell_SendStr((const unsigned char*)"0\r\n", io->stdOut);
    }

    McuShell_SendStatusStr((const unsigned char*)"  pos right", (const unsigned char*)"", io->stdOut);
    McuShell_SendNum32s((int32_t)QuadCounter_currPos_Right, io->stdOut);
    McuShell_SendStr((const unsigned char*)"\r\n", io->stdOut);

    *handled = TRUE;
  } else if (McuUtility_strcmp((const char*)cmd, "quad reset")==0) {
    QuadCounter_SetPosLeft(0);
    QuadCounter_SetPosRight(0);
    *handled = TRUE;
  }
  return res;
}

void QuadCounter_Deinit(void) {
  C1_Left = McuGPIO_DeinitGPIO(C1_Left);
  C2_Left = McuGPIO_DeinitGPIO(C2_Left);
  C1_Right = McuGPIO_DeinitGPIO(C1_Right);
  C2_Right = McuGPIO_DeinitGPIO(C2_Right);
}

void QuadCounter_EnablePullups(void) {
  McuGPIO_SetPullResistor(C1_Left, McuGPIO_PULL_UP);
  McuGPIO_SetPullResistor(C1_Right, McuGPIO_PULL_UP);
  McuGPIO_SetPullResistor(C2_Left, McuGPIO_PULL_UP);
  McuGPIO_SetPullResistor(C2_Right, McuGPIO_PULL_UP);
}

static void QuadCounter_PinInit(void) {
  McuGPIO_Config_t gpioConfig;

  QUADCOUNTER_CONFIG_ENABLE_CLOCK();
  McuGPIO_GetDefaultConfig(&gpioConfig);
#if QUADCOUNTER_CONFIG_ENABLE_PULL
  gpioConfig.hw.pull = McuGPIO_PULL_UP;
#endif
  gpioConfig.isInput = true;
  gpioConfig.hw.gpio = QUADCOUNTER_CONFIG_LEFT_A_GPIO;
  gpioConfig.hw.port = QUADCOUNTER_CONFIG_LEFT_A_PORT;
  gpioConfig.hw.pin = QUADCOUNTER_CONFIG_LEFT_A_PIN;
  C1_Left = McuGPIO_InitGPIO(&gpioConfig);
  if (C1_Left==NULL) {
    for(;;){}
  }
  gpioConfig.hw.gpio = QUADCOUNTER_CONFIG_LEFT_B_GPIO;
  gpioConfig.hw.port = QUADCOUNTER_CONFIG_LEFT_B_PORT;
  gpioConfig.hw.pin = QUADCOUNTER_CONFIG_LEFT_B_PIN;
  C2_Left = McuGPIO_InitGPIO(&gpioConfig);
  if (C2_Left==NULL) {
    for(;;){}
  }

  gpioConfig.hw.gpio = QUADCOUNTER_CONFIG_RIGHT_A_GPIO;
  gpioConfig.hw.port = QUADCOUNTER_CONFIG_RIGHT_A_PORT;
  gpioConfig.hw.pin = QUADCOUNTER_CONFIG_RIGHT_A_PIN;
  C1_Right = McuGPIO_InitGPIO(&gpioConfig);
  if (C1_Right==NULL) {
    for(;;){}
  }
  gpioConfig.hw.gpio = QUADCOUNTER_CONFIG_RIGHT_B_GPIO;
  gpioConfig.hw.port = QUADCOUNTER_CONFIG_RIGHT_B_PORT;
  gpioConfig.hw.pin = QUADCOUNTER_CONFIG_RIGHT_B_PIN;
  C2_Right = McuGPIO_InitGPIO(&gpioConfig);
  if (C2_Right==NULL) {
    for(;;){}
  }
}

#define QUADRATURE_PIT_BASEADDR       PIT
#define QUADRATURE_PIT_SOURCE_CLOCK   CLOCK_GetFreq(kCLOCK_BusClk)
#define QUADRATURE_PIT_CHANNEL        kPIT_Chnl_0
#define QUADRATURE_PIT_HANDLER        PIT0_IRQHandler
#define QUADRATURE_PIT_IRQ_ID         PIT0_IRQn

void QUADRATURE_PIT_HANDLER(void) {
  PIT_ClearStatusFlags(QUADRATURE_PIT_BASEADDR, QUADRATURE_PIT_CHANNEL, kPIT_TimerFlag);
  QuadCounter_Sample();
  __DSB();
}

void Quadrature_StartQuadratureTimer(void) {
  PIT_StartTimer(QUADRATURE_PIT_BASEADDR, QUADRATURE_PIT_CHANNEL);
}

void Quadrature_StopQuadratureTimer(void) {
  PIT_StopTimer(QUADRATURE_PIT_BASEADDR, QUADRATURE_PIT_CHANNEL);
}

static void Quadrature_InitQuadratureTimer(void) {
  pit_config_t config;

  PIT_GetDefaultConfig(&config);
  config.enableRunInDebug = false;
  PIT_Init(QUADRATURE_PIT_BASEADDR, &config);
  PIT_SetTimerPeriod(QUADRATURE_PIT_BASEADDR, QUADRATURE_PIT_CHANNEL, USEC_TO_COUNT(200U, QUADRATURE_PIT_SOURCE_CLOCK));
  PIT_EnableInterrupts(QUADRATURE_PIT_BASEADDR, QUADRATURE_PIT_CHANNEL, kPIT_TimerInterruptEnable);
  NVIC_SetPriority(QUADRATURE_PIT_IRQ_ID, 1);
  EnableIRQ(QUADRATURE_PIT_IRQ_ID);
  PIT_StartTimer(QUADRATURE_PIT_BASEADDR, QUADRATURE_PIT_CHANNEL);
}

void QuadCounter_Init(void) {
  QuadCounter_PinInit();
  Quadrature_InitQuadratureTimer();
  QuadCounter_currPos_Left = 0;
  QuadCounter_currPos_Right = 0;
  QuadCounter_last_quadrature_value_left = QuadCounter_GET_C1_C2_PINS_LEFT();
  QuadCounter_last_quadrature_value_right = QuadCounter_GET_C1_C2_PINS_RIGHT();
#if QuadCounter_SWAP_PINS_AT_RUNTIME
  QuadCounter_swappedPins_Left = FALSE;
  QuadCounter_swappedPins_Right = FALSE;
#endif
}
