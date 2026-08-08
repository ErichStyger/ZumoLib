/*
 * Copyright (c) 2019-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * Module which is used to send commands between ESP and robot over the UART interface.
 * This module is used by the robot to receive messages from the ESP and respond to the commands.
 */

#include "platform.h"
#if PL_CONFIG_USE_ROBOT_TO_ESP
#include "robotToEsp.h"
#include "McuRTOS.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "McuESP32.h"
#include "McuShellUart.h"
#if PL_CONFIG_USE_DRIVE
  #include "drive.h"
#endif
#include "shell.h"
#if PL_CONFIG_USE_BUZZER
  #include "buzzer.h"
#endif

static QueueHandle_t RobotToEsp_RxFromESP_Queue; /* queue for data received from the ESP sent to the robot */
static QueueHandle_t RobotToEsp_TxToESP_Queue;   /* queue for the data to be sent as response to the ESP from the robot */

#define ROBOT_TO_ESP_UART_RX_FROM_ESP_QUEUE_LENGTH      32 /* items in queue */
#define ROBOT_TO_ESP_UART_RX_FROM_ESP_QUEUE_ITEM_SIZE   1  /* each item is a single character */
#define ROBOT_TO_ESP_UART_TX_FROM_ESP_QUEUE_LENGTH      32 /* items in queue */
#define ROBOT_TO_ESP_UART_TX_FROM_ESP_QUEUE_ITEM_SIZE   1  /* each item is a single character */

/* called by the gateway task to put a char from the ESP into the queue for the remote */
void RobotToEsp_GatewayRxFromESP(unsigned char ch) {
  if (xQueueSendToBack(RobotToEsp_RxFromESP_Queue, &ch, pdMS_TO_TICKS(10))!=pdPASS) {
    /* was not possible to put it into the queue: will loose data here */
    McuLog_error("failed to store '%c' in queue", ch);
  }
}

/* called by the gateway task to get chars from the remote task to be sent to the ESP32 */
bool RobotToEsp_GatewayTxToESP(unsigned char *pch) {
  portBASE_TYPE res;

  res = xQueueReceive(RobotToEsp_TxToESP_Queue, pch, 0); /* poll queue */
  if (res==errQUEUE_EMPTY) {
    return false;
  } else {
    return true;
  }
}

#if 0 /* code on ESP? */
/* ----------------- buffer handling for shell messages sent to ESP32 */
static unsigned char *esp_io_buf; /* pointer to buffer */
static size_t esp_io_buf_size; /* size of buffer */

static void esp_io_buf_SendChar(unsigned char ch) {
  McuUtility_chcat(esp_io_buf, esp_io_buf_size, ch);
}

static void esp_io_buf_ReadChar(uint8_t *c) {
  *c = '\0';
}

static bool esp_io_buf_CharPresent(void) {
  return false;
}

static McuShell_ConstStdIOType esp_stdio = {
  .stdIn = (McuShell_StdIO_In_FctType)esp_io_buf_ReadChar,
  .stdOut = (McuShell_StdIO_OutErr_FctType)esp_io_buf_SendChar,
  .stdErr = (McuShell_StdIO_OutErr_FctType)esp_io_buf_SendChar,
  .keyPressed = esp_io_buf_CharPresent, /* if input is not empty */
#if McuShell_CONFIG_ECHO_ENABLED
  .echoEnabled = false, /* echo enabled for idf.py monitor */
#endif
};

void RobotToEsp_SendToESPAndGetResponse(const unsigned char *msg, unsigned char *response, size_t responseSize) {
  esp_io_buf = response;
  esp_io_buf_size = responseSize;
  esp_io_buf[0] = '\0'; /* initialize buffer */
  McuLog_info("Sending to ESP Shell: %s", msg);
  McuShell_ParseWithCommandTableExt(msg, &esp_stdio, CmdParserTable, true); /* send to ESP32 shell */
  if (response[0]=='\0') { /* empty response? add a default */
    McuUtility_strcpy(response, responseSize, (unsigned char*)"OK"); /* default response */
  }
}
#endif
/* ----------------------------------------------------------------------*/
/* I/O to handle forwarded characters from the ESP for scanning by the robot for commands */
static void dummyReadChar(uint8_t *c) {
  *c = '\0'; /* nothing received, receive interface not used */
}

static bool dummyCharPreset(void) {
  return false; /* nothing received, receive interface not used */
}

static void RxWriteChar(unsigned char ch) {
  RobotToEsp_GatewayRxFromESP(ch); /* store incoming character into queue. It wil be then consumed by the RemoteToEspTask() task */
}

static int RxWriteData(const void *data, size_t size) {
  for(int i=0; i<size; i++) {
    RxWriteChar(((char*)data)[i]);
  }
  return size;
}

/* IO to receive data stream from ESP, to scan it for commands and writing back the response.
   Only the writing (stdout, writeData) is used by the upper layer */
McuShell_ConstStdIOType robotParsingESPcommands = {
  .stdIn = (McuShell_StdIO_In_FctType)dummyReadChar,  /* read characters sent by the ESP */
  .stdOut = (McuShell_StdIO_OutErr_FctType)RxWriteChar, /* send character to the ESP */
  .stdErr = (McuShell_StdIO_OutErr_FctType)RxWriteChar, /* send character to the ESP*/
  .keyPressed = dummyCharPreset, /* if input from the ESP is not empty */
#if McuShell_CONFIG_ECHO_ENABLED
  .echoEnabled = false,
#endif
#if McuShell_CONFIG_HAS_WRITE_DATA
  .writeData = RxWriteData,
#endif
};

/* this I/O is used to receive data from the ESP which the robot scans for commands */
McuShell_ConstStdIOTypePtr RobotToEsp_GetIOforEspRx(void) {
  return &robotParsingESPcommands; /* caller will use .stdout or .writedate to send us the data from the ESP */
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
        #if PL_CONFIG_USE_BUZZER
          Buzzer_Beep(200, 100);
        #endif
        } else if (McuUtility_strcmp((char*)buf, (char*)"beep")==0) {
          McuLog_trace("received beep");
        #if PL_CONFIG_USE_BUZZER
          Buzzer_Beep(200, 500);
        #endif
        } else if (McuUtility_strcmp((char*)buf, (char*)"fw")==0) {
          McuLog_trace("received fw");
        #if PL_CONFIG_USE_BUZZER
          Buzzer_Beep(200, 500);
        #endif
        } else if (McuUtility_strncmp((char*)buf, (char*)"cmd ", sizeof("cmd ")-1)==0) { /* handle by the shell command parser */
          const unsigned char *p;

          p = buf+sizeof("cmd ")-1;
          McuLog_trace("received cmd \"%s\"", p);
          Shell_ParseCommandWithIO((unsigned char*)p, McuESP32_GetTxToESPStdio()); /* parse command and end answer to ESP */
        }
        *state = CMD_PARSER_SCANNING; /* start scanning again */
        break;

      default:
        break;
    } /* switch */
    /* get here with a break */
  } /* for */
}

static void RemoteToEspTask(void *pv) {
  unsigned char buf[64];
  static CMD_ParserState_e state = CMD_PARSER_INIT;
  unsigned char ch;
  BaseType_t res;

  (void)pv; /* not used */
#if 0 && PL_CONFIG_USE_BUZZER /* example making a beep */
  Buzzer_Beep(200, 500);
#endif
#if 0 && PL_CONFIG_USE_DRIVE /* example driving the robot */
  DRV_SetMode(DRV_MODE_SPEED);
  DRV_SetSpeed(500, 500);
  vTaskDelay(pdMS_TO_TICKS(1000));
  DRV_SetMode(DRV_MODE_STOP);
#endif
#if 0 /* example receiving character from queue */
  res = xQueueReceive(RobotToEsp_RxFromESP_Queue, &ch, pdMS_TO_TICKS(10)); /* wait max 10 ms */
  if (res==errQUEUE_EMPTY) {
    ch = '\0'; /* nothing received */
  }
  res = xQueueReceive(RobotToEsp_RxFromESP_Queue, &ch, portMAX_DELAY); /* wait 'forever' */
  if (res==errQUEUE_EMPTY) {
    ch = '\0'; /* nothing received */
  }
#endif
  for(;;) {
    res = xQueueReceive(RobotToEsp_RxFromESP_Queue, &ch, portMAX_DELAY); /* wait 'forever' */
    if (res==errQUEUE_EMPTY) {
      /* queue timeout */
    } else {
      Scan(&state, ch, buf, sizeof(buf)); /* scan received text and scan for command received */
    }
  }
}

uint8_t RobotToEsp_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  (void)cmd;
  (void)handled;
  (void)io;
  /* nothing implemented yet */
  return ERR_OK;
}

void RobotToEsp_Deinit(void) {
  /* nothing needed */
}

void RobotToEsp_Init(void) {
  if (xTaskCreate(RemoteToEspTask, "Remote2Esp", 1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
  RobotToEsp_RxFromESP_Queue = xQueueCreate(ROBOT_TO_ESP_UART_RX_FROM_ESP_QUEUE_LENGTH, ROBOT_TO_ESP_UART_RX_FROM_ESP_QUEUE_ITEM_SIZE);
  if (RobotToEsp_RxFromESP_Queue==NULL) {
    for(;;){} /* out of memory? */
  }
  vQueueAddToRegistry(RobotToEsp_RxFromESP_Queue, "RemoteRxFromESPQueue");
  RobotToEsp_TxToESP_Queue = xQueueCreate(ROBOT_TO_ESP_UART_TX_FROM_ESP_QUEUE_LENGTH, ROBOT_TO_ESP_UART_TX_FROM_ESP_QUEUE_ITEM_SIZE);
  if (RobotToEsp_TxToESP_Queue==NULL) {
    for(;;){} /* out of memory? */
  }
  vQueueAddToRegistry(RobotToEsp_TxToESP_Queue, "RemoteTxToESPQueue");
}

#endif /* PL_CONFIG_USE_ROBOT_TO_ESP */
