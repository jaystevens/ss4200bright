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

#ifndef __SMBUS_H__
#define __SMBUS_H__

#ifdef __cplusplus
extern "C" {
#endif


#define SMBUS_DEV         "/dev/i2c-0"
#define HWMON_SLAVE       ( 0x2e )

/* Sensor register definitions copied from Windows Sio driver */
#define V2_IN_REG         ( 0x20 )
#define VCCP_REG          ( 0x21 )
#define VCC_REG           ( 0x22 )
#define P5VTR_REG         ( 0x23 )
#define V1_IN_REG         ( 0x24 )
#define RMT_DIODE1_REG    ( 0x25 )
#define INT_TEMP_REG      ( 0x26 )
#define RMT_DIODE2_REG    ( 0x27 )
#define FANTACH1_LSB_REG  ( 0x28 )
#define FANTACH1_MSB_REG  ( 0x29 )
#define FANTACH2_LSB_REG  ( 0x2a )
#define FANTACH2_MSB_REG  ( 0x2b )
#define FANTACH3_LSB_REG  ( 0x2c )
#define FANTACH3_MSB_REG  ( 0x2d )
#define READY_LOCK_START  ( 0x40 )
#define AD_CONV5_REG      ( 0x84 )
#define AD_CONV1_REG      ( 0x85 )
#define AD_CONV2_REG      ( 0x86 )
#define AD_CONV3_REG      ( 0x87 )
#define AD_CONV4_REG      ( 0x88 )
#define VTR_REG           ( 0x99 )
#define VBAT_REG          ( 0x9a )
#define PWMA_CYCLE_REG    ( 0xa5 )
#define PWMA_FREQ_REG     ( 0xa7 )

/* SMSC READY_LOCK_START bit definitions copied from Windows Sio driver */
#define SMSC_MONITOR_VBAT ( 0x10 )


/* Bus access functions */
tCfStatus smbusSetSlave( int lFd, ui8 ucSlave );
tCfStatus smbusReadByteData( int lFd, ui8 ucReg, ui8* pucData );
tCfStatus smbusWriteByteData( int lFd, ui8 ucReg, ui8 ucData );

/* Utility functions */
tCfStatus smbusReadSensors( int lFd, tHwmInfoRaw *ptHwmInfoRaw );      


#ifdef __cplusplus
}
#endif

#endif /* __SMBUS_H__ */
