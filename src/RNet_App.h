/**
 * \file
 * \brief This is the interface to the application entry point.
 * \author (c) 2016 Erich Styger, http://mcuoneclipse.com/
 * \note MIT License (http://opensource.org/licenses/mit-license.html)
 */

#ifndef RNETAPP_H_
#define RNETAPP_H_

#include "RNet/McuRNetConfig.h"
#if McuRNET_CONFIG_IS_ENABLED

#include "platform.h"
#include "McuRTOS.h"
#include "RNet/RNWK.h"
#include "RNet/RApp.h"

#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"
  /*!
   * \brief Parses RNet module shell commands.
   * \param cmd Command string to parse.
   * \param handled Set to TRUE if the command was handled.
   * \param io Shell standard I/O descriptor.
   * \return Error code, ERR_OK if everything was fine.
   */
  uint8_t RNETA_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

/*!
 * \brief Sends a 16bit ID plus 32bit value pair
 * \param msgType Message type
 * \param id Message ID
 * \param value Message value
 * \param addr Remote node address
 * \param flags Network flags, like request for acknowledge
 * \return Error code, ERR_OK if no failure.
 */
uint8_t RNETA_SendIdValuePairMessage(uint8_t msgType, uint16_t id, uint32_t value, RAPP_ShortAddrType addr, RAPP_FlagsType flags);

/*!
 * \brief Set the remote node source address.
 * \param addr Node address
 */
void RNETA_SetSrcAddr(RNWK_ShortAddrType addr);

/*!
 * \brief Return the current remote source node address.
 * \return Node address
 */
RNWK_ShortAddrType RNETA_GetSrcAddr(void);

/*!
 * \brief Set the remote node destination address.
 * \param addr Node address
 */
void RNETA_SetDestAddr(RNWK_ShortAddrType addr);

/*!
 * \brief Return the current remote destination node address.
 * \return Node address
 */
RNWK_ShortAddrType RNETA_GetDestAddr(void);

/*! \brief Driver de-initialization */
void RNETA_Deinit(void);

/*! \brief Driver initialization */
void RNETA_Init(void);

#endif /* McuRNET_CONFIG_IS_ENABLED */

#endif /* RNETAPP_H_ */
