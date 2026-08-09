/**
 * \file
 * \brief Module to handle a LED with RNet
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_REMOTE_RNET_LED
#include "remoteRnetLED.h"
#include "RNet_App.h"
#include "McuShell.h"
#include "McuLog.h"
#include "McuUtility.h"
#include "leds.h"

static bool LedIsOn(void) {
#if McuLib_CONFIG_CPU_IS_ESP32
  return Leds_Get(LEDS_RED);
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

uint8_t RemoteRnetLED_HandleRemoteRxMessage(RAPP_MSG_Type type, uint8_t size, uint8_t *data, RNWK_ShortAddrType srcAddr, bool *handled, RPHY_PacketDesc *packet) {
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
  McuShell_SendStatusStr((unsigned char*)"remoteled", (unsigned char*)"Remote RNet LED status\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  local LED", LedIsOn()?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"remoteled", (unsigned char*)"Group of remote RNet LED commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  get led", (unsigned char*)"Query remote LED state with RNet\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  set led on|off", (unsigned char*)"Set remot LED with RNet\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t RemoteRnetLED_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"remoteled help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"remoteled status")==0) {
    *handled = TRUE;
    return PrintStatus(io);
    return RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE, RAPP_MSG_TYPE_DATA_ID_BUTTON, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"remoteled get led")==0) {
    *handled = true;
    return RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_QUERY_VALUE, RAPP_MSG_TYPE_DATA_ID_LED, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"remoteled set led on")==0) {
    *handled = true;
    return RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_REQUEST_SET_VALUE, RAPP_MSG_TYPE_DATA_ID_LED, 1, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"remoteled set led off")==0) {
    *handled = true;
    return RAPP_SendIdValuePairMessage(RAPP_MSG_TYPE_REQUEST_SET_VALUE, RAPP_MSG_TYPE_DATA_ID_LED, 0, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
  }
  return ERR_OK;
}

void RemoteRnetLED_Init(void) {
}

#endif /* PL_CONFIG_USE_REMOTE_RNET_LED */
