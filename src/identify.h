/**
 * \file
 * \brief Module to identify different devices based on their unique ID.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * With this module individual devices are identified based on their unique ID.
 */

#ifndef __IDENTIFY_H_
#define __IDENTIFY_H_

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#if PL_CONFIG_USE_IDENTIFY
  #if PL_CONFIG_USE_SHELL
    #include "McuShell.h"

    /*!
     * \brief Parses a command
     * \param cmd Command string to be parsed
     * \param handled Sets this variable to TRUE if command was handled
     * \param io I/O stream to be used for input/output
     * \return Error code, ERR_OK if everything was fine
     */
    uint8_t ID_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
  #endif /* PL_CONFIG_USE_SHELL */

  typedef enum { /*! \todo Verify and update information */
    ID_ROBOT_E0,
    ID_ROBOT_E1,
    ID_ROBOT_E2,
    ID_ROBOT_E3,
    ID_ROBOT_E4,
    ID_ROBOT_E5,
    ID_ROBOT_E6,
    ID_ROBOT_E7,
    ID_ROBOT_E8,
    ID_ROBOT_E9,
    ID_ROBOT_E10,
    ID_ROBOT_E11,
    ID_ROBOT_E12,
    ID_ROBOT_E13,
    ID_ROBOT_E14,
//    ID_ROBOT_E15,
//    ID_ROBOT_E16,
    ID_ROBOT_E17,
    ID_ROBOT_E18,
    ID_ROBOT_E27,
    ID_ROBOT_E34,

    ID_ROBOT_L0, /* USB port ripped off */
    ID_ROBOT_L1,
    ID_ROBOT_L3,
    ID_ROBOT_L17,
    ID_ROBOT_L20,

    ID_ROBOT_R0, /* USB port ripped off */
    ID_ROBOT_R8,
    ID_ROBOT_R9,
    ID_ROBOT_R23,
    ID_ROBOT_R27,
    ID_ROBOT_R28,
    ID_ROBOT_R29,
    ID_ROBOT_R32,
    ID_ROBOT_R33,
    ID_ROBOT_R34,
    ID_ROBOT_R36,
    ID_ROBOT_R37,
    ID_ROBOT_R44,
    ID_ROBOT_R45,

    ID_ROBOT_UNKNOWN, /* unknown robot, unknown ID */
    ID_ROBOT_NONE /* initialization value, used internally */
  } ID_Robot_e;

typedef enum { 
    ID_ESP32E_00, /* UNIQUE ID for ESP Devices*/
    ID_ESP32E_01,
    ID_ESP32E_02,
    ID_ESP32E_03,
    ID_ESP32E_04,
    ID_ESP32E_05,
    ID_ESP32E_06,
    ID_ESP32E_07,
    ID_ESP32E_08,
    ID_ESP32E_09,
    ID_ESP32E_10,
    ID_ESP32E_11,
    ID_ESP32E_12,
    ID_ESP32E_13,
    ID_ESP32E_14,
    ID_ESP32E_15,
    ID_ESP32E_16,
    ID_ESP32E_17,
    ID_ESP32E_18,
    ID_ESP32E_19,
    ID_ESP32E_20,
    ID_ESP32E_21,
    ID_ESP32E_22,

    ID_ESP32E_UNKNOWN, /* unknown robot, unknown ID */
    ID_ESP32E_NONE /* initialization value, used internally */
  } ID_ESP_e;

  /*!
   * \brief Determines the currently running robot device identity.
   * \return Detected robot identifier.
   */
  ID_Robot_e ID_WhichDevice(void);

  /*!
   * \brief Returns the RNet address associated with the detected robot.
   * \return RNet short address.
   */
  uint8_t ID_GetRnetAddr(void);

  /*! \brief Module de-initialization */
  void ID_Deinit(void);

  /*! \brief Module initialization */
  void ID_Init(void);
#endif /* PL_CONFIG_USE_IDENTIFY */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __IDENTIFY_H_ */
