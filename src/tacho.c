/**
 * \file
 * \brief Tachometer Module
 * \author Erich Styger, erich.styger@hslu.ch
* \license SPDX-License-Identifier: BSD-3-Clause
 * Module to calculate the speed based on the quadrature counter.
 */

#include "platform.h" /* interface to the platform */
#if PL_CONFIG_USE_TACHO
#include "tacho.h"    /* our own interface */
#include "quadCounter.h"
#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"
#endif
#include "McuUtility.h"
#include "McuRTOS.h"
#include "McuCriticalSection.h"

#define TACHO_NOF_HISTORY (16U+1U) 
  /*!< number of samples for speed calculation (>0):the more, the better, but the slower. */

static QuadCounter_QuadCntrType Tacho_LeftPosHistory[TACHO_NOF_HISTORY], Tacho_RightPosHistory[TACHO_NOF_HISTORY];
  /*!< for better accuracy, we calculate the speed over some samples */
static uint8_t Tacho_PosHistory_Index = 0;
  /*!< position index in history */

static int32_t Tacho_currLeftSpeed = 0, Tacho_currRightSpeed = 0;
  /*!< position index in history */

static TimerHandle_t timerTacho; /* timer for sampling tacho data */

static bool Tacho_isEnabled = true;

int32_t Tacho_GetSpeed(bool isLeft) {
  if (isLeft) {
    return Tacho_currLeftSpeed;
  } else {
    return Tacho_currRightSpeed;
  }
}

void Tacho_CalcSpeed(void) {
  /* we calculate the speed as follow:
                              1000         
  steps/sec =  delta * ----------------- 
                       samplePeriod (ms) 
  As this function may be called very frequently, it is important to make it as efficient as possible!
   */
  int32_t deltaLeft, deltaRight, newLeft, newRight, oldLeft, oldRight;
  int32_t speedLeft, speedRight;
  bool negLeft, negRight;
  McuCriticalSection_CriticalVariable()

  if (!Tacho_isEnabled) {
    return;
  }
  McuCriticalSection_EnterCritical();
  oldLeft = (int32_t)Tacho_LeftPosHistory[Tacho_PosHistory_Index]; /* oldest left entry */
  oldRight = (int32_t)Tacho_RightPosHistory[Tacho_PosHistory_Index]; /* oldest right entry */
  if (Tacho_PosHistory_Index==0) { /* get newest entry */
    newLeft = (int32_t)Tacho_LeftPosHistory[TACHO_NOF_HISTORY-1];
    newRight = (int32_t)Tacho_RightPosHistory[TACHO_NOF_HISTORY-1];
  } else {
    newLeft = (int32_t)Tacho_LeftPosHistory[Tacho_PosHistory_Index-1];
    newRight = (int32_t)Tacho_RightPosHistory[Tacho_PosHistory_Index-1];
  }
  McuCriticalSection_ExitCritical();
  deltaLeft = oldLeft-newLeft; /* delta of oldest position and most recent one */
  /* use unsigned arithmetic */
  if (deltaLeft < 0) {
    deltaLeft = -deltaLeft;
    negLeft = TRUE;
  } else {
    negLeft = FALSE;
  }
  deltaRight = oldRight-newRight; /* delta of oldest position and most recent one */
  /* use unsigned arithmetic */
  if (deltaRight < 0) {
    deltaRight = -deltaRight;
    negRight = TRUE;
  } else {
    negRight = FALSE;
  }
  /* calculate speed. this is based on the delta and the time (number of samples or entries in the history table) */
  speedLeft = (int32_t)(deltaLeft*1000U/(TACHO_SAMPLE_PERIOD_MS*(TACHO_NOF_HISTORY-1)));
  if (negLeft) {
    speedLeft = -speedLeft;
  }
  speedRight = (int32_t)(deltaRight*1000U/(TACHO_SAMPLE_PERIOD_MS*(TACHO_NOF_HISTORY-1)));
  if (negRight) {
    speedRight = -speedRight;
  }
  Tacho_currLeftSpeed = -speedLeft; /* store current speed in global variable */
  Tacho_currRightSpeed = -speedRight; /* store current speed in global variable */
}

static void Tacho_Sample(void) {
  /* calling with period of TACHO_SAMPLE_PERIOD_MS from FreeRTOS timer */
  Tacho_LeftPosHistory[Tacho_PosHistory_Index] = QuadCounter_GetPosLeft();
  Tacho_RightPosHistory[Tacho_PosHistory_Index] = QuadCounter_GetPosRight();
  Tacho_PosHistory_Index++;
  if (Tacho_PosHistory_Index >= TACHO_NOF_HISTORY) {
    Tacho_PosHistory_Index = 0;
  }
}

static void vTimerCallbackTacho(TimerHandle_t pxTimer) {
  Tacho_Sample();
}

void Tacho_StartSamplingTimer(void) {
  (void)xTimerStart(timerTacho, portMAX_DELAY);
  Tacho_isEnabled = true;
}

void Tacho_StopSamplingTimer(void) {
  Tacho_isEnabled = false;
  (void)xTimerStop(timerTacho, portMAX_DELAY);
}

#if PL_CONFIG_USE_SHELL
/*!
 * \brief Prints the system low power status
 * \param io I/O channel to use for printing status
 */
static void Tacho_PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"tacho", (unsigned char*)"Tachometer status\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  enabled", Tacho_isEnabled?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  L speed", (unsigned char*)"", io->stdOut);
  McuShell_SendNum32s(Tacho_GetSpeed(TRUE), io->stdOut);
  McuShell_SendStr((unsigned char*)" steps/sec\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  R speed", (unsigned char*)"", io->stdOut);
  McuShell_SendNum32s(Tacho_GetSpeed(FALSE), io->stdOut);
  McuShell_SendStr((unsigned char*)" steps/sec\r\n", io->stdOut);
}

/*! 
 * \brief Prints the help text to the console
 * \param io I/O channel to be used
 */
static void Tacho_PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"tacho", (unsigned char*)"Group of tacho commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows tacho help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  enable|disable", (unsigned char*)"Enable and disable tacho\r\n", io->stdOut);
}

uint8_t Tacho_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"tacho help")==0) {
    *handled = TRUE;
    Tacho_PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"tacho status")==0) {
    *handled = TRUE;
    Tacho_PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"tacho enable")==0) {
    *handled = TRUE;
    Tacho_StartSamplingTimer();
  } else if (McuUtility_strcmp((char*)cmd, (char*)"tacho disable")==0) {
    *handled = TRUE;
    Tacho_StopSamplingTimer();
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

void Tacho_Deinit(void) {
}

void Tacho_Init(void) {
  Tacho_currLeftSpeed = 0;
  Tacho_currRightSpeed = 0;
  Tacho_PosHistory_Index = 0;

  timerTacho = xTimerCreate(
    "tachoTimer", /* name */
    pdMS_TO_TICKS(TACHO_SAMPLE_PERIOD_MS), /* period/time */
    pdTRUE, /* auto reload */
    (void*)0, /* timer ID */
    vTimerCallbackTacho); /* callback */
  if (timerTacho==NULL) {
    for(;;); /* failure! */
  }
  xTimerStart(timerTacho, portMAX_DELAY);
}

#endif /* PL_HAS_MOTOR_TACHO */

