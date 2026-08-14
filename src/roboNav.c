/**
 * \file
 * \brief Robot navigation with a a nav/joystick (real or virtual).
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_ROBO_NAV
#include "roboNav.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "McuDebounce.h"
#if PL_CONFIG_USE_DRIVE
  #include "drive.h"
#endif
#if PL_CONFIG_USE_BUZZER
  #include "buzzer.h"
#endif
#include "buttons.h"

#define ROBO_NAV_TIMEOUT_PERIOD_MS   (1000)
static TimerHandle_t timeoutTimer; /* timer for timeout and to stop robot in case of no communication */

static void TimeoutRestart(void) {
  (void)xTimerStart(timeoutTimer, pdMS_TO_TICKS(100));
}

static void TimeoutStop(void) {
  (void)xTimerStop(timeoutTimer, pdMS_TO_TICKS(100));
}

static void vTimerCallbacktTimeout(TimerHandle_t pxTimer) {
  McuLog_trace("timeout timer expired");
#if PL_CONFIG_USE_DRIVE
  DRV_SetMode(DRV_MODE_STOP);
#endif
}

void RoboNav_OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event) {
  const char *p = NULL;
#if PL_CONFIG_USE_DRIVE
  DRV_Mode newMode = DRV_MODE_NONE;
  int32_t speedL = -1;
  int32_t speedR = -1;
  int32_t speed = 0;
#endif

  if (event==MCUDBNC_EVENT_PRESSED) {
    switch(button) {
      case BUTTONS_NAV_UP:      p = "pressed up";      break;
      case BUTTONS_NAV_DOWN:    p = "pressed down";    break;
      case BUTTONS_NAV_LEFT:    p = "pressed left";    break;
      case BUTTONS_NAV_RIGHT:   p = "pressed right";   break;
      case BUTTONS_NAV_CENTER:  p = "pressed center";  break;
      default:                  p = NULL;              break;
    } /* switch */
  } else if (event==MCUDBNC_EVENT_PRESSED_REPEAT) {
    switch(button) {
      case BUTTONS_NAV_UP:      p = "long up";      break;
      case BUTTONS_NAV_DOWN:    p = "long down";    break;
      case BUTTONS_NAV_LEFT:    p = "long left";    break;
      case BUTTONS_NAV_RIGHT:   p = "long right";   break;
      case BUTTONS_NAV_CENTER:  p = "long center";  break;
      default:                  p = NULL;           break;
    } /* switch */
  } else if (event==MCUDBNC_EVENT_LONG_PRESSED_REPEAT) {
   switch(button) {
      case BUTTONS_NAV_UP:      p = "long repeat up";      break;
      case BUTTONS_NAV_DOWN:    p = "long repeat down";    break;
      case BUTTONS_NAV_LEFT:    p = "long repeat left";    break;
      case BUTTONS_NAV_RIGHT:   p = "long repeat right";   break;
      case BUTTONS_NAV_CENTER:  p = "long repeat center";  break;
      default:                  p = NULL;                  break;
    } /* switch */
  } else if (event==MCUDBNC_EVENT_RELEASED) {
    switch(button) {
      case BUTTONS_NAV_UP:      p = "release up";      break;
      case BUTTONS_NAV_DOWN:    p = "release down";    break;
      case BUTTONS_NAV_LEFT:    p = "release left";    break;
      case BUTTONS_NAV_RIGHT:   p = "release right";   break;
      case BUTTONS_NAV_CENTER:  p = "release center";  break;
      default:                  p = NULL;              break;    
    } /* switch */
  } else if (event==MCUDBNC_EVENT_LONG_RELEASED) {
    switch(button) {
      case BUTTONS_NAV_UP:      p = "long release up";      break;
      case BUTTONS_NAV_DOWN:    p = "long release down";    break;
      case BUTTONS_NAV_LEFT:    p = "long release left";    break;
      case BUTTONS_NAV_RIGHT:   p = "long release right";   break;
      case BUTTONS_NAV_CENTER:  p = "long release center";  break;
      default:                  p = NULL;                   break;
    } /* switch */
  } /* if-else */

#if PL_CONFIG_USE_DRIVE
  switch(event) {
    case MCUDBNC_EVENT_PRESSED:
      speed = 500;
      break;
    case MCUDBNC_EVENT_PRESSED_REPEAT:
      speed = 1000;
      break;
    case MCUDBNC_EVENT_LONG_PRESSED:
      speed = 1500;
      break;
    case MCUDBNC_EVENT_LONG_PRESSED_REPEAT:
      speed = 2000;
      break;
    case MCUDBNC_EVENT_RELEASED:
    case MCUDBNC_EVENT_LONG_RELEASED:
    default:
      speed = 0;
      break;
  } /* switch */

   switch(button) {
      case BUTTONS_NAV_UP:
        speedL = speed;
        speedR = speed;
        newMode = DRV_MODE_SPEED;
        TimeoutRestart();
        break;
      case BUTTONS_NAV_DOWN:
        speedL = -speed;
        speedR = -speed;
        newMode = DRV_MODE_SPEED;
        TimeoutRestart();
        break;
      case BUTTONS_NAV_LEFT:
        speedL = -speed;
        speedR = speed;
        newMode = DRV_MODE_SPEED;
        TimeoutRestart();
        break;
      case BUTTONS_NAV_RIGHT:
        speedL = speed;
        speedR = -speed;
        newMode = DRV_MODE_SPEED;
        TimeoutRestart();
        break;
      case BUTTONS_NAV_CENTER:
        newMode = DRV_MODE_STOP;
        TimeoutStop();
        break;
      default:
        break;
    } /* switch */
#endif /* PL_CONFIG_USE_DRIVE */

#if PL_CONFIG_USE_DRIVE
  if (speedL!=-1 && speedR!=-1) {
    DRV_SetSpeed(speedL, speedR);
  }
  if (DRV_GetMode()!=newMode) { /* changed mode? */
    DRV_SetMode(newMode);
  }
#endif /* PL_CONFIG_USE_DRIVE */
  if (p!=NULL) {
    McuLog_info(p);
  #if PL_CONFIG_USE_BUZZER
    Buzzer_Beep(500, 200);
  #endif
  }
}

static uint8_t HandleNavCommand(const char *cmd) {
  Buttons_e button;
  McuDbnc_EventKinds kind = MCUDBNC_EVENT_PRESSED;

  while(*cmd!='\0' && *cmd!=' ') {
    switch(*cmd) {
      case 'u': button = BUTTONS_NAV_UP; break;
      case 'd': button = BUTTONS_NAV_DOWN; break;
      case 'l': button = BUTTONS_NAV_LEFT; break;
      case 'r': button = BUTTONS_NAV_RIGHT; break;
      case 'c': button = BUTTONS_NAV_CENTER; break;
      default:
        break;
    }
    cmd++;
  }
  if (*cmd!=' ') {
    McuLog_error("must be a space between <udlrc> and on|off");
    return ERR_FAILED;
  }
  cmd++;
  if (McuUtility_strcmp((char*)cmd, (char*)"on")==0) {
    kind = MCUDBNC_EVENT_PRESSED;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"off")==0) {
    kind = MCUDBNC_EVENT_RELEASED;
  } else {
    McuLog_error("must be 'on' or 'off'");
    return ERR_FAILED;
  }
  RoboNav_OnButtonEvent(button, kind);
  return ERR_OK;
}

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"robonav", (unsigned char*)"Robot navigation status\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"robonav", (unsigned char*)"Group of robot navigation commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  nav <udlrc> on|off", (unsigned char*)"Use nav (up, down, left, right, center) buttons for the robot\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t RoboNav_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"robonav help")==0) {
    *handled = true;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"robonav status")==0) {
    *handled = true;
    return PrintStatus(io);
  } else if (McuUtility_strncmp((char*)cmd, (char*)"robonav nav ", sizeof("robonav nav ")-1)==0) {
    *handled = true;
    return HandleNavCommand((char*)(cmd+sizeof("robonav nav ")-1));
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

void RoboNav_Init(void) {
  /* use a timeout on the robot: if after a move command we do not get a stop, we get a timeout and stop automatically */
  timeoutTimer = xTimerCreate(
        "timeout", /* name */
        pdMS_TO_TICKS(ROBO_NAV_TIMEOUT_PERIOD_MS), /* period/time */
        pdFALSE, /* auto reload */
        (void*)0, /* timer ID */
        vTimerCallbacktTimeout); /* callback */
  if (timeoutTimer==NULL) {
    McuLog_fatal("failed creating timer");
    for(;;); /* failure! */
  }
}

#endif /* PL_CONFIG_USE_ROBO_NAV */