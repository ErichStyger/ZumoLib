/*
 * Copyright (c) 2019-2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "remoteESPUart.h"
#if PL_CONFIG_USE_REMOTE
#include "McuRTOS.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "drive.h"
#include "shell.h"
#include "buzzer.h"
#include "McuESP32.h"

static QueueHandle_t REMOTE_RxFromESP_Queue;
static QueueHandle_t REMOTE_TxToESP_Queue;

#define REMOTE_RX_FROM_ESP_QUEUE_LENGTH      32 /* items in queue */
#define REMOTE_RX_FROM_ESP_QUEUE_ITEM_SIZE   1  /* each item is a single character */
#define REMOTE_TX_FROM_ESP_QUEUE_LENGTH      32 /* items in queue */
#define REMOTE_TX_FROM_ESP_QUEUE_ITEM_SIZE   1  /* each item is a single character */

/* called by the gateway task to put a char from the ESP into the queue for the remote */
void REMOTE_GatewayRxFromESP(unsigned char ch) {
  if (xQueueSendToBack(REMOTE_RxFromESP_Queue, &ch, pdMS_TO_TICKS(10))!=pdPASS) {
    /* was not possible to put it into the queue: will loose data here */
  }
}

/* called by the gateway task to get chars from the remote task to be sent to the ESP32 */
bool REMOTE_GatewayTxToESP(unsigned char *pch) {
  portBASE_TYPE res;

  res = xQueueReceive(REMOTE_TxToESP_Queue, pch, 0); /* poll queue */
  if (res==errQUEUE_EMPTY) {
    return false;
  } else {
    return true;
  }
}

typedef enum {
  CMD_PARSER_INIT,
  CMD_PARSER_START_DETECTED, /* start detected */
  CMD_PARSER_SCANNING,  /* scan for start */
  CMD_PARSER_SCANNIGN_CMD,   /* reading command */
  CMD_PARSER_CMD_FINSISHED,   /* command end detected */
} CMD_ParserState_e;

static void Scan(CMD_ParserState_e *state, unsigned char ch, unsigned char *buf, size_t bufSize) {
  /* command starts with @robot: and ends with !, format is like this:
   @robot:cmd buzzer buz 200 500!
   */
  if (ch=='@') { /* a marker? start scanning again */
    *state = CMD_PARSER_START_DETECTED;
  }
  for(;;) { /* breaks or returns */
    switch(*state) {
      case CMD_PARSER_INIT:
        if (ch=='@') { /* a marker? start scanning again */
          *state = CMD_PARSER_START_DETECTED;
          break; /* continue state machine */
        }
        return;

       case CMD_PARSER_START_DETECTED:
        buf[0] = '\0'; /* init buffer */
        *state = CMD_PARSER_SCANNING;
        break; /* continue state machine */

       case CMD_PARSER_SCANNING:
          McuUtility_chcat(buf,bufSize, ch);
          if (McuUtility_strncmp((char*)buf, (char*)"@robot:", sizeof("@robot:")-1)==0) { /* a match! */
            buf[0] = '\0'; /* init buffer */
            *state = CMD_PARSER_SCANNIGN_CMD;
          }
          return;

      case CMD_PARSER_SCANNIGN_CMD:
        if (ch=='!') {
          *state = CMD_PARSER_CMD_FINSISHED;
          break; /* continue state machine */
        }
        McuUtility_chcat(buf, bufSize, ch);
        return;

      case CMD_PARSER_CMD_FINSISHED:
        if (McuUtility_strcmp((char*)buf, (char*)"stop")==0) {
          McuLog_trace("received stop");
          Buzzer_Beep(200, 100);
        } else if (McuUtility_strcmp((char*)buf, (char*)"beep")==0) {
          McuLog_trace("received beep");
          Buzzer_Beep(200, 500);
        } else if (McuUtility_strcmp((char*)buf, (char*)"fw")==0) {
          McuLog_trace("received fw");
          Buzzer_Beep(200, 500);
        } else if (McuUtility_strncmp((char*)buf, (char*)"cmd ", sizeof("cmd ")-1)==0) {
          const unsigned char *p;

          p = buf+sizeof("cmd ")-1;
          McuLog_trace("received cmd \"%s\"", p);
          Shell_ParseCommandWithIO((unsigned char*)p, McuESP32_GetTxToESPStdio());
        }
        *state = CMD_PARSER_SCANNING; /* start scanning again */
        break;

      default:
        break;
    } /* switch */
    /* get here with a break */
  } /* for */
}

static void RemoteTask(void *pv) {
  unsigned char buf[64];
  static CMD_ParserState_e state = CMD_PARSER_INIT;
  unsigned char ch;
  BaseType_t res;

  (void)pv; /* not used */
#if 0 /* example making a beep */
  BUZ_Beep(200, 500);
#endif
#if 0 /* example driving the robot */
  DRV_SetMode(DRV_MODE_SPEED);
  DRV_SetSpeed(500, 500);
  vTaskDelay(pdMS_TO_TICKS(1000));
  DRV_SetMode(DRV_MODE_STOP);
#endif
#if 0 /* example receiving character from queue */
  res = xQueueReceive(REMOTE_RxFromESP_Queue, &ch, pdMS_TO_TICKS(10)); /* wait max 10 ms */
  if (res==errQUEUE_EMPTY) {
    ch = '\0'; /* nothing received */
  }
  res = xQueueReceive(REMOTE_RxFromESP_Queue, &ch, portMAX_DELAY); /* wait 'forever' */
  if (res==errQUEUE_EMPTY) {
    ch = '\0'; /* nothing received */
  }
#endif
  for(;;) {
    res = xQueueReceive(REMOTE_RxFromESP_Queue, &ch, portMAX_DELAY); /* wait 'forever' */
    if (res==errQUEUE_EMPTY) {
      /* queue timeout */
    } else {
      Scan(&state, ch, buf, sizeof(buf));
    }
  }
}

uint8_t REMOTE_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  (void)cmd;
  (void)handled;
  (void)io;
  return ERR_OK;
}

void REMOTE_Deinit(void) {
  /* nothing needed */
}

void REMOTE_Init(void) {
  if (xTaskCreate(RemoteTask, "Remote", 1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
  REMOTE_RxFromESP_Queue = xQueueCreate(REMOTE_RX_FROM_ESP_QUEUE_LENGTH, REMOTE_RX_FROM_ESP_QUEUE_ITEM_SIZE);
  if (REMOTE_RxFromESP_Queue==NULL) {
    for(;;){} /* out of memory? */
  }
  vQueueAddToRegistry(REMOTE_RxFromESP_Queue, "RemoteRxFromESPQueue");
  REMOTE_TxToESP_Queue = xQueueCreate(REMOTE_TX_FROM_ESP_QUEUE_LENGTH, REMOTE_TX_FROM_ESP_QUEUE_ITEM_SIZE);
  if (REMOTE_TxToESP_Queue==NULL) {
    for(;;){} /* out of memory? */
  }
  vQueueAddToRegistry(REMOTE_TxToESP_Queue, "RemoteTxToESPQueue");
}

#endif /* PL_CONFIG_USE_REMOTE */
