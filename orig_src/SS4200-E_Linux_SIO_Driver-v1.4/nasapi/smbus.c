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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "ptypes.h"
#include "nas.h"
#include "smbus.h"

/*********************************************************************
 * Function: smbusReadByteData
 *
 * Read a single byte from the specified register.  The device file
 * should already be open, and the slave address already set.
 *
 * Parameters:
 * lFd       File descriptor of smbus device file
 * ucReg     Register address of the byte to read
 * pucData   Pointer to a byte, filled by this function
 *
 * Returns:
 * CF_OK on success, CF_FAILURE on error
 *
 *********************************************************************/
tCfStatus smbusReadByteData( int lFd, ui8 ucReg, ui8* pucData )
{
   tCfStatus tStatus = CF_OK;
   struct i2c_smbus_ioctl_data cmd;
   union i2c_smbus_data cmdData;

   cmd.read_write = I2C_SMBUS_READ;
   cmd.command = ucReg;
   cmd.size = I2C_SMBUS_BYTE_DATA;
   cmd.data = &cmdData;
   if( -1 == ioctl( lFd, I2C_SMBUS, &cmd ) )
   {
      perror( "Error on smbus read byte" );
      tStatus = CF_FAILURE;
   }
   else
   {
      *pucData = cmdData.byte;
   }

   return( tStatus );
}


/*********************************************************************
 * Function: smbusWriteByteData
 *
 * Write a single byte to the specified register.  The device file
 * should already be open, and the slave address already set.
 *
 * Parameters:
 * lFd       File descriptor of smbus device file
 * ucReg     Register address to which to write the byte
 * ucData    Data byte to write
 *
 * Returns:
 * CF_OK on success, CF_FAILURE on error
 *
 *********************************************************************/
tCfStatus smbusWriteByteData( int lFd, ui8 ucReg, ui8 ucData )
{
   tCfStatus tStatus = CF_OK;
   struct i2c_smbus_ioctl_data cmd;
   union i2c_smbus_data cmdData;

   cmd.read_write = I2C_SMBUS_WRITE;
   cmd.command = ucReg;
   cmd.size = I2C_SMBUS_BYTE_DATA;
   cmd.data = &cmdData;
   cmdData.byte = ucData;
   if( -1 == ioctl( lFd, I2C_SMBUS, &cmd ) )
   {
      perror( "Error on smbus write byte" );
      tStatus = CF_FAILURE;
   }

   return( tStatus );
}


/*********************************************************************
 * Function: smbusSetSlave
 *
 * Set the slave address we'll communicate with in future commands
 * The smbus device should already be open
 *
 * Parameters:
 * lFd       File descriptor of smbus device file
 * ucSlave   Slave address for future read/write commands
 *
 * Returns:
 * CF_OK on success, CF_FAILURE on error
 *
 *********************************************************************/
tCfStatus smbusSetSlave( int lFd, ui8 ucSlave )
{
   tCfStatus tStatus = CF_OK;

   if( -1 == ioctl( lFd, I2C_SLAVE, ucSlave ) )
   {
      perror( "Error setting target slave address" );
      tStatus = CF_FAILURE;
   }
   
   return( tStatus );
}


/*********************************************************************
 * Function: smbusReadSensors
 *
 * Read all the hardware sensors via smbus and fill in the
 * hardware info structure.  The smbus device should already
 * be open.
 *
 * Parameters:
 * lFd           File descriptor of smbus device file
 * ptHwmInfoRaw  Pointer to sensor info structure to fill
 *
 * Returns:
 * CF_OK on success, CF_FAILURE on error
 *
 *********************************************************************/
tCfStatus smbusReadSensors( int lFd, tHwmInfoRaw *ptHwmInfoRaw )
{
   tCfStatus tStatus = CF_OK;
   ui8 dataByte;

   do
   {
      /* Set the slave address with which we'll be communicating */
      tStatus = smbusSetSlave( lFd, HWMON_SLAVE );
      if( CF_FAILURE == tStatus )
      {
         tStatus = CF_FAILURE;
         break;
      }

      /*
      ** -jpa- translated from Windows driver
      **    Before we start retrieving data, we must tell the SIO to sample
      **    Vbat.  This is not a one time configuration thing, but must be 
      **    done every time.
      */
      tStatus = smbusReadByteData( lFd, READY_LOCK_START, &dataByte );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      dataByte |= SMSC_MONITOR_VBAT;

      tStatus = smbusWriteByteData( lFd, READY_LOCK_START, dataByte );
      if( CF_FAILURE == tStatus )
      {
         break;
      }
      /*-jpa-*/



      /* Read each sensor one after another.  It would be better if this was more
      ** automated and less hardcoded, but due to the way the sensor structure is
      ** designed, this is not easily done at this point. */

      tStatus = smbusReadByteData( lFd, P5VTR_REG, &( ptHwmInfoRaw->ucPlus5Vtr ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, VCCP_REG, &( ptHwmInfoRaw->ucVccp ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, VCC_REG, &( ptHwmInfoRaw->ucVcc ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, V2_IN_REG, &( ptHwmInfoRaw->ucV2_in ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, V1_IN_REG, &( ptHwmInfoRaw->ucV1_in ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, VTR_REG, &( ptHwmInfoRaw->ucVtr ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, RMT_DIODE1_REG, (ui8 *) &( ptHwmInfoRaw->cCpuTemp ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, AD_CONV1_REG, &( ptHwmInfoRaw->ucCpuTempLsb ) );
      ptHwmInfoRaw->ucCpuTempLsb &= 0x0f;
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, INT_TEMP_REG, (ui8 *) &( ptHwmInfoRaw->cInternalTemp ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, AD_CONV2_REG, &( ptHwmInfoRaw->ucInternalTempLsb ) );
      ptHwmInfoRaw->ucInternalTempLsb &= 0x0f;
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, RMT_DIODE2_REG, (ui8 *) &( ptHwmInfoRaw->cBoardTemp ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, AD_CONV1_REG, &( ptHwmInfoRaw->ucBoardTempLsb ) );
      ptHwmInfoRaw->ucBoardTempLsb >>= 4;
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, FANTACH1_LSB_REG, &( ptHwmInfoRaw->ucCpuFanSpeedLsb ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, FANTACH1_MSB_REG, &( ptHwmInfoRaw->ucCpuFanSpeed ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, FANTACH2_LSB_REG, &( ptHwmInfoRaw->ucRFan1SpeedLsb ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, FANTACH2_MSB_REG, &( ptHwmInfoRaw->ucRFan1Speed ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, FANTACH3_LSB_REG, &( ptHwmInfoRaw->ucRFan2SpeedLsb ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      tStatus = smbusReadByteData( lFd, FANTACH3_MSB_REG, &( ptHwmInfoRaw->ucRFan2Speed ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }

      /*
      ** -jpa- translated from Windows driver
      **    Vbat is not valid until SMSC_MONITOR_VBAT has been turned off.
      */
      do {
         tStatus = smbusReadByteData( lFd, READY_LOCK_START, &dataByte );
         if( CF_FAILURE == tStatus )
         {
            break;
         }
      } while ( dataByte & SMSC_MONITOR_VBAT );
      if( CF_FAILURE == tStatus )
      {
         break;
      }
      
      tStatus = smbusReadByteData( lFd, VBAT_REG, &( ptHwmInfoRaw->ucVbat ) );
      if( CF_FAILURE == tStatus )
      {
         break;
      }
      /*-jpa-*/

   } while( false );   

   return( tStatus );
}


