/*
 * Copyright (c) 2019-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * Module which is used to send commands between ESP and robot over the UART interface.
 * This module is used by the robot to receive messages from the ESP and respond to the commands.
 */

#include "platform.h"
#if PL_CONFIG_USE_ROBOT2ESP
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

static QueueHandle_t RobotToEsp_RxFromESP_Queue; /* queue for data received from the ESP, to be read by the robot */
static QueueHandle_t RobotToEsp_TxToESP_Queue;   /* queue for the data to be sent from the robot as response for the ESP commands */

#define ROBOT_TO_ESP_UART_RX_FROM_ESP_QUEUE_LENGTH      64 /* items in queue */
#define ROBOT_TO_ESP_UART_RX_FROM_ESP_QUEUE_ITEM_SIZE   1  /* each item is a single character */
#define ROBOT_TO_ESP_UART_TX_FROM_ESP_QUEUE_LENGTH      64 /* items in queue */
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
          Shell_ParseCommandWithIO((unsigned char*)p, McuESP32_GetTxToESPStdio()); /* parse command and send answer to ESP */
        }
        *state = CMD_PARSER_SCANNING; /* start scanning again */
        break;

      default:
        break;
    } /* switch */
    /* get here with a break */
  } /* for */
}

static void RobotToEspTask(void *pv) {
  unsigned char buf[64];
  static CMD_ParserState_e state = CMD_PARSER_INIT;
  unsigned char ch;
  BaseType_t res;

  (void)pv; /* not used */
  for(;;) {
    res = xQueueReceive(RobotToEsp_RxFromESP_Queue, &ch, portMAX_DELAY); /* wait 'forever' */
    if (res==errQUEUE_EMPTY) {
      /* queue timeout */
    } else {
      Scan(&state, ch, buf, sizeof(buf)); /* scan received text and scan for command received */
    }
  }
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"robot2esp", (unsigned char*)"Robot2Esp status\r\n", io->stdOut);  return ERR_OK;
}

static void PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"robot2esp", (unsigned char*)"Group of robot2esp commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  send <text>", (unsigned char*)"Send text or command to the ESP\r\n", io->stdOut);
}

uint8_t RobotToEsp_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"robot2esp help")==0) {
    *handled = true;
    PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"robot2esp status")==0) {
    *handled = true;
    return PrintStatus(io);
  } else if (McuUtility_strncmp((char*)cmd, (char*)"robot2esp send ", sizeof("robot2esp send ")-1)==0) {
    *handled = true;
    p = cmd + sizeof("robot2esp send ")-1;
    /* use the shell UART for the command output and data received from the ESP */
    McuESP32_SetRxFromESPStdio(McuShellUart_GetStdio()); /* assign optional I/O for incoming ESP data: forward to shell UART */
    while(*p!='\0') {
      McuESP32_GetTxToESPStdio()->stdOut(*p);
      p++;
    }
    McuESP32_GetTxToESPStdio()->stdOut('\r');
    McuESP32_GetTxToESPStdio()->stdOut('\n');
    vTaskDelay(pdMS_TO_TICKS(500)); /* give some time prior switching back to the parser */
    McuESP32_SetRxFromESPStdio(&robotParsingESPcommands); /* assign optional I/O for incoming ESP data: forward it to the parser in this module */
    return ERR_OK;
  }
  return ERR_OK;
}

void RobotToEsp_Deinit(void) {
  /* nothing needed */
}

void RobotToEsp_Init(void) {
  if (xTaskCreate(RobotToEspTask, "Robot2Esp", 1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
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

  McuESP32_SetRxFromESPStdio(&robotParsingESPcommands); /* assign optional I/O for incoming ESP data: forward it to the parser in this module */
}

#endif /* PL_CONFIG_USE_ROBOT2ESP */
