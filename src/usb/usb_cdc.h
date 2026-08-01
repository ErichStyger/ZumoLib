/*
 * Copyright (c) 2022, Stefan Odermatt
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief USB CDC helper interface.
 * \author Stefan Odermatt
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef USB_CDC_H_
#define USB_CDC_H_

/*!
 * \brief Module initialization.
 */
void UsbCdc_Init(void);

/*!
 * \brief Module deinitialization.
 */
void UsbCdc_Deinit(void);

#endif /* USB_CDC_H_ */
