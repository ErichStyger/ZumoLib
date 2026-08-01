/**
 * \file
 * \brief Buzzer driver interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module implements the driver for the buzzer.
 */

#include "platform.h"
#if PL_CONFIG_USE_BUZZER
#include "buzzer.h"
#include "McuGPIO.h"
#include "McuUtility.h"
#include "McuRTOS.h"
#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"
#endif

#define BUZZER_USE_FREERTOS_TIMER   (1)  /* if using FreeRTOS or software timer */

static TimerHandle_t timerToggle, timerDuration;
static McuGPIO_Handle_t Buzzer_Pin;

typedef struct {
  int freq; /* frequency */
  int ms; /* milliseconds */
} Buzzer_Tune;

#define NOTE_C5 261
#define NOTE_D5 293
#define NOTE_E5 329
#define NOTE_F5 349
#define NOTE_G5 392

static const Buzzer_Tune MelodyJingleBells[] =
{ /* freq, ms */
    {NOTE_E5, 4*100},
    {NOTE_E5, 4*100},
    {NOTE_E5, 4*200},
    {NOTE_E5, 4*100},
    {NOTE_E5, 4*100},
    {NOTE_E5, 4*200},
    {NOTE_E5, 4*100},
    {NOTE_G5, 4*100},
    {NOTE_C5, 4*100},
    {NOTE_D5, 4*100},
    {NOTE_E5, 4*200},
    {0, 4*100},
    {NOTE_F5, 4*100},
    {NOTE_F5, 4*100},
    {NOTE_F5, 4*100},
    {NOTE_F5, 4*100},
    {NOTE_F5, 4*100},
    {NOTE_E5, 4*100},
    {NOTE_E5, 4*100},
    {NOTE_E5, 4*100},
    {NOTE_E5, 4*100},
    {NOTE_D5, 4*100},
    {NOTE_D5, 4*100},
    {NOTE_E5, 4*100},
    {NOTE_D5, 4*200},
    {NOTE_G5, 4*200},
};

static const Buzzer_Tune MelodyWelcome[] =
{ /* freq, ms */
    {300,500},
    {500,200},
    {300,100},
    {200,300},
    {500,400},
    {300,100},
    {200,300},
    {300,100},
    {200,300},
    {500,400},
};

static const Buzzer_Tune MelodyButton[] =
{ /* freq, ms */
    {200,100},
    {600,100},
};

static const Buzzer_Tune MelodyButtonLong[] =
{ /* freq, ms */
    {500,50},
    {100,100},
    {300,50},
    {150,50},
    {450,50},
    {500,50},
    {250,200},
};

static const Buzzer_Tune MelodyMazeDestination[] =
{ /* freq, ms */
    {500,50},
    {100,100},
    {300,50},
    {150,50},
    {450,50},
    {500,50},
    {250,200},
    {500,50},
    {100,100},
    {300,50},
    {150,50},
    {450,50},
    {500,50},
    {250,200},
    {500,50},
    {100,100},
    {300,50},
    {150,50},
    {450,50},
    {500,50},
    {250,200},
};

typedef struct {
  int idx; /* current index */
  int maxIdx; /* maximum index */
  const Buzzer_Tune *melody;
} MelodyDesc;

static MelodyDesc *playingMelody = NULL;

static MelodyDesc Buzzer_Melodies[] = {
  {0, sizeof(MelodyWelcome)/sizeof(MelodyWelcome[0]),                   MelodyWelcome}, /* BUZZER_TUNE_WELCOME */
  {0, sizeof(MelodyButton)/sizeof(MelodyButton[0]),                     MelodyButton}, /* BUZZER_TUNE_BUTTON */
  {0, sizeof(MelodyButtonLong)/sizeof(MelodyButtonLong[0]),             MelodyButtonLong}, /* BUZZER_TUNE_BUTTON_LONG */
  {0, sizeof(MelodyMazeDestination)/sizeof(MelodyMazeDestination[0]),   MelodyMazeDestination}, /* BUZZER_TUNE_MAZE_DESTINATION */
  {0, sizeof(MelodyJingleBells)/sizeof(MelodyJingleBells[0]),           MelodyJingleBells}, /* BUZZER_TUNE_JINGLE_BELLS */
};

uint8_t Buzzer_Beep(uint16_t freqHz, uint16_t durationMs) {
  BaseType_t res;

  if (freqHz>1000) { /* timer frequency is max 1 kHz */
    return ERR_FAILED;
  }
  if (freqHz==0) { /* pause */
    res = xTimerChangePeriod(timerToggle, pdMS_TO_TICKS(durationMs), pdMS_TO_TICKS(100));
  } else {
    res = xTimerChangePeriod(timerToggle, pdMS_TO_TICKS(1000/freqHz), pdMS_TO_TICKS(100));
  }
  if (res!=pdPASS) {
    return ERR_FAILED;
  }
  res = xTimerChangePeriod(timerDuration, pdMS_TO_TICKS(durationMs), pdMS_TO_TICKS(100));
  if (res!=pdPASS) {
    return ERR_FAILED;
  }
  res = xTimerStart(timerToggle, pdMS_TO_TICKS(100));
  if (res!=pdPASS) {
    return ERR_FAILED;
  }
  res = xTimerStart(timerDuration, pdMS_TO_TICKS(100));
  if (res!=pdPASS) {
    return ERR_FAILED;
  }
  return ERR_OK;
}

static void Buzzer_Play(void *dataPtr) {
  MelodyDesc *melody = (MelodyDesc*)dataPtr;

  /* play tune */
  (void)Buzzer_Beep(melody->melody[melody->idx].freq, melody->melody[melody->idx].ms);
  melody->idx++;
  if (melody->idx<melody->maxIdx) { /* not reached end of tune? */
  } else { /* end of tune */
    playingMelody = NULL;
  }
}

void Buzzer_StopTune(void) {
  (void)xTimerStop(timerToggle, pdMS_TO_TICKS(100));
  (void)xTimerStop(timerDuration, pdMS_TO_TICKS(100));
}

void Buzzer_StopBeep(void) {
  (void)xTimerStop(timerToggle, pdMS_TO_TICKS(100));
  (void)xTimerStop(timerDuration, pdMS_TO_TICKS(100));
}

uint8_t Buzzer_PlayTune(Buzzer_Tunes tune) {
  if (tune>=BUZZER_TUNE_NOF_TUNES) {
    return ERR_OVERFLOW;
  }
  Buzzer_Melodies[tune].idx = 0; /* reset index */
  playingMelody = &Buzzer_Melodies[tune];
  Buzzer_Play((void*)playingMelody);
  return ERR_OK;
}

static void vTimerCallbackToggle(TimerHandle_t pxTimer) {
  (void)pxTimer; /* not used */
  /* called with TIMER_PERIOD_MS while playing */
  McuGPIO_Toggle(Buzzer_Pin);
}

static void vTimerCallbackDuration(TimerHandle_t pxTimer) {
  /* called with TIMER_PERIOD_MS while playing */
  (void)pxTimer; /* not used */
  (void)xTimerStop(timerToggle, pdMS_TO_TICKS(100));
  if (playingMelody!=NULL) {
    Buzzer_Play((void*)playingMelody); /* next tune */
  }
}

#if PL_CONFIG_USE_SHELL
static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"buzzer", (unsigned char*)"Group of buzzer commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows buzzer help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  buz <freq> <time>", (unsigned char*)"Beep for time (ms) and frequency (Hz)\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  play tune <tune>", (unsigned char*)"Play tune, default tune 0\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[16];
  McuShell_SendStatusStr((unsigned char*)"buzzer", (unsigned char*)"Buzzer status\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0 - ");
  McuUtility_strcatNum32u(buf, sizeof(buf), BUZZER_TUNE_NOF_TUNES-1);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  tunes", buf, io->stdOut);
  return ERR_OK;
}

uint8_t Buzzer_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;
  uint16_t freq, duration;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"buzzer help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"buzzer status")==0) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if (McuUtility_strncmp((char*)cmd, (char*)"buzzer buz ", sizeof("buzzer buz ")-1)==0) {
    *handled = TRUE;
    p = cmd+sizeof("buzzer buz ")-1;
    if (McuUtility_ScanDecimal16uNumber(&p, &freq)==ERR_OK && McuUtility_ScanDecimal16uNumber(&p, &duration)==ERR_OK) {
      if (Buzzer_Beep(freq, duration)!=ERR_OK) {
        McuShell_SendStr((unsigned char*)"Starting buzzer failed\r\n", io->stdErr);
        return ERR_FAILED;
      }
      return ERR_OK;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"buzzer play tune ", sizeof("buzzer play tune ")-1)==0) {
    uint8_t tune;

    *handled = TRUE;
    p = cmd+sizeof("buzzer play tune ")-1;
    McuUtility_ScanDecimal8uNumber(&p, &tune);
    if (tune<BUZZER_TUNE_NOF_TUNES) {
      return Buzzer_PlayTune(tune);
    } else {
      return ERR_FAILED;
    }
  } else if (McuUtility_strcmp((char*)cmd, (char*)"buzzer play tune")==0) {
    *handled = TRUE;
    return Buzzer_PlayTune(BUZZER_TUNE_WELCOME);
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

void Buzzer_Deinit(void) {
  (void)xTimerDelete(timerToggle, pdMS_TO_TICKS(100));
  timerToggle = NULL;
  (void)xTimerDelete(timerDuration, pdMS_TO_TICKS(100));
  timerDuration = NULL;
  Buzzer_Pin = McuGPIO_DeinitGPIO(Buzzer_Pin);
}

void Buzzer_Init(void) {
  McuGPIO_Config_t gpioConfig;

  BUZZER_CONFIG_ENABLE_CLOCK();
  McuGPIO_GetDefaultConfig(&gpioConfig);
  gpioConfig.hw.gpio = BUZZER_PINS_USER_GPIO;
  gpioConfig.hw.port = BUZZER_PINS_USER_PORT;
  gpioConfig.hw.pin = BUZZER_PINS_USER_PIN;
  gpioConfig.isInput = false;
  gpioConfig.isHighOnInit = true;
  Buzzer_Pin = McuGPIO_InitGPIO(&gpioConfig);
  timerToggle = xTimerCreate(
          "buzToggle", /* name */
          1, /* period/time */
          pdTRUE, /* auto reload */
          (void*)0, /* timer ID */
          vTimerCallbackToggle); /* callback */
  if (timerToggle==NULL) {
    for(;;); /* failure! */
  }
  timerDuration = xTimerCreate(
          "buzDuration", /* name */
          1, /* period/time */
          pdFALSE, /* auto reload */
          (void*)0, /* timer ID */
          vTimerCallbackDuration); /* callback */
  if (timerDuration==NULL) {
    for(;;); /* failure! */
  }
}
#endif /* PL_CONFIG_USE_BUZZER */
