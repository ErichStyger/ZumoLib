#if 0 /* not used */
/*
 * Copyright (c) 2022, Stefan Odermatt
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "McuRTOS.h"
#include <stdint.h>
#include "tusb.h"
#include <stdio.h>

#define TestBufferSize (CFG_TUD_CDC_RX_BUFSIZE)

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
/*!
 * \brief FreeRTOS CDC task, produces a simple echo
 */
void cdc_task(void* params) {
	(void) params;
	for(;;) {
		// There are data available
		while (tud_cdc_available()) {
			uint8_t buf[TestBufferSize];
			uint16_t receivedBytes;
			// read and echo back
			receivedBytes = tud_cdc_read(buf, sizeof(buf));
			tud_cdc_write(buf, receivedBytes);
		}
		tud_cdc_write_flush();
		vTaskDelay(1);
	}
}

/*!
 * \brief module initialization
 */
void UsbCdc_Init(void) {
	BaseType_t res;

	res = xTaskCreate(cdc_task, "cdc", 2048/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+2, NULL);
  if(res !=pdPASS) {
    /* Error */
    for(;;) {}
  }
}

/*!
 * \brief module deinitialization
 */
void UsbCdc_Deinit(void) {
	/* Nothing to do here */
}

#endif