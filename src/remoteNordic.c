/**
 * \file
 * \brief Module to handle the remote controller with Nordic NRF24L01+. Code is both for Robot and ESP32.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module implements the catch-the-flag extensions.
 */

#include "platform.h"
#if PL_CONFIG_USE_REMOTE_NORDIC
#include "remoteNordic.h"
#include "RNet_App.h"
#include "McuShell.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "shell.h"
#if PL_CONFIG_HAS_BATTERY_ADC
  #include "Battery.h"
#endif
#if PL_CONFIG_USE_BUZZER
  #include "buzzer.h"
#endif
#include "buttons.h"
#include "leds.h"
#include "McuLED.h"
#if PL_CONFIG_USE_DRIVE
  #include "Drive.h"
#endif
#if PL_CONFIG_HAS_LCD
  #include "LCD.h"
#endif
#if PL_CONFIG_USE_MININI
  #include "minIni/McuMinINI.h"
#endif
#if PL_CONFIG_USE_ROBO_NAV
  #include "roboNav.h"
#endif
#include "RNet_App.h"

#if PL_CONFIG_USE_MININI
  /* section for RemoteNordic settings */
  #define NVMC_MININI_FILE_NAME       "settings.ini" /* 'file' name used */
  #define NVMC_MININI_SECTION_NORDIC_REMOTE             "nordicRemote"
  #define NVMC_MININI_KEY_NORDIC_REMOTE_SADDR              "saddr"     /* number, RNet source addr */
  #define NVMC_MININI_KEY_NORDIC_REMOTE_DADDR              "daddr"     /* number, RNet destination addr */
#endif

#define NVMC_MININI_KEY_NORDIC_REMOTE_SADDR_DEFAULT   0xff
#define NVMC_MININI_KEY_NORDIC_REMOTE_DADDR_DEFAULT   0xff

#if PL_CONFIG_IS_ESP32
  static RemoteNordic_RobotMoveStatus_e RemoteNordic_RobotMoveStatus = RemoteNordic_MOVE_STATUS_UNKNOWN;
  static uint32_t RemoteNordic_RobotBatteryVoltage_mV = 0;
#elif PL_CONFIG_IS_ROBOT
  #define NORDIC_REMOTE_TIMEOUT_PERIOD_MS   (1000)
  static TimerHandle_t timeoutTimer; /* timer for timeout and to stop robot in case of no communication */
#endif

static void RemoteNordic_SetRNetSourceAddr(RAPP_ShortAddrType addr) {
  RNETA_SetSrcAddr(addr);
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_NORDIC_REMOTE, NVMC_MININI_KEY_NORDIC_REMOTE_SADDR, addr, NVMC_MININI_FILE_NAME);
#endif
}

static RAPP_ShortAddrType RemoteNordic_GetRNetSourceAddr(void)  {
  return RNETA_GetSrcAddr();
}

static void RemoteNordic_SetRNetDestinationAddr(RAPP_ShortAddrType addr) {
  RNETA_SetDestAddr(addr);
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_NORDIC_REMOTE, NVMC_MININI_KEY_NORDIC_REMOTE_DADDR, addr, NVMC_MININI_FILE_NAME);
#endif
}

static RAPP_ShortAddrType RemoteNordic_GetRNetDestinationAddr(void)  {
  return RNETA_GetDestAddr();
}

#if PL_CONFIG_IS_ROBOT
void RemoteNordic_TimeoutRestart(void) {
  (void)xTimerStart(timeoutTimer, pdMS_TO_TICKS(100));
}

void RemoteNordic_TimeoutStop(void) {
  (void)xTimerStop(timeoutTimer, pdMS_TO_TICKS(100));
}

static void vTimerCallbacktTimeout(TimerHandle_t pxTimer) {
  McuLog_trace("timeout timer expired");
#if PL_CONFIG_USE_DRIVE
  if (RemoteNordic_GetRobotMoveStatus()!=RemoteNordic_MOVE_STATUS_STOPPED) {
    RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_NOTIFY_VALUE, RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE, RemoteNordic_MOVE_STATUS_STOPPED, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NO_ACK);
  }
#endif
}
#endif /* PL_CONFIG_IS_ROBOT */

int RemoteNordic_QueryRobotBatteryVoltage(void) {
  return RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE, RAPP_MSG_TYPE_DATA_ID_BATTERY_V, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
}

int RemoteNordic_QueryRobotButton(void) {
  return RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE, RAPP_MSG_TYPE_DATA_ID_BUTTON, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
}

#if PL_CONFIG_IS_ESP32
static void RemoteNordic_SetRobotBatteryVoltage(uint32_t mV) {
  RemoteNordic_RobotBatteryVoltage_mV = mV;
}
#endif

#if PL_CONFIG_IS_ESP32
uint32_t RemoteNordic_GetRobotBatteryVoltage_mV(void) {
  return RemoteNordic_RobotBatteryVoltage_mV;
}
#endif

RemoteNordic_RobotMoveStatus_e RemoteNordic_GetRobotMoveStatus(void) {
#if PL_CONFIG_IS_ROBOT
  #if PL_CONFIG_USE_DRIVE
    switch(DRV_GetMode()) {
      case DRV_MODE_STOP: return RemoteNordic_MOVE_STATUS_STOPPED;
      case DRV_MODE_POS:
      case DRV_MODE_SPEED: return RemoteNordic_MOVE_STATUS_MOVING;
      default: return RemoteNordic_MOVE_STATUS_UNKNOWN;
    }
  #else
    return RemoteNordic_MOVE_STATUS_UNKNOWN;
  #endif
#else
  return RemoteNordic_RobotMoveStatus;
#endif
}

#if PL_CONFIG_IS_ESP32
void RemoteNordic_SetRobotMoveStatus(RemoteNordic_RobotMoveStatus_e status) {
  RemoteNordic_RobotMoveStatus = status;
}
#endif

#if PL_CONFIG_USE_BUTTONS
static bool RemoteNordic_ButtonIsPressed(void) {
#if PL_CONFIG_IS_ROBOT
  return Buttons_IsPressed(BUTTONS_USER);
#elif PL_CONFIG_IS_ESP32
  return Buttons_IsPressed(BUTTONS_NAV_CENTER);
#endif
}
#endif

#if PL_CONFIG_IS_ROBOT
static void RemoteNordic_RobotOnButtonEvent(Buttons_e button, McuDbnc_EventKinds event) {
  RAPP_MSG_DataIDType notifyID = RAPP_MSG_TYPE_DATA_ID_NONE;
  uint32_t notifyValue = 0;

  switch(button) {
    case BUTTONS_NAV_UP:
      notifyID = RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE;
      notifyValue = RemoteNordic_MOVE_STATUS_MOVING;
      break;
    case BUTTONS_NAV_DOWN:
      notifyID = RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE;
      notifyValue = RemoteNordic_MOVE_STATUS_MOVING;
      break;
    case BUTTONS_NAV_LEFT:
      notifyID = RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE;
      notifyValue = RemoteNordic_MOVE_STATUS_MOVING;
      break;
    case BUTTONS_NAV_RIGHT:
      notifyID = RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE;
      notifyValue = RemoteNordic_MOVE_STATUS_MOVING;
      break;
    case BUTTONS_NAV_CENTER:
      notifyID = RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE;
      notifyValue = RemoteNordic_MOVE_STATUS_STOPPED;
      break;
    default:
      notifyID = RAPP_MSG_TYPE_DATA_ID_NONE;
      break;
  } /* switch */
  if (notifyID!=RAPP_MSG_TYPE_DATA_ID_NONE) {
    RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_NOTIFY_VALUE, notifyID, notifyValue, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NO_ACK);
  }
#if PL_CONFIG_USE_ROBO_NAV
  RoboNav_OnButtonEvent(button, event);
#endif
}
#endif /* PL_CONFIG_IS_ROBOT */

#if McuLib_CONFIG_CPU_IS_ESP32
void RemoteNordic_ESP32OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event) {
#if PL_CONFIG_HAS_LCD /* navigation button messages are handled by the LCD module and forwarded if configured as such */
  LCD_OnButtonEvent(button, event);
#elif PL_CONFIG_USE_NORDIC_RADIO /* send directly navigation button messages */
  uint32_t val = (event<<16)|button;
  McuLog_info("Sending RAPP_MSG_TYPE_DATA_ID_NAV notify value: button=%d, event=%d", button, event);
  RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_NOTIFY_VALUE, RAPP_MSG_TYPE_DATA_ID_NAV, val, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NO_ACK);
#endif
}
#endif /* McuLib_CONFIG_CPU_IS_ESP32 */

#if PL_CONFIG_USE_NORDIC_RADIO
uint8_t RemoteNordic_HandleRemoteRxMessage(RAPP_MSG_Type type, uint8_t size, uint8_t *data, RNWK_ShortAddrType srcAddr, bool *handled, RPHY_PacketDesc *packet) {
  RAPP_MSG_DataIDType msgID;
  uint32_t msgValue;

  switch(type) {
    /* ------------ General data messages -------------------------------*/
    #if PL_CONFIG_IS_ROBOT
    case RAPP_MSG_TYPE_JOYSTICK_BTN:
      *handled =true;
      char button = data[0];
      switch(button) {
        case 'a': RemoteNordic_RobotOnButtonEvent(BUTTONS_NAV_UP, MCUDBNC_EVENT_PRESSED); break;
        case 'c': RemoteNordic_RobotOnButtonEvent(BUTTONS_NAV_DOWN, MCUDBNC_EVENT_PRESSED); break;
        case 'd': RemoteNordic_RobotOnButtonEvent(BUTTONS_NAV_LEFT, MCUDBNC_EVENT_PRESSED); break;
        case 'b': RemoteNordic_RobotOnButtonEvent(BUTTONS_NAV_RIGHT, MCUDBNC_EVENT_PRESSED); break;
        case 's': RemoteNordic_RobotOnButtonEvent(BUTTONS_NAV_CENTER, MCUDBNC_EVENT_PRESSED); break; /* stop */
        default:
          break;
      }
      break;
    #endif
    /* ------------ Received a request to set a value -------------------*/
    case RAPP_MSG_TYPE_REQUEST_SET_VALUE:
      msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
      switch(msgID) {
        default:
          break;
      } /* switch */
      break;
    /* ------------ Received a Query -> send back response -------------------*/
    case RAPP_MSG_TYPE_QUERY_VALUE:
      msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
      switch(msgID) {
      #if PL_CONFIG_HAS_BATTERY_ADC
        case RAPP_MSG_TYPE_DATA_ID_BATTERY_V:
          *handled = true;
          uint16_t value16;
          BATT_MeasureBatteryVoltage(&value16);
          RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE, msgID, (uint32_t)(value16*10), srcAddr, RPHY_PACKET_FLAGS_NONE);
        #if PL_CONFIG_USE_BUZZER
          Buzzer_Beep(500, 200);
        #endif
          break;
      #endif
      #if PL_CONFIG_USE_BUTTONS
        case RAPP_MSG_TYPE_DATA_ID_BUTTON:
          *handled =true;
          msgValue = RemoteNordic_ButtonIsPressed();
          RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE, msgID, msgValue, srcAddr, RPHY_PACKET_FLAGS_NONE);
          break;
      #endif
      #if PL_CONFIG_IS_ROBOT
        case RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE:
          *handled =true;
          msgValue = RemoteNordic_GetRobotMoveStatus();
          RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE, msgID, msgValue, srcAddr, RPHY_PACKET_FLAGS_NONE);
          break;
      #endif
        default:
          break;
      }
      break;
    /* ------------ Received the response to a a Query -------------------*/
    case RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE: /* received data value for request */
      msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
      msgValue = McuUtility_GetValue32LE(&data[2]);
      switch(msgID) {
        case RAPP_MSG_TYPE_DATA_ID_BATTERY_V:
          *handled = true;
          McuLog_info("Rx: Battery voltage is %d mV", msgValue);
          #if McuLib_CONFIG_CPU_IS_ESP32
          RemoteNordic_SetRobotBatteryVoltage(msgValue);
          #endif
          break;
        case RAPP_MSG_TYPE_DATA_ID_BUTTON:
          *handled = true;
          McuLog_info("Rx: Button is %s", msgValue==0?"off":"on");
          break;
        default:
          break;
      }
      break;
      /* ------------ Received a Notifications -------------------*/
      case RAPP_MSG_TYPE_NOTIFY_VALUE: /* received notification */
        msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
        msgValue = McuUtility_GetValue32LE(&data[2]);
        switch(msgID) {
        #if PL_CONFIG_HAS_BATTERY_ADC
          case RAPP_MSG_TYPE_DATA_ID_BATTERY_V:
            *handled = true;
            McuLog_info("Notify: Battery voltage is %d mV", msgValue);
            break;
        #endif
          case RAPP_MSG_TYPE_DATA_ID_BUTTON:
            *handled = true;
            McuLog_info("Notify: Button is %s", msgValue==0?"off":"on");
            break;

          case RAPP_MSG_TYPE_DATA_ID_NAV:
            *handled = true;
            McuLog_info("Notify: Nav is 0x%0x", msgValue);
            #if PL_CONFIG_IS_ROBOT
              RemoteNordic_RobotOnButtonEvent(msgValue&0xffff /* button */, (msgValue>>16)&0xffff /* button press event */);
            #endif
            break;

          case RAPP_MSG_TYPE_DATA_ID_ROBOT_MOVE:
            *handled = true;
            McuLog_info("Notify: Robot move status is %d %s", msgValue, msgValue==0?"stopped":msgValue==1?"stopped":msgValue==2?"moving":"unknown");
            #if McuLib_CONFIG_CPU_IS_ESP32
              RemoteNordic_SetRobotMoveStatus((RemoteNordic_RobotMoveStatus_e)msgValue);
            #endif
            break;

          default:
            break;
        } /* switch */
        break;

    default:
      break;
  } /* switch */
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_NORDIC_RADIO */

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  uint8_t buf[48];
  McuShell_SendStatusStr((unsigned char*)"RemoteNordic", (unsigned char*)"RemoteNordic status\r\n", io->stdOut);
#if McuLib_CONFIG_CPU_IS_ESP32
  McuUtility_Num32uToStr(buf, sizeof(buf), RemoteNordic_GetRobotBatteryVoltage_mV());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" mV\r\n");
  McuShell_SendStatusStr((unsigned char*)"  Battery", buf, io->stdOut);
#endif
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"src:0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), RemoteNordic_GetRNetSourceAddr());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", dst:0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), RemoteNordic_GetRNetDestinationAddr());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  RNetAddr", buf, io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"RemoteNordic", (unsigned char*)"Group of RemoteNordic commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows NordicRemote help or status\r\n", io->stdOut);
#if PL_CONFIG_IS_ESP32
  McuShell_SendHelpStr((unsigned char*)"  beep <f> <t>", (unsigned char*)"Send beep to robot with frequency and duration\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  get battery", (unsigned char*)"Query robot battery voltage\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  get button", (unsigned char*)"Query robot button status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  nav <udlrc> on|off", (unsigned char*)"Send nav (up, down, left, right, center) button message\r\n", io->stdOut);
#endif
  McuShell_SendHelpStr((unsigned char*)"  saddr <addr>", (unsigned char*)"Set RNet source address\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  daddr <addr>", (unsigned char*)"Set RNet destination address\r\n", io->stdOut);
  return ERR_OK;
}

#if PL_CONFIG_USE_NORDIC_RADIO && McuLib_CONFIG_CPU_IS_ESP32
static uint8_t HandleNavCommand(const unsigned char *cmd) {
  uint32_t buttonBits = 0;
  McuDbnc_EventKinds kind = MCUDBNC_EVENT_PRESSED;

  while(*cmd!='\0' && *cmd!=' ') {
    switch(*cmd) {
      case 'u': buttonBits |= BUTTONS_BIT_NAV_UP; break;
      case 'd': buttonBits |= BUTTONS_BIT_NAV_DOWN; break;
      case 'l': buttonBits |= BUTTONS_BIT_NAV_LEFT; break;
      case 'r': buttonBits |= BUTTONS_BIT_NAV_RIGHT; break;
      case 'c': buttonBits |= BUTTONS_BIT_NAV_CENTER; break;
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
#if McuLib_CONFIG_CPU_IS_ESP32
  RemoteNordic_ESP32OnButtonEvent(buttonBits, kind);
  return ERR_OK;
#else
  /* handle for robot */
#endif
  return ERR_FAILED;
}
#endif

uint8_t RemoteNordic_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;
  int32_t val;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"RemoteNordic help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"RemoteNordic status")==0) {
    *handled = TRUE;
    return PrintStatus(io);
#if PL_CONFIG_IS_ESP32
  } else if (McuUtility_strncmp((char*)cmd, (char*)"RemoteNordic beep ", sizeof("RemoteNordic beep ")-1)==0) {
    uint16_t freq, time;
    uint8_t dataBuf[4]; /* 2 byte frequency, 2 byte duration */
    const unsigned char *p;

    p = cmd + sizeof("RemoteNordic beep ")-1;
    *handled = true;
    if (McuUtility_ScanDecimal16uNumber(&p, &freq)==ERR_OK && McuUtility_ScanDecimal16uNumber(&p, &time)==ERR_OK) {
      McuUtility_SetValue16LE(freq, &dataBuf[0]);
      McuUtility_SetValue16LE(time, &dataBuf[2]);
      return RAPP_SendPayloadDataBlock(dataBuf, sizeof(dataBuf), RAPP_MSG_TYPE_BEEP, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
    } else {
      McuShell_SendStr((unsigned char*)"ERR: wrong format\r\n", io->stdErr);
      return ERR_FAILED;
    }
#endif
#if PL_CONFIG_IS_ESP32
  } else if (McuUtility_strcmp((char*)cmd, (char*)"RemoteNordic get battery")==0) {
    *handled = true;
    return RemoteNordic_QueryRobotBatteryVoltage();
#endif
#if PL_CONFIG_IS_ESP32
  } else if (McuUtility_strcmp((char*)cmd, (char*)"RemoteNordic get button")==0) {
    *handled = true;
    return RemoteNordic_QueryRobotButton();
#endif
  } else if (McuUtility_strncmp((char*)cmd, (char*)"RemoteNordic saddr ", sizeof("RemoteNordic saddr ")-1)==0) {
    *handled = true;
    p = cmd + sizeof("RemoteNordic saddr ")-1;
    if (McuUtility_xatoi(&p, &val)==ERR_OK) {
      RemoteNordic_SetRNetSourceAddr(val);
      return ERR_OK;
    } else {
      return ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"RemoteNordic daddr ", sizeof("RemoteNordic daddr ")-1)==0) {
    *handled = true;
    p = cmd + sizeof("RemoteNordic daddr ")-1;
    if (McuUtility_xatoi(&p, &val)==ERR_OK) {
      RemoteNordic_SetRNetDestinationAddr(val);
      return ERR_OK;
    } else {
      return ERR_FAILED;
    }
#if PL_CONFIG_IS_ESP32
  } else if (McuUtility_strncmp((char*)cmd, (char*)"RemoteNordic nav ", sizeof("RemoteNordic nav ")-1)==0) {
    *handled = true;
    return HandleNavCommand(cmd+sizeof("RemoteNordic nav ")-1);
#endif
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

static void NordicRemoteTask(void *pv) {
#if PL_CONFIG_USE_MININI
  RNWK_ShortAddrType addr;
  addr = McuMinINI_ini_getl(NVMC_MININI_SECTION_NORDIC_REMOTE, NVMC_MININI_KEY_NORDIC_REMOTE_DADDR, NVMC_MININI_KEY_NORDIC_REMOTE_SADDR_DEFAULT, NVMC_MININI_FILE_NAME);
  RNETA_SetSrcAddr(addr);
  addr = McuMinINI_ini_getl(NVMC_MININI_SECTION_NORDIC_REMOTE, NVMC_MININI_KEY_NORDIC_REMOTE_DADDR, NVMC_MININI_KEY_NORDIC_REMOTE_DADDR_DEFAULT, NVMC_MININI_FILE_NAME);
  RNETA_SetDestAddr(addr);
#else
  RNETA_SetSrcAddr(NVMC_MININI_KEY_NORDIC_REMOTE_SADDR_DEFAULT);
  RNETA_SetDestAddr(NVMC_MININI_KEY_NORDIC_REMOTE_DADDR_DEFAULT);
#endif
  for(;;) {
    #if 0 && PL_CONFIG_USE_BUTTONS /* template/example for how to send a button event */
    if (RemoteNordic_ButtonIsPressed()) {
      RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_NOTIFY_VALUE, RAPP_MSG_TYPE_DATA_ID_BUTTON, 1, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE); /* pressed */
      while(RemoteNordic_ButtonIsPressed()) {
        vTaskDelay(pdMS_TO_TICKS(10));
      }
      RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_NOTIFY_VALUE, RAPP_MSG_TYPE_DATA_ID_BUTTON, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE); /* released */
    }
    #endif
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void RemoteNordic_Init(void) {
  if (xTaskCreate(NordicRemoteTask, "NordicRemote", 1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
#if PL_CONFIG_IS_ROBOT
  /* use a timeout on the robot: if after a move command we do not get a stop, we get a timeout and stop automatically */
  timeoutTimer = xTimerCreate(
        "timeout", /* name */
        pdMS_TO_TICKS(NORDIC_REMOTE_TIMEOUT_PERIOD_MS), /* period/time */
        pdFALSE, /* auto reload */
        (void*)0, /* timer ID */
        vTimerCallbacktTimeout); /* callback */
  if (timeoutTimer==NULL) {
    McuLog_fatal("failed creating timer");
    for(;;); /* failure! */
  }
#endif
}

#endif /* PL_CONFIG_USE_REMOTE_NORDIC */
