/*
 * SS4200-E Hardware API
 * Copyright (c) 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <sys/ioctl.h>

#include "ptypes.h"
#include "nas.h"
#include "gpio.h"

/*********************************************************************
 * Function: setGpioLedState
 *
 * Set the state of an LED via GPIO
 *
 * Parameters:
 * lFd       File descriptor of gpio device file
 * tLed      Enumeration specifying the LED whose state we're setting
 * tLedState Enumeration specifying what LED state to set
 *
 * Returns:
 * CF_OK on success, CF_FAILURE on error
 *
 *********************************************************************/
inline tCfStatus setGpioLedState( int lFd, tLeds tLed, tLedStates tLedState )
{
	SET_STATE_INFO ledReq;
   tCfStatus tStatus = CF_OK;

   ledReq.tLed = tLed;
   ledReq.tLedState = tLedState;
   if( -1 == ioctl( lFd, IOCTL_NAS_FPLED_SET_STATE, &ledReq ) )
   {
      tStatus = CF_FAILURE;
   }

   return( tStatus );
}

/*********************************************************************
 * Function: getGpioRecoveryButton
 *
 * Get the status of the recovery button via GPIO
 *
 * Parameters:
 * lFd       File descriptor of gpio device file
 * pulButton Will be set with the value of the recovery button state
 *
 * Returns:
 * CF_OK on success, CF_FAILURE on error
 *
 *********************************************************************/
inline tCfStatus getGpioRecoveryButton( int lFd, ui32* pulButton )
{
   tCfStatus tStatus = CF_OK;

   if( -1 == ioctl( lFd, IOCTL_NAS_READ_RECOVERY_BUTTON, pulButton ) )
   {
      tStatus = CF_FAILURE;
   }

   return( tStatus );
}


/*********************************************************************
 * Function: setGpioWatchdog
 *
 * Set the watchdog via GPIO
 *
 * Parameters:
 * lFd       File descriptor of gpio device file
 * ulSeconds The timeout to set on the watchdog timer
 *
 * Returns:
 * CF_OK on success, CF_FAILURE on error
 *
 *********************************************************************/
inline tCfStatus setGpioWatchdog( int lFd, ui32 ulSeconds )
{
   tCfStatus tStatus = CF_OK;

   if( -1 == ioctl( lFd, IOCTL_NAS_SET_WATCHDOG, &ulSeconds ) )
   {
      tStatus = CF_FAILURE;
   }

   return( tStatus );
}
