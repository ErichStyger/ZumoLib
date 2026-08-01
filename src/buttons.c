/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Button driver implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_BUTTONS
#include "buttons.h"
#include "buttons_config.h"
#include "McuButton.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "debounce.h"
#if McuLib_CONFIG_CPU_IS_KINETIS
  #include "fsl_port.h"
#endif

typedef struct Buttons_Desc_t {
  McuBtn_Handle_t handle; /* handle of button pin */
} Buttons_Desc_t;

static Buttons_Desc_t Buttons_Infos[BUTTONS_NOF_BUTTONS];

bool Buttons_IsPressed(Buttons_e btn) {
  return Buttons_Infos[btn].handle!=NULL && McuBtn_IsOn(Buttons_Infos[btn].handle);
}

uint32_t Buttons_GetButtons(void) {
  uint32_t val = 0;

  if (Buttons_IsPressed(BUTTONS_USER)) {
    val |= BUTTONS_BIT_USER;
  }
  if (Buttons_IsPressed(BUTTONS_NAV_UP)) {
    val |= BUTTONS_BIT_NAV_UP;
  }
  if (Buttons_IsPressed(BUTTONS_NAV_DOWN)) {
    val |= BUTTONS_BIT_NAV_DOWN;
  }
  if (Buttons_IsPressed(BUTTONS_NAV_LEFT)) {
    val |= BUTTONS_BIT_NAV_LEFT;
  }
  if (Buttons_IsPressed(BUTTONS_NAV_RIGHT)) {
    val |= BUTTONS_BIT_NAV_RIGHT;
  }
  if (Buttons_IsPressed(BUTTONS_NAV_CENTER)) {
    val |= BUTTONS_BIT_NAV_CENTER;
  }
  return val;
}

#if McuLib_CONFIG_CPU_IS_KINETIS && BUTTONS_CONFIG_USE_IRQ
void BUTTONS_CONFIG_INTERRUPT_HANDLER(void) {
  uint32_t flags;
  uint32_t buttons = 0;
  BaseType_t xHigherPriorityTaskWoken = false;

  flags = GPIO_PortGetInterruptFlags(BUTTONS_CONFIG_PINS_USER_GPIO);
  if (flags&(1U<<BUTTONS_CONFIG_PINS_USER_PIN)) {
    buttons |= BUTTONS_BIT_USER; /* \todo could be more generic? */
  }
  GPIO_PortClearInterruptFlags(BUTTONS_CONFIG_PINS_USER_GPIO, flags);
  Debounce_StartDebounceFromISR(buttons, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  __DSB();
}
#elif McuLib_CONFIG_CPU_IS_ESP32 && BUTTONS_CONFIG_USE_IRQ
static void IRAM_ATTR gpio_interrupt_handler(void *args) {
  int gpio = (int)args;
  uint32_t button = 0; /* init */
  BaseType_t xHigherPriorityTaskWoken = false;

  switch(gpio) {
    case BUTTONS_PINS_NAVCENTER_PIN:
      button = BUTTONS_BIT_NAV_CENTER;
      break;
    case BUTTONS_PINS_NAVUP_PIN:
      button = BUTTONS_BIT_NAV_UP;
      break;
    case BUTTONS_PINS_NAVDOWN_PIN:
      button = BUTTONS_BIT_NAV_DOWN;
      break;
    case BUTTONS_PINS_NAVLEFT_PIN:
      button = BUTTONS_BIT_NAV_LEFT;
      break;
    case BUTTONS_PINS_NAVRIGHT_PIN:
      button = BUTTONS_BIT_NAV_RIGHT;
      break;
    default:
      button = 0;
      break;
  }
  if (button!=0) {
   Debounce_StartDebounceFromISR(button, &xHigherPriorityTaskWoken);
  }
}
#endif

void Buttons_Deinit(void) {
  for(int i=0; i<BUTTONS_NOF_BUTTONS; i++) {
    if(Buttons_Infos[i].handle != NULL) {
      Buttons_Infos[i].handle = McuBtn_DeinitButton(Buttons_Infos[i].handle);
    }
  }
}

#if McuLib_CONFIG_CPU_IS_KINETIS

static void initButtonsKinetis(void) {
  McuBtn_Config_t btnConfig;

  BUTTONS_CONFIG_ENABLE_CLOCK();
  McuBtn_GetDefaultConfig(&btnConfig);
  btnConfig.isLowActive = true;
  btnConfig.hw.gpio = BUTTONS_CONFIG_PINS_USER_GPIO;
  btnConfig.hw.port = BUTTONS_CONFIG_PINS_USER_PORT;
  btnConfig.hw.pin = BUTTONS_CONFIG_PINS_USER_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  Buttons_Infos[BUTTONS_USER].handle = McuBtn_InitButton(&btnConfig);

#if BUTTONS_CONFIG_USE_IRQ
  PORT_SetPinInterruptConfig(BUTTONS_CONFIG_PINS_USER_PORT, BUTTONS_CONFIG_PINS_USER_PIN,  kPORT_InterruptFallingEdge);
  NVIC_SetPriority(BUTTONS_CONFIG_INTERRUPT_LINE, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
  EnableIRQ(BUTTONS_CONFIG_INTERRUPT_LINE);
#endif
}

#elif McuLib_CONFIG_CPU_IS_ESP32

static void initButtonsESP(void) {
  McuBtn_Config_t btnConfig;

  BUTTONS_ENABLE_CLOCK();
  McuBtn_GetDefaultConfig(&btnConfig);
  btnConfig.isLowActive = true;
  btnConfig.hw.pull = McuGPIO_PULL_DISABLE;

  btnConfig.hw.pin = BUTTONS_PINS_NAVCENTER_PIN;
  Buttons_Infos[BUTTONS_NAV_CENTER].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVLEFT_PIN;
  Buttons_Infos[BUTTONS_NAV_LEFT].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVRIGHT_PIN;
  Buttons_Infos[BUTTONS_NAV_RIGHT].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVUP_PIN;
  Buttons_Infos[BUTTONS_NAV_UP].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVDOWN_PIN;
  Buttons_Infos[BUTTONS_NAV_DOWN].handle = McuBtn_InitButton(&btnConfig);

#if BUTTONS_CONFIG_USE_IRQ
  #define ESP_INTR_FLAG_DEFAULT 0
  esp_err_t res;

  res = gpio_set_intr_type(BUTTONS_PINS_NAVCENTER_PIN, GPIO_INTR_NEGEDGE); /* set to falling edge */
  if (res!=ESP_OK) {
    McuLog_fatal("Failed setting interrupt type");
    for(;;) {}
  }
  res = gpio_set_intr_type(BUTTONS_PINS_NAVLEFT_PIN, GPIO_INTR_NEGEDGE); /* set to falling edge */
  if (res!=ESP_OK) {
    McuLog_fatal("Failed setting interrupt type");
    for(;;) {}
  }
  res = gpio_set_intr_type(BUTTONS_PINS_NAVRIGHT_PIN, GPIO_INTR_NEGEDGE); /* set to falling edge */
  if (res!=ESP_OK) {
    McuLog_fatal("Failed setting interrupt type");
    for(;;) {}
  }
  res = gpio_set_intr_type(BUTTONS_PINS_NAVUP_PIN, GPIO_INTR_NEGEDGE); /* set to falling edge */
  if (res!=ESP_OK) {
    McuLog_fatal("Failed setting interrupt type");
    for(;;) {}
  }
  res = gpio_set_intr_type(BUTTONS_PINS_NAVDOWN_PIN, GPIO_INTR_NEGEDGE); /* set to falling edge */
  if (res!=ESP_OK) {
    McuLog_fatal("Failed setting interrupt type");
    for(;;) {}
  }
  res = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  if (res!=ESP_OK && res!=ESP_ERR_INVALID_STATE) { /* invalid state if service is already installed */
    McuLog_error("Failed installing interrupt service");
    for(;;) {}
  }
  res = gpio_isr_handler_add(BUTTONS_PINS_NAVCENTER_PIN, gpio_interrupt_handler, (void *)BUTTONS_PINS_NAVCENTER_PIN);
  if (res!=ESP_OK) {
    McuLog_fatal("Failed adding interrupt handler");
    for(;;) {}
  }
  res = gpio_isr_handler_add(BUTTONS_PINS_NAVLEFT_PIN, gpio_interrupt_handler, (void *)BUTTONS_PINS_NAVLEFT_PIN);
  if (res!=ESP_OK) {
    McuLog_fatal("Failed adding interrupt handler");
    for(;;) {}
  }
  res = gpio_isr_handler_add(BUTTONS_PINS_NAVRIGHT_PIN, gpio_interrupt_handler, (void *)BUTTONS_PINS_NAVRIGHT_PIN);
  if (res!=ESP_OK) {
    McuLog_fatal("Failed adding interrupt handler");
    for(;;) {}
  }
  res = gpio_isr_handler_add(BUTTONS_PINS_NAVUP_PIN, gpio_interrupt_handler, (void *)BUTTONS_PINS_NAVUP_PIN);
  if (res!=ESP_OK) {
    McuLog_fatal("Failed adding interrupt handler");
    for(;;) {}
  }
  res = gpio_isr_handler_add(BUTTONS_PINS_NAVDOWN_PIN, gpio_interrupt_handler, (void *)BUTTONS_PINS_NAVDOWN_PIN);
  if (res!=ESP_OK) {
    McuLog_fatal("Failed adding interrupt handler");
    for(;;) {}
  }
#endif /* BUTTONS_CONFIG_USE_IRQ */
}

#endif /* Kinetis/ESP */

#if !BUTTONS_CONFIG_USE_IRQ
static void buttonTask(void *pv) {
  for(;;) {
    uint32_t buttons = Buttons_GetButtons();
    if (buttons!=0) { /* poll buttons */
      Debounce_StartDebounce(buttons);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
#endif /* !BUTTONS_CONFIG_USE_IRQ */

void Buttons_Init(void) {
#if McuLib_CONFIG_CPU_IS_KINETIS
  initButtonsKinetis();
#elif McuLib_CONFIG_CPU_IS_ESP32
  initButtonsESP();
#endif
#if !BUTTONS_CONFIG_USE_IRQ
  if (xTaskCreate(
      buttonTask,  /* pointer to the task */
      "button", /* task name for kernel awareness debugging */
      1024/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+2,  /* initial priority */
      NULL /* optional task handle to create */
    ) != pdPASS)
  {
    McuLog_fatal("Failed creating button task");
    for(;;){} /* error! probably out of memory */
  }
#endif /* !BUTTONS_CONFIG_USE_IRQ */
}
#endif /* PL_CONFIG_USE_BUTTONS */
