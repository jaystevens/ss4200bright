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

#ifndef __GPIO_H__
#define __GPIO_H__

#include "nas.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Device file */
#define GPIO_DEV                       "/dev/nasGpio0"

/* Device type -- in the "User Defined" range. */
#define NAS_TYPE                       ( 0x86 )

/* The IOCTL function codes from 0x800 to 0xFFF are for customer use. */
#define IOCTL_NAS_FPLED_SET_STATE      _IOW( NAS_TYPE, 0x01, SET_STATE_INFO )
#define IOCTL_NAS_READ_RECOVERY_BUTTON _IOR( NAS_TYPE, 0x03, ui32 )
#define IOCTL_NAS_SET_WATCHDOG         _IOW( NAS_TYPE, 0x04, ui32 )

typedef struct
{
   tLeds      tLed;            
   tLedStates tLedState;   
} SET_STATE_INFO, *PSET_STATE_INFO;


tCfStatus setGpioLedState( int lNasGpio, tLeds tLed, tLedStates tLedState );
tCfStatus getGpioRecoveryButton( int lNasGpio, ui32* pulButton );
tCfStatus setGpioWatchdog( int lNasGpio, ui32 ulSeconds );


#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H__ */
