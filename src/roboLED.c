/**
 * \file
 * \brief Module to handle a LED with RNet
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module implements the catch-the-flag extensions.
 */

#include "platform.h"
#if PL_CONFIG_USE_REMOTE_ROBO_LED
#include "roboLED.h"
#include "RNet_App.h"
#include "McuShell.h"
#include "McuLog.h"
#include "McuUtility.h"
#include "leds.h"

static bool LedIsOn(void) {
#if McuLib_CONFIG_CPU_IS_ESP32
  return Leds_Get(LEDS_CONFIG_HAS_RED_LED);
#else /* robot */
  return Leds_Get(LEDS_LEFT_RED);
#endif
}

static void SetLed(bool on) {
#if McuLib_CONFIG_CPU_IS_ESP32
  if (on) {
    Leds_On(LEDS_RED);
  } else {
    Leds_Off(LEDS_RED);
  }
#else /* robot */
  if (on) {
    Leds_On(LEDS_LEFT_RED);
  } else {
    Leds_Off(LEDS_LEFT_RED);
  }
#endif
}

uint8_t RoboLED_HandleRemoteRxMessage(RAPP_MSG_Type type, uint8_t size, uint8_t *data, RNWK_ShortAddrType srcAddr, bool *handled, RPHY_PacketDesc *packet) {
  RAPP_MSG_DataIDType msgID;
  uint32_t msgValue;
  uint16_t value16;

  switch(type) {
    /* ------------ Received a request to set a value -------------------*/
    case RAPP_MSG_TYPE_REQUEST_SET_VALUE:
      msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
      switch(msgID) {
        case RAPP_MSG_TYPE_DATA_ID_LED:
          *handled =true;
          value16 = McuUtility_GetValue16LE(&data[2]);
          SetLed(value16);
          break;
        default:
          break;
      } /* switch */
      break;
    /* ------------ Received a Query -> send back response -------------------*/
    case RAPP_MSG_TYPE_QUERY_VALUE:
      msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
      switch(msgID) {
        case RAPP_MSG_TYPE_DATA_ID_LED:
          *handled =true;
          msgValue = LedIsOn();
          RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE, msgID, msgValue, srcAddr, RPHY_PACKET_FLAGS_NONE);
          break;
       default:
          break;
      }
      break;
    /* ------------ Received the response to a a Query -------------------*/
    case RAPP_MSG_TYPE_QUERY_VALUE_RESPONSE: /* received data value for request */
      msgID = McuUtility_GetValue16LE(&data[0]); /* ID in little endian format */
      msgValue = McuUtility_GetValue32LE(&data[2]);
      switch(msgID) {
        case RAPP_MSG_TYPE_DATA_ID_LED:
          *handled = true;
          McuLog_info("Rx: LED is %s", msgValue==0?"off":"on");
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
          case RAPP_MSG_TYPE_DATA_ID_LED:
            *handled = true;
            McuLog_info("Notify: LED is %s", msgValue==0?"off":"on");
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

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"roboled", (unsigned char*)"Group of remote LED commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  get led", (unsigned char*)"Query LED state\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  set led on|off", (unsigned char*)"Set LED state\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t RoboLED_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"roboled help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"roboled status")==0) {
    *handled = TRUE;
    return PrintStatus(io);
    return RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE, RAPP_MSG_TYPE_DATA_ID_BUTTON, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"roboled get led")==0) {
    *handled = true;
    return RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE, RAPP_MSG_TYPE_DATA_ID_LED, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"roboled set led on")==0) {
    *handled = true;
    return RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_REQUEST_SET_VALUE, RAPP_MSG_TYPE_DATA_ID_LED, 1, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"roboled set led off")==0) {
    *handled = true;
    return RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_REQUEST_SET_VALUE, RAPP_MSG_TYPE_DATA_ID_LED, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  }
  return ERR_OK;
}

void RobotLED_Init(void) {
}

#endif /* PL_CONFIG_USE_REMOTE_ROBO_LED */
