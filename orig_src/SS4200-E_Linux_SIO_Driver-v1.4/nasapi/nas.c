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

/** Includes **/
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ptypes.h"
#include "nas.h"
#include "smbus.h"
#include "gpio.h"

/** Defines **/
//#define CF_DEBUG  (1)

static int     lNasSmbus = -1;
static int     lNasGpio  = -1;
static tCfBool bNasOpen  = false;


#ifdef CF_DEBUG
static i8 *cLedName [] =
{
   "HDD1_PRESENCE",
   "HDD1_FAULT",
   "HDD2_PRESENCE",
   "HDD2_FAULT",
   "HDD3_PRESENCE",
   "HDD3_FAULT",
   "HDD4_PRESENCE",
   "HDD4_FAULT",
   "SYSTEM_POWER",
   "SYSTEM_FAULT",
   "INVALID_LED"
};

static i8 *cLedState [] =
{
   "LED_OFF",
   "LED_ON",
   "LED_BLINK",
   "LED_INVALID_STATE"
};
#endif


/**
*** Close NAS device files
**/
void nasClose( void )
{
   close( lNasGpio );
   close( lNasSmbus );
	lNasGpio = -1;
	lNasSmbus = -1;
   bNasOpen = false;
}


/**
*** Open NAS device files
***
*** \return CF_OK on success, CF_FAILURE on error
**/
tCfStatus nasOpen( void )
{
   tCfStatus tStatus = CF_OK;

   lNasGpio = open( GPIO_DEV, O_WRONLY );
   if( -1 == lNasGpio )
   {
      perror( "NasGpio failed to open" );
      tStatus = CF_FAILURE;
   }
   else
   {
      lNasSmbus = open( SMBUS_DEV, O_RDWR );
      if( -1 == lNasSmbus )
      {
         perror( "NasSmbus failed to open" );
         tStatus = CF_FAILURE;
      }
      else
      {
         bNasOpen = true;
      }
   }

   return( tStatus );
}


/**
*** Read the hardware sensors and return human readable sensor values
***
*** \param ptHwmInfo Pointer to a tHwmInfo structure to be filled with
***                  sensor readings
***
*** \return CF_OK on success, CF_NOT_OPEN or CF_FAILURE on error
**/
tCfStatus nasHardwareMonitorInfoGet( tHwmInfo *ptHwmInfo )
{
   tHwmInfoRaw tHwmInfoRaw;
   double temp_value;
   ui16 fan_counts;
   tCfStatus tStatus = CF_OK;

   if( true == bNasOpen )
   {
      /* Get the raw data */
      tStatus = nasHardwareMonitorInfoGetRaw( &tHwmInfoRaw, sizeof( tHwmInfoRaw ) );

      temp_value = (double)tHwmInfoRaw.cCpuTemp + ( (double)tHwmInfoRaw.ucCpuTempLsb * 0.0625 );
      ptHwmInfo->fCpuTemp = (float)temp_value;

      temp_value = (double)tHwmInfoRaw.cInternalTemp + ( (double)tHwmInfoRaw.ucInternalTempLsb * 0.0625 );
      ptHwmInfo->fInternalTemp = (float)temp_value;

      temp_value = (double)tHwmInfoRaw.cBoardTemp + ( (double)tHwmInfoRaw.ucBoardTempLsb * 0.0625 );
      ptHwmInfo->fBoardTemp = (float)temp_value;

      fan_counts = (ui16)tHwmInfoRaw.ucCpuFanSpeed << 8;
      fan_counts += tHwmInfoRaw.ucCpuFanSpeedLsb;
      if( 0xFFFF == fan_counts )
      {
         temp_value = 0;
      }
      else
      {
         temp_value = ( ( 90000 / (double)fan_counts ) * 60 ); 
      }
      ptHwmInfo->fCpuFanSpeed = (float)temp_value;

      fan_counts = (ui16)tHwmInfoRaw.ucRFan1Speed << 8;
      fan_counts += tHwmInfoRaw.ucRFan1SpeedLsb;
      if( 0xFFFF == fan_counts )
      {
         temp_value = 0;
      }
      else
      {
         temp_value = ( ( 90000 / (double)fan_counts ) * 60 );
      }
      ptHwmInfo->fRFan1Speed = (float)temp_value;

      fan_counts = (ui16)tHwmInfoRaw.ucRFan2Speed << 8;
      fan_counts += tHwmInfoRaw.ucRFan2SpeedLsb;
      if( 0xFFFF == fan_counts )
      {
         temp_value = 0;
      }
      else
      {
         temp_value = ( ( 90000 / (double)fan_counts ) * 60 );
      }
      ptHwmInfo->fRFan2Speed = (float)temp_value;

      // The following conversions are derived from the SMSC5027 datasheet table 28.5 "Voltage VS. Register Reading".
      // Adjustments are made for V1_in and V2_in for voltage dividers on the inputs per the HI main board schematic.
      // Voltage = (MAX VOLTAGE * 1000) * reading / (1000 * MAX # of counts)

      ptHwmInfo->fPlus5Vtr = (( 6660 * (float)tHwmInfoRaw.ucPlus5Vtr) / (1000 * 255)); // 6.66v max / 255 counts
      ptHwmInfo->fVccp =     (( 2988 * (float)tHwmInfoRaw.ucVccp) / (1000 * 255));    // 2.988v max / 255 counts
      ptHwmInfo->fVcc =      (( 4380 * (float)tHwmInfoRaw.ucVcc) / (1000 * 255));     // 4.38v max / 255 counts
      ptHwmInfo->fV1_in =    (( 2000 * (float)tHwmInfoRaw.ucV1_in) / (1000 * 255));   // 2.00v max / 255 counts (0.75 voltage divider)
      ptHwmInfo->fV2_in =    (( 15990 * (float)tHwmInfoRaw.ucV2_in) / (1000 * 255));  // 15.00v max / 255 counts (0.1 Voltage divider)
      ptHwmInfo->fVtr =      (( 4380 * (float)tHwmInfoRaw.ucVtr) / (1000 * 255));     // 4.38v max / 255 counts
      ptHwmInfo->fVbat =     (( 4380 * (float)tHwmInfoRaw.ucVbat) / (1000 * 255));    // 4.38v max / 255 counts
   }
   else
   {
      /* HardwareMonitor is not open */
      tStatus = CF_NOT_OPEN;
   }

   return( tStatus );
}


/**
*** Get raw hardware sensor readings
***
*** \param ptHwmInfoRaw Pointer to a tHwmInfoRaw structure to be filled with
***                     sensor readings
*** \param ulSize       Size of available storage at ptHwmInfoRaw
***
*** \return CF_OK on success, CF_NOT_OPEN or CF_FAILURE on error
**/
tCfStatus nasHardwareMonitorInfoGetRaw( tHwmInfoRaw *ptHwmInfoRaw, ui32 ulSize )
{
   tCfStatus tStatus = CF_OK;

   if( true == bNasOpen )
   {
      if( ulSize >= sizeof( tHwmInfoRaw ) )
      {
         tStatus = smbusReadSensors( lNasSmbus, ptHwmInfoRaw );
      }
      else
      {
         /* size mismatch */
         tStatus = CF_FAILURE;
      }
   }
   else
   {
      /* HardwareMonitor is not open */
      tStatus = CF_NOT_OPEN;
   }

   return( tStatus );
}


/**
*** Get LED power level
***
*** \param ulPowerLevel Level of power going to the LEDs, from 0 to 100
***
*** \return CF_OK on success, CF_NOT_OPEN or CF_FAILURE on error
**/
tCfStatus nasLedPowerLevelSet( ui32 ulPowerLevel )
{
   tCfStatus tStatus = CF_OK;

   if( true == bNasOpen )
   {
      if( ulPowerLevel >= 0 && ulPowerLevel <= 100 )
      {
#ifdef CF_DEBUG
         printf( "power level = %i\n", ulPowerLevel );
#endif
         /* Set destination slave address for write message */
         tStatus = smbusSetSlave( lNasSmbus, HWMON_SLAVE );
         if( CF_OK == tStatus )
         {
            /* Set the power level */
            tStatus = smbusWriteByteData( lNasSmbus, PWMA_FREQ_REG, 0x07 );  //  temp until set in BIOS
            ulPowerLevel *= 2.55;      /* percent * resolution */
            tStatus = smbusWriteByteData( lNasSmbus, PWMA_CYCLE_REG, (ui8)ulPowerLevel );
         }
      }
      else 
      {
         /* Invalid powerLevel value */
         tStatus = CF_FAILURE;
      }
   }
   else
   {
      /* LED driver is not open */
      tStatus = CF_NOT_OPEN;
   }

   return( tStatus );
}


/**
*** Set the on/off/blink state of an LED
***
*** \param tLed      Enumeration specifying the LED whose state we're setting
*** \param tLedState Enumeration specifying what LED state to set
***
*** \return CF_OK on success, CF_NOT_OPEN or CF_FAILURE on error
**/
tCfStatus nasLedStateSet( tLeds tLed, tLedStates tLedState )
{
   tCfStatus tStatus = CF_OK;

   if( true == bNasOpen )
   {
      /* Check the LED value */
      if( ( tLed >= HDD1_PRESENCE ) && ( tLed < INVALID_LED ) )
      {
         /* Check the LED state */
         if( ( tLedState >= LED_OFF ) && ( tLedState < LED_INVALID_STATE ) )
         {
#ifdef CF_DEBUG
            printf( "%s state = %s\n", cLedName[tLed], cLedState[tLedState] );
#endif
            /* Set the LED state */
            tStatus = setGpioLedState( lNasGpio, tLed, tLedState );
         }
         else 
         {
            /* Invalid tLedState */
            tStatus = CF_FAILURE;
         }
      }
      else 
      {
         /* Invalid tLed */
         tStatus = CF_FAILURE;
      }
   }
   else
   {
      /* LED driver is not open */
      tStatus = CF_NOT_OPEN;
   }

   return( tStatus );
}


/**
*** Set a callback to monitor the state of the recovery button
***
*** \param ulMilliseconds Time delay between recovery button checks
*** \param pCallback      Function to call if the recovery button state changes
***
*** \return CF_OK on success, CF_NOT_OPEN or CF_FAILURE on error
*** \note Functionality is currently unimplemented
**/
tCfStatus nasSysRecoveryButtonMonitor( ui32 ulMilliseconds, void( *pCallback )( void ) )
{
   tCfStatus tStatus = CF_OK;

   if ( true == bNasOpen )
   {
      /* Temp values 0.5 second to 20 seconds */
      if ( ulMilliseconds >= 500 && ulMilliseconds <= 20000 )  
      {
         /* Callback is set */
         if ( NULL != pCallback )  
         {
#ifdef CF_DEBUG
            printf( "Recovery button time = %i milliseconds\n", ulMilliseconds );
#endif
         }
         else
         {
            /* Invalid callback value */
            tStatus = CF_FAILURE;
         }
      }
      else
      {
         /* Invalid milliseconds value */
         tStatus = CF_FAILURE;
      }
   }
   else
   {
      /* sysRecoveryButton is not open */
      tStatus = CF_NOT_OPEN;
   }

   return( tStatus );
}


/**
*** Read the state of the recovery button
***
*** \param pulButton Pointer to an ui32 that will be filled with the
***                  recovery button state
***
*** \return CF_OK on success, CF_NOT_OPEN or CF_FAILURE on error
**/
tCfStatus nasSysRecoveryButtonRead( ui32 *pulButton )
{
   tCfStatus tStatus = CF_OK;

   if( true == bNasOpen )
   {
      tStatus = getGpioRecoveryButton( lNasGpio, pulButton );
   }
   else
   {
      /* sysRecoveryButton is not open */
      tStatus = CF_NOT_OPEN;
   }

   return( tStatus );
}


/**
*** Set the timeout on the watchdog timer
***
*** \param ulSeconds Watchdog timer value to set
***
*** \return CF_OK on success, CF_NOT_OPEN or CF_FAILURE on error
**/
tCfStatus nasWatchdogSet( ui32 ulSeconds )
{
   tCfStatus tStatus = CF_OK;

   if( true == bNasOpen )
   {
      if( ( ulSeconds >= 0 ) && ( ulSeconds <= 255 ) )
      {
#ifdef CF_DEBUG
         printf( "watchdog time = %i seconds\n", ulSeconds );
#endif
         tStatus = setGpioWatchdog( lNasGpio, ulSeconds );
      }
      else 
      {
         /* Invalid watchdog value */
         tStatus = CF_FAILURE;
      }
   }
   else
   {
      /* Watchdog is not open */
      tStatus = CF_NOT_OPEN;
   }

   return( tStatus );
}

