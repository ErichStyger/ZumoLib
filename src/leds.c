/*
 * Copyright (c) 2023-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_LEDS
#if PL_CONFIG_USE_PICO_W
  #include "pico/cyw43_arch.h"
  #include "McuPicoWiFi.h"
#endif
#include "McuLib.h"
#include "leds_config.h"
#include "leds.h"
#include "McuLED.h"
#include "McuUtility.h"
#include "McuLog.h"

static McuLED_Handle_t leds[LEDS_NOF_LEDS];

#if LEDS_CONFIG_HAS_ONBOARD_LED && PL_CONFIG_USE_PICO_W
  static bool onBoardLedIsOn = false; /* need to remember state */
#endif

void Leds_On(LEDS_Leds_e led) {
#if LEDS_CONFIG_HAS_ONBOARD_LED && PL_CONFIG_USE_PICO_W
  if (led==LEDS_ONBOARD) {
    onBoardLedIsOn = true;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, onBoardLedIsOn);
    return;
  }
#endif
  McuLED_On(leds[led]);
}

void Leds_Off(LEDS_Leds_e led) {
#if LEDS_CONFIG_HAS_ONBOARD_LED && PL_CONFIG_USE_PICO_W
  if (led==LEDS_ONBOARD) {
    onBoardLedIsOn = false;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, onBoardLedIsOn);
    return;
  }
#endif
  McuLED_Off(leds[led]);
}

void Leds_Neg(LEDS_Leds_e led) {
#if LEDS_CONFIG_HAS_ONBOARD_LED && PL_CONFIG_USE_PICO_W
  if (led==LEDS_ONBOARD) {
    onBoardLedIsOn = !onBoardLedIsOn;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, onBoardLedIsOn);
    return;
  }
#endif
  McuLED_Toggle(leds[led]);
}

bool Leds_Get(LEDS_Leds_e led) {
#if LEDS_CONFIG_HAS_ONBOARD_LED && PL_CONFIG_USE_PICO_W
  if (led==LEDS_ONBOARD) {
    return onBoardLedIsOn;
  }
#endif
  return McuLED_Get(leds[led]);
}

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(McuShell_ConstStdIOType *io) {
  uint8_t buf[16];

  McuShell_SendStatusStr((const unsigned char*)"led", (const unsigned char*)"LED module status\r\n", io->stdOut);
#if LEDS_CONFIG_HAS_LEFT_RED_LED
  if (Leds_Get(LEDS_LEFT_RED)) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is ON\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is OFF\r\n");
  }
  McuShell_SendStatusStr((const unsigned char*)"  left red", (const unsigned char*)buf, io->stdOut);
#endif
#if LEDS_CONFIG_HAS_RIGHT_RED_LED
  if (Leds_Get(LEDS_RIGHT_RED)) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is ON\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is OFF\r\n");
  }
  McuShell_SendStatusStr((const unsigned char*)"  right red", (const unsigned char*)buf, io->stdOut);
#endif
#if LEDS_CONFIG_HAS_RED_LED
  if (Leds_Get(LEDS_RED)) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is ON\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is OFF\r\n");
  }
  McuShell_SendStatusStr((const unsigned char*)"  red", (const unsigned char*)buf, io->stdOut);
#endif
#if LEDS_CONFIG_HAS_GREEN_LED
  if (Leds_Get(LEDS_GREEN)) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is ON\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is OFF\r\n");
  }
  McuShell_SendStatusStr((const unsigned char*)"  green", (const unsigned char*)buf, io->stdOut);
#endif
#if LEDS_CONFIG_HAS_BLUE_LED
  if (Leds_Get(LEDS_BLUE)) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is ON\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is OFF\r\n");
  }
  McuShell_SendStatusStr((const unsigned char*)"  blue", (const unsigned char*)buf, io->stdOut);
#endif
#if LEDS_CONFIG_HAS_ORANGE_LED
  if (Leds_Get(LEDS_ORANGE)) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is ON\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is OFF\r\n");
  }
  McuShell_SendStatusStr((const unsigned char*)"  orange", (const unsigned char*)buf, io->stdOut);
#endif
#if LEDS_CONFIG_HAS_ONBOARD_LED
  if (Leds_Get(LEDS_ONBOARD)) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is ON\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"is OFF\r\n");
  }
  McuShell_SendStatusStr((const unsigned char*)"  onboard", (const unsigned char*)buf, io->stdOut);
#endif
  return ERR_OK;
}

static uint8_t PrintHelp(McuShell_ConstStdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"led", (const unsigned char*)"Group of LED commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (const unsigned char*)"Print help or status information\r\n", io->stdOut);
#if LEDS_CONFIG_HAS_LEFT_RED_LED
  McuShell_SendHelpStr((unsigned char*)"  leftred on|off|neg", (const unsigned char*)"Control left red LED\r\n", io->stdOut);
#endif
#if LEDS_CONFIG_HAS_LEFT_RED_LED
  McuShell_SendHelpStr((unsigned char*)"  rightred on|off|neg", (const unsigned char*)"Control right red LED\r\n", io->stdOut);
#endif
#if LEDS_CONFIG_HAS_RED_LED
  McuShell_SendHelpStr((unsigned char*)"  red on|off|neg", (const unsigned char*)"Control red LED\r\n", io->stdOut);
#endif
#if LEDS_CONFIG_HAS_GREEN_LED
  McuShell_SendHelpStr((unsigned char*)"  green on|off|neg", (const unsigned char*)"Control green LED\r\n", io->stdOut);
#endif
#if LEDS_CONFIG_HAS_BLUE_LED
  McuShell_SendHelpStr((unsigned char*)"  blue on|off|neg", (const unsigned char*)"Control blue LED\r\n", io->stdOut);
#endif
#if LEDS_CONFIG_HAS_ORANGE_LED
  McuShell_SendHelpStr((unsigned char*)"  orange on|off|neg", (const unsigned char*)"Control orange LED\r\n", io->stdOut);
#endif
#if LEDS_CONFIG_HAS_ONBOARD_LED
  McuShell_SendHelpStr((unsigned char*)"  onboard on|off|neg", (const unsigned char*)"Control onboard LED\r\n", io->stdOut);
#endif
  return ERR_OK;
}

static bool ParseLedCommand(const unsigned char *cmd, bool *handled, unsigned char *ledStr, LEDS_Leds_e led) {
  unsigned char ledCmd[16];
  size_t len;

  McuUtility_strcpy(ledCmd, sizeof(ledCmd), (const unsigned char*)"led ");
  McuUtility_strcat(ledCmd, sizeof(ledCmd), ledStr);
  len = McuUtility_strlen((char*)ledCmd);
  if (McuUtility_strncmp((char*)cmd, (char*)ledCmd, len)==0) {
    *handled = true;
    cmd += len;
    if (McuUtility_strcmp((char*)cmd, " on")==0) {
      Leds_On(led);
      return true;
    } else if (McuUtility_strcmp((char*)cmd, " off")==0) {
      Leds_Off(led);
      return true;
    } else if (McuUtility_strcmp((char*)cmd, " neg")==0) {
      Leds_Neg(led);
      return true;
    }
  }
  return false; /* not found */
}

uint8_t Leds_ParseCommand(const uint8_t *cmd, bool *handled, McuShell_ConstStdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "led help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "led status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
#if LEDS_CONFIG_HAS_LEFT_RED_LED
  } else if (ParseLedCommand(cmd, handled, (unsigned char*)"leftred", LEDS_LEFT_RED)) {
    return ERR_OK;
#endif
#if LEDS_CONFIG_HAS_RIGHT_RED_LED
  } else if (ParseLedCommand(cmd, handled, (unsigned char*)"rightred", LEDS_RIGHT_RED)) {
    return ERR_OK;
#endif
#if LEDS_CONFIG_HAS_RED_LED
  } else if (ParseLedCommand(cmd, handled, (unsigned char*)"red", LEDS_RED)) {
    return ERR_OK;
#endif
#if LEDS_CONFIG_HAS_GREEN_LED
  } else if (ParseLedCommand(cmd, handled, (unsigned char*)"green", LEDS_GREEN)) {
    return ERR_OK;
#endif
#if LEDS_CONFIG_HAS_BLUE_LED
  } else if (ParseLedCommand(cmd, handled, (unsigned char*)"blue", LEDS_BLUE)) {
    return ERR_OK;
#endif
#if LEDS_CONFIG_HAS_ORANGE_LED
  } else if (ParseLedCommand(cmd, handled, (unsigned char*)"orange", LEDS_ORANGE)) {
    return ERR_OK;
#endif
#if LEDS_CONFIG_HAS_ONBOARD_LED
  } else if (ParseLedCommand(cmd, handled, (unsigned char*)"onboard", LEDS_ONBOARD)) {
    return ERR_OK;
#endif
  }
  return ERR_OK; /* no error */
}
#endif /* PL_CONFIG_USE_SHELL */

void Leds_Init(void) {
  McuLED_Config_t config;

  LEDS_CONFIG_ENABLE_CLOCK(); /* enable clocking or initialize GPIO as required by hardware */

  McuLED_GetDefaultConfig(&config);
  config.isOnInit = false;

#if LEDS_CONFIG_HAS_LEFT_RED_LED
  #if McuLib_CONFIG_CPU_IS_KINETIS
  config.hw.gpio = LEDS_CONFIG_LEFT_RED_GPIO;
  config.hw.port = LEDS_CONFIG_LEFT_RED_PORT;
  #endif
  config.hw.pin = LEDS_CONFIG_LEFT_RED_PIN;
  config.isLowActive = LEDS_CONFIG_LEFT_RED_LOW_ACTIVE;
  leds[LEDS_LEFT_RED] = McuLED_InitLed(&config);
  if (leds[LEDS_LEFT_RED]==NULL) {
    for(;;) {}
  }
#endif

#if LEDS_CONFIG_HAS_RIGHT_RED_LED
  #if McuLib_CONFIG_CPU_IS_KINETIS
  config.hw.gpio = LEDS_CONFIG_RIGHT_RED_GPIO;
  config.hw.port = LEDS_CONFIG_RIGHT_RED_PORT;
  #endif
  config.hw.pin = LEDS_CONFIG_RIGHT_RED_PIN;
  config.isLowActive = LEDS_CONFIG_RIGHT_RED_LOW_ACTIVE;
  leds[LEDS_RIGHT_RED] = McuLED_InitLed(&config);
  if (leds[LEDS_RIGHT_RED]==NULL) {
    for(;;) {}
  }
#endif

#if LEDS_CONFIG_HAS_RED_LED
  #if McuLib_CONFIG_CPU_IS_KINETIS
  config.hw.gpio = LEDS_CONFIG_RED_GPIO;
  config.hw.port = LEDS_CONFIG_RED_PORT;
  #endif
  config.hw.pin = LEDS_CONFIG_RED_PIN;
  config.isLowActive = LEDS_CONFIG_RED_LOW_ACTIVE;
  leds[LEDS_RED] = McuLED_InitLed(&config);
  if (leds[LEDS_RED]==NULL) {
    for(;;) {}
  }
#endif

#if LEDS_CONFIG_HAS_GREEN_LED
  #if McuLib_CONFIG_CPU_IS_KINETIS
  config.hw.gpio = LEDS_CONFIG_GREEN_GPIO;
  config.hw.port = LEDS_CONFIG_GREEN_PORT;
  #endif
  config.hw.pin = LEDS_CONFIG_GREEN_PIN;
  config.isLowActive = LEDS_CONFIG_GREEN_LOW_ACTIVE;
  leds[LEDS_GREEN] = McuLED_InitLed(&config);
  if (leds[LEDS_GREEN]==NULL) {
    for(;;) {}
  }
#endif

#if LEDS_CONFIG_HAS_BLUE_LED
  #if McuLib_CONFIG_CPU_IS_KINETIS
  config.hw.gpio = LEDS_CONFIG_BLUE_GPIO;
  config.hw.port = LEDS_CONFIG_BLUE_PORT;
  #endif
  config.hw.pin = LEDS_CONFIG_BLUE_PIN;
  config.isLowActive = LEDS_CONFIG_BLUE_LOW_ACTIVE;
  leds[LEDS_BLUE] = McuLED_InitLed(&config);
  if (leds[LEDS_BLUE]==NULL) {
    for(;;) {}
  }
#endif

#if LEDS_CONFIG_HAS_ORANGE_LED
  #if McuLib_CONFIG_CPU_IS_KINETIS
  config.hw.gpio = LEDS_CONFIG_ORANGE_GPIO;
  config.hw.port = LEDS_CONFIG_ORANGE_PORT;
  #endif
  config.hw.pin = LEDS_CONFIG_ORANGE_PIN;
  config.isLowActive = LEDS_CONFIG_ORANGE_LOW_ACTIVE;
  leds[LEDS_ORANGE] = McuLED_InitLed(&config);
  if (leds[LEDS_ORANGE]==NULL) {
    for(;;) {}
  }
#endif

#if LEDS_CONFIG_HAS_ONBOARD_LED
  #if PL_CONFIG_USE_PICO_W
  /* NOTE: you have to call Leds_InitFromTask() from a FreeRTOS task context! */
  #else
    #if McuLib_CONFIG_CPU_IS_KINETIS
    config.hw.gpio = LEDS_CONFIG_ONBOARD_GPIO;
    config.hw.port = LEDS_CONFIG_ONBOARD_PORT;
    #endif
    config.hw.pin = LEDS_CONFIG_ONBOARD_PIN;
    config.isLowActive = LEDS_CONFIG_ONBOARD_LOW_ACTIVE;
    leds[LEDS_ONBOARD] = McuLED_InitLed(&config);
    if (leds[LEDS_ONBOARD]==NULL) {
      for(;;) {}
    }
  #endif
#endif
}

void Leds_InitFromTask(void) {
#if !PL_CONFIG_USE_WIFI && PL_CONFIG_USE_PICO_W
  if (cyw43_arch_init()==0)  { /* need to init for accessing LEDs and other pins */
    McuPicoWiFi_SetArchIsInitialized(true);
  } else {
    McuLog_fatal("failed initializing CYW43");
    for(;;){}
  }
  onBoardLedIsOn = false;
#endif
}

void Leds_Deinit(void) {
  for(int i=0; i<LEDS_NOF_LEDS; i++) {
    if (leds[i]!=NULL) {
      leds[i] = McuLED_DeinitLed(leds[i]);
    }
  }
}

#endif /* PL_CONFIG_USE_LEDS */
