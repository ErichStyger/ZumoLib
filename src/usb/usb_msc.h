/*
 * Copyright (c) 2022, Stefan Odermatt
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief USB mass-storage helper interface.
 * \author Stefan Odermatt
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef USB_MSC_H_
#define USB_MSC_H_

#include <stdbool.h>
#include "McuShell.h"

/*! Error types for MSC module. */
typedef enum
{
	ERR_MSC_OK,
	ERR_MSC_FAILURE
} Err_Msc_t;

/*!
 * \brief Indicates whether USB MSC is connected.
 * \return TRUE if MSC is not connected.
 */
bool UsbMsc_IsEjected(void);

/*!
 * \brief Indicates whether copying LittleFS to FatFS has finished.
 * \return TRUE if the copy has finished.
 */
bool UsbMsc_CopyFinished(void);

/*!
 * \brief Toggles the USB MSC connection state.
 * At least one copy from LittleFS to FatFS must be performed before a connection is allowed.
 */
void UsbMsc_ToggleTransferState(void);

/*!
 * \brief Starts copying the LittleFS file system to FatFS.
 */
void UsbMsc_StartFSCopy(void);

/*!
 * \brief Parses shell commands for the MSC module.
 */
uint8_t UsbMsc_ParseCommand(const unsigned char* cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Module initialization.
 */
void UsbMsc_Init(void);

/*!
 * \brief Module deinitialization.
 */
void UsbMsc_Deinit(void);

#endif /* USB_MSC_H_ */
