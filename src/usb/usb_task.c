#if 0 /* handled by McuShellCdcDevice */
/*
 * Copyright (c) 2022, Stefan Odermatt
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "McuRTOS.h"
#include "McuLog.h"
#include "tusb.h"
#include "clock_config.h"

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static TimerHandle_t blinky_tm; /* blinky timer handle, to indicate connection status */

static void led_blinky_cb(TimerHandle_t xTimer) {
   /* \TODO */
}

//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+
void USB0_IRQHandler(void)
{
#if CFG_TUH_ENABLED
  tuh_int_handler(0);
#endif
#if CFG_TUD_ENABLED
  tud_int_handler(0);
#endif
}

/*!
 * \brief sets up USB Clock and USB interrupts
 */
static void usb_hardware_init(void) {
#if CFG_TUSB_OS == OPT_OS_FREERTOS
	/* If freeRTOS is used, IRQ priority is limit by max syscall ( smaller is higher ) */
	NVIC_SetPriority(USB0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
#endif
	/* Set Clock for USB (is not enabled by default. Alternatively, could call usb_clock_init() in the system module. */
	SystemCoreClockUpdate();
	CLOCK_EnableUsbfs0Clock(kCLOCK_UsbSrcIrc48M, 48000000U);
}

static bool tiny_usb_init(void) {
  tusb_rhport_init_t const rhport_init = {
    .role  = TUSB_ROLE_DEVICE,
    .speed = TUSB_SPEED_FULL,
  };
  return tusb_init(0, &rhport_init);
}

/*!
 * brief\ USB Device Driver task
 * This top level thread process all usb events and invoke callbacks
 */
void usb_device_task(void* param) {
  (void)param;

  if (!tiny_usb_init()) {
	  McuLog_fatal("failed initializing USB");
	  for(;;) {}
  }
  for(;;) {
    // put this thread to waiting state until there is a new events
    tud_task();
    // following code only run if tud_task() process at least 1 event
    tud_cdc_write_flush();
  }
}

//--------------------------------------------------------------------+
// USB Device callbacks
//--------------------------------------------------------------------+
/*!
 * \brief Invoked when device is mounted
 */
void tud_mount_cb(void) {
  if (blinky_tm!=NULL) {
	  xTimerGetPeriod(blinky_tm);
	  xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
  }
}

/*!
 * \brief Invoked when device is unmounted
 */
void tud_umount_cb(void) {
  if (blinky_tm!=NULL) {
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
  }
}

/*!
 * \brief Invoked when usb bus is suspended
 * remote_wakeup_en : if host allow us  to perform remote wakeup
 * Within 7ms, device must draw an average of current less than 2.5 mA from bus
 */
void tud_suspend_cb(bool remote_wakeup_en) {
  (void)remote_wakeup_en;
  if (blinky_tm!=NULL) {
    xTimerGetPeriod(blinky_tm);
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
  }
}

/*!
 * \brief module initialization
 */
void UsbTask_Init(void) {
#if 0
	usb_hardware_init(); /* Set up USB clock and interrupts */

	BaseType_t res;
	/* Create USB device Task */
	res =  xTaskCreate(usb_device_task, "usb_device_task", 1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+2, NULL);
	if(res != pdPASS) {
		/* Error */
		for(;;) {}
	}
#endif
	blinky_tm = xTimerCreate("BlinkyTimer", pdMS_TO_TICKS(BLINK_NOT_MOUNTED), true, NULL, led_blinky_cb);
	if(blinky_tm == NULL) {
		/* Error */
		for(;;) {}
	}
	xTimerStart(blinky_tm, 0);
}

/*!
 * \brief module deinitialization
 */
void UsbTask_Deinit(void) {
}

#endif /* if 0 */