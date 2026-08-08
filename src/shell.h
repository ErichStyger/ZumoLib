/*
 * Copyright (c) 2019-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Command line shell interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SHELL_H_
#define SHELL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "McuShell.h"
#include "McuRTOS.h"

/*!
 * \brief Send a string to all supported I/Os
 * \param str String to send
 */
void Shell_SendString(const unsigned char *str);

/*!
 * \brief Send a string to a given IO. It tries to accelerate it by sending a buffer instead char by char.
 * \param str String to be sent.
 * \param io I/O to be used.
 */
void Shell_SendStringToIO(const unsigned char *str, McuShell_ConstStdIOType *io);

/*!
 * \brief Send a character to all supported I/Os
 * \param ch Character to send
 */
void SHELL_SendChar(unsigned char ch);

/*!
 * \brief Parses a command with a given standard I/O channel
 * \param command Command to be parsed
 * \param io I/O to be used. If NULL, the standard default I/O will be used
 * \param silent If parsing shall be silent or not
 * \return Error code, ERR_OK for no error
 */
uint8_t Shell_ParseCommandIO(const unsigned char *command, McuShell_ConstStdIOType *io, bool silent);

/*!
 * \brief Parses a command with a given standard I/O channel, in non-silent mode
 * \param command Command to be parsed
 * \param io I/O to be used. If NULL, the standard default I/O will be used
 * \return Error code, ERR_OK for no error
 */
uint8_t Shell_ParseCommandWithIO(unsigned char *cmd, McuShell_ConstStdIOType *io);

/*!
 * \brief Get a mutex handle to get exclusive control over the shell parsing. Needed for ESP-2-Robot gateway.
 * \return Mutex handle to control access to shell parsing.
 */
SemaphoreHandle_t Shell_GetMutex(void);

/*!
 * \brief Module de-initialization
 */
void Shell_Deinit(void);

/*!
 * \brief Module Initialization
 */
void Shell_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SHELL_H_ */
