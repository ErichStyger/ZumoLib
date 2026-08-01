/**
 * \file
 * \brief Interface to the line following module
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This is the interface to line following module.
 */

#ifndef LINEFOLLOW_H_
#define LINEFOLLOW_H_

#include "platform.h"
#if PL_CONFIG_LINE_FOLLOWING
#include "reflectance.h"

#if PL_CONFIG_USE_SHELL
#include "McuShell.h"

/*!
 * \brief Module command line parser
 * \param cmd Pointer to command string to be parsed
 * \param handled Set to TRUE if command has handled by parser
 * \param io Shell standard I/O handler
 * \return Error code, ERR_OK if everything was ok
 */
uint8_t LineFollow_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

/*!
 * \brief Start line following
 */
void LineFollow_StartFollowing(void);

/*!
 * \brief Stop line following
 */
void LineFollow_StopFollowing(void);

/*!
 * \brief Start/stop line following
 */
void LineFollow_StartStopFollowing(void);

/*!
 * \brief Function to determine if line following is active
 * \return TRUE if currently line following, FALSE otherwise
 */
bool LineFollow_IsFollowing(void);

/*!
 * \brief Move from outside onto a line/segment to follow it.
 * \return Returns TRUE if still on line
 */
bool LineFollow_MoveOnSegment(bool turningLeft);

/*!
 * \brief follows a line segment.
 * \return Returns TRUE if still on line segment
 */
bool LineFollow_FollowSegment(REF_LineKindMode mode, bool forward);

/*!
 * \brief Follows a line segment using a fixed line-position setpoint.
 * \param mode Line kind interpretation mode.
 * \param setLinePos Desired normalized line position.
 * \return TRUE if still on line segment, FALSE otherwise.
 */
bool LineFollow_FollowSegmentLinePos(REF_LineKindMode mode, uint16_t setLinePos);

/*!
 * \brief Module initialization.
 */
void LineFollow_Init(void);

/*!
 * \brief Module de-initialization.
 */
void LineFollow_Deinit(void);

#endif /* PL_CONFIG_LINE_FOLLOWING */

#endif /* LINEFOLLOW_H_ */
