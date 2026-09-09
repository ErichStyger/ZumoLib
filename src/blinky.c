/*
 * Copyright (c) 2020-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_BLINKY
#include "blinky.h"
#include "McuRTOS.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "leds.h"
#if PL_CONFIG_USE_NEO_PIXEL_HW
  #include "NeoPixel.h"
#endif

static TaskHandle_t taskHandle;
static bool blinkyIsRunning = false;

#define LED_OFF_TIME_MS    10
static uint32_t onTimeMs = 500; /* default on time of LED, used for network status indication */

void Blinky_SetOnTime(uint32_t ms) {
  onTimeMs = ms;
}

void Blinky_On(void) {
#if LEDS_CONFIG_HAS_RED_LED
  Leds_On(LEDS_RED);
#elif LEDS_CONFIG_HAS_LEFT_RED_LED
  Leds_On(LEDS_LEFT_RED);
#endif
}

void Blinky_Off(void) {
#if LEDS_CONFIG_HAS_RED_LED
  Leds_Off(LEDS_RED);
#elif LEDS_CONFIG_HAS_LEFT_RED_LED
  Leds_Off(LEDS_LEFT_RED);
#endif
}

void Blinky_Suspend(void) {
  if (taskHandle!=NULL) {
    vTaskSuspend(taskHandle);
    blinkyIsRunning = false;
    Blinky_Off();
  }
}

void Blinky_Resume(void) {
  if (taskHandle!=NULL) {
    vTaskResume(taskHandle);
    blinkyIsRunning = true;
  }
}

void Blinky_GetStatus(unsigned char *buf, size_t bufSize) {
  buf[0] = '\0';
  if (taskHandle!=NULL) {
    eTaskState state;

    state = eTaskGetState(taskHandle);
    switch(state) {
    case eSuspended:
      McuUtility_strcpy(buf, bufSize, (unsigned char*)"suspended");
      break;
    case eRunning:
    case eBlocked:
      McuUtility_strcpy(buf, bufSize, (unsigned char*)"running");
      break;
    default:
    case eDeleted:
      McuUtility_strcpy(buf, bufSize, (unsigned char*)"ERROR!");
      break;
    }
  } else {
    McuUtility_strcpy(buf, bufSize, (unsigned char*)"ERROR: no task!");
  }
}

static const uint32_t rainbowTable[256] = {
  0xFF0000u, 0xFC0300u, 0xF90600u, 0xF60900u, 0xF30C00u, 0xF00F00u, 0xED1200u, 0xEA1500u, 0xE71800u, 0xE41B00u, 0xE11E00u, 0xDE2100u, 0xDB2400u, 0xD82700u, 0xD52A00u, 0xD22D00u,
  0xCF3000u, 0xCC3300u, 0xC93600u, 0xC63900u, 0xC33C00u, 0xC03F00u, 0xBD4200u, 0xBA4500u, 0xB74800u, 0xB44B00u, 0xB14E00u, 0xAE5100u, 0xAB5400u, 0xA85700u, 0xA55A00u, 0xA25D00u,
  0x9F6000u, 0x9C6300u, 0x996600u, 0x966900u, 0x936C00u, 0x906F00u, 0x8D7200u, 0x8A7500u, 0x877800u, 0x847B00u, 0x817E00u, 0x7E8100u, 0x7B8400u, 0x788700u, 0x758A00u, 0x728D00u,
  0x6F9000u, 0x6C9300u, 0x699600u, 0x669900u, 0x639C00u, 0x609F00u, 0x5DA200u, 0x5AA500u, 0x57A800u, 0x54AB00u, 0x51AE00u, 0x4EB100u, 0x4BB400u, 0x48B700u, 0x45BA00u, 0x42BD00u,
  0x3FC000u, 0x3CC300u, 0x39C600u, 0x36C900u, 0x33CC00u, 0x30CF00u, 0x2DD200u, 0x2AD500u, 0x27D800u, 0x24DB00u, 0x21DE00u, 0x1EE100u, 0x1BE400u, 0x18E700u, 0x15EA00u, 0x12ED00u,
  0x0FF000u, 0x0CF300u, 0x09F600u, 0x06F900u, 0x03FC00u, 0x00FF00u, 0x00FC03u, 0x00F906u, 0x00F609u, 0x00F30Cu, 0x00F00Fu, 0x00ED12u, 0x00EA15u, 0x00E718u, 0x00E41Bu, 0x00E11Eu,
  0x00DE21u, 0x00DB24u, 0x00D827u, 0x00D52Au, 0x00D22Du, 0x00CF30u, 0x00CC33u, 0x00C936u, 0x00C639u, 0x00C33Cu, 0x00C03Fu, 0x00BD42u, 0x00BA45u, 0x00B748u, 0x00B44Bu, 0x00B14Eu,
  0x00AE51u, 0x00AB54u, 0x00A857u, 0x00A55Au, 0x00A25Du, 0x009F60u, 0x009C63u, 0x009966u, 0x009669u, 0x00936Cu, 0x00906Fu, 0x008D72u, 0x008A75u, 0x008778u, 0x00847Bu, 0x00817Eu,
  0x007E81u, 0x007B84u, 0x007887u, 0x00758Au, 0x00728Du, 0x006F90u, 0x006C93u, 0x006996u, 0x006699u, 0x00639Cu, 0x00609Fu, 0x005DA2u, 0x005AA5u, 0x0057A8u, 0x0054ABu, 0x0051AEu,
  0x004EB1u, 0x004BB4u, 0x0048B7u, 0x0045BAu, 0x0042BDu, 0x003FC0u, 0x003CC3u, 0x0039C6u, 0x0036C9u, 0x0033CCu, 0x0030CFu, 0x002DD2u, 0x002AD5u, 0x0027D8u, 0x0024DBu, 0x0021DEu,
  0x001EE1u, 0x001BE4u, 0x0018E7u, 0x0015EAu, 0x0012EDu, 0x000FF0u, 0x000CF3u, 0x0009F6u, 0x0006F9u, 0x0003FCu, 0x0000FFu, 0x0300FCu, 0x0600F9u, 0x0900F6u, 0x0C00F3u, 0x0F00F0u,
  0x1200EDu, 0x1500EAu, 0x1800E7u, 0x1B00E4u, 0x1E00E1u, 0x2100DEu, 0x2400DBu, 0x2700D8u, 0x2A00D5u, 0x2D00D2u, 0x3000CFu, 0x3300CCu, 0x3600C9u, 0x3900C6u, 0x3C00C3u, 0x3F00C0u,
  0x4200BDu, 0x4500BAu, 0x4800B7u, 0x4B00B4u, 0x4E00B1u, 0x5100AEu, 0x5400ABu, 0x5700A8u, 0x5A00A5u, 0x5D00A2u, 0x60009Fu, 0x63009Cu, 0x660099u, 0x690096u, 0x6C0093u, 0x6F0090u,
  0x72008Du, 0x75008Au, 0x780087u, 0x7B0084u, 0x7E0081u, 0x81007Eu, 0x84007Bu, 0x870078u, 0x8A0075u, 0x8D0072u, 0x90006Fu, 0x93006Cu, 0x960069u, 0x990066u, 0x9C0063u, 0x9F0060u,
  0xA2005Du, 0xA5005Au, 0xA80057u, 0xAB0054u, 0xAE0051u, 0xB1004Eu, 0xB4004Bu, 0xB70048u, 0xBA0045u, 0xBD0042u, 0xC0003Fu, 0xC3003Cu, 0xC60039u, 0xC90036u, 0xCC0033u, 0xCF0030u,
  0xD2002Du, 0xD5002Au, 0xD80027u, 0xDB0024u, 0xDE0021u, 0xE1001Eu, 0xE4001Bu, 0xE70018u, 0xEA0015u, 0xED0012u, 0xF0000Fu, 0xF3000Cu, 0xF60009u, 0xF90006u, 0xFC0003u, 0xFF0000u
};

static void blinkyTask(void *pv) {
  (void)pv;
  blinkyIsRunning = true;
  McuLog_info("started blinky task");
  for(;;) {
  #if LEDS_CONFIG_HAS_LEFT_RED_LED
    Leds_On(LEDS_LEFT_RED);
    vTaskDelay(pdMS_TO_TICKS(onTimeMs));
    Leds_Off(LEDS_LEFT_RED);
  #endif
  #if LEDS_CONFIG_HAS_RIGHT_RED_LED
    Leds_On(LEDS_RIGHT_RED);
    vTaskDelay(pdMS_TO_TICKS(onTimeMs));
    Leds_Off(LEDS_RIGHT_RED);
  #endif
  #if LEDS_CONFIG_HAS_RED_LED
    Leds_On(LEDS_RED);
    vTaskDelay(pdMS_TO_TICKS(onTimeMs));
    Leds_Off(LEDS_RED);
  #endif
  #if LEDS_CONFIG_HAS_GREEN_LED
    Leds_On(LEDS_GREEN);
    vTaskDelay(pdMS_TO_TICKS(onTimeMs));
    Leds_Off(LEDS_GREEN);
  #endif
  #if LEDS_CONFIG_HAS_BLUE_LED
    Leds_On(LEDS_BLUE);
    vTaskDelay(pdMS_TO_TICKS(onTimeMs));
    Leds_Off(LEDS_BLUE);
  #endif
  #if LEDS_CONFIG_HAS_ONBOARD_LED
    Leds_On(LEDS_ONBOARD);
    vTaskDelay(pdMS_TO_TICKS(onTimeMs));
    Leds_Off(LEDS_ONBOARD);
  #endif
  #if PL_CONFIG_USE_NEO_PIXEL_HW
  #if 0
    static uint32_t color = 0xff0000; /* red */
    NEO_ClearAllPixel();
    NEO_SetAllPixelColor(color);
    NEO_TransferPixels();
    if (color==0xff0000) {
      color = 0x00ff00; /* green */
    } else if (color==0x00ff00) {
      color = 0x0000ff; /* blue */
    } else {
      color = 0xff0000; /* red */
    }
  #else
    static int tableIdx = 0;
    int pos = 0;
    for(int i=0; i<NEOC_NOF_LEDS_FIRST; i++) {
      NEO_SetPixelColor(NEOC_LANE_FIRST, pos++, rainbowTable[tableIdx++]);
      if (tableIdx==sizeof(rainbowTable)/sizeof(rainbowTable[0])) {
        tableIdx = 0;
      }
    }
    for(int i=0; i<NEOC_NOF_LEDS_SECOND; i++) {
      NEO_SetPixelColor(NEOC_LANE_SECOND, pos++, rainbowTable[tableIdx++]);
      if (tableIdx==sizeof(rainbowTable)/sizeof(rainbowTable[0])) {
        tableIdx = 0;
      }
    }
    NEO_TransferPixels();
  #endif
  #endif
    vTaskDelay(pdMS_TO_TICKS(LED_OFF_TIME_MS));
  }
}

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"blinky", (unsigned char*)"Blinky status\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  status", blinkyIsRunning?(unsigned char*)"resumed\r\n":(unsigned char*)"suspended\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"blinky", (unsigned char*)"Group of blinky commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Show help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  suspend", (unsigned char*)"Suspend the blinky task\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  resume", (unsigned char*)"Resume the blinky task\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t Blinky_ParseCommand(const unsigned char* cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"blinky help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"blinky status")==0) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"blinky suspend")==0) {
    *handled = TRUE;
    Blinky_Suspend();
  } else if (McuUtility_strcmp((char*)cmd, (char*)"blinky resume")==0) {
    *handled = TRUE;
    Blinky_Resume();
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

void Blinky_Deinit(void) {
}

void Blinky_Init(void) {
  BaseType_t res;
  res = xTaskCreate(blinkyTask, "blinkyTask", 4*1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY, &taskHandle);
  if (res!=pdPASS) {
    McuLog_fatal("failed creating blinky!");
  }
}

#endif /* PL_CONFIG_USE_BLINKY */
