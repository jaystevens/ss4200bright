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

#ifndef _NAS_H_
#define _NAS_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Includes **/

#include "ptypes.h"

/** Defines **/

typedef enum
{
   HDD1_PRESENCE,
   HDD1_FAULT,
   HDD2_PRESENCE,
   HDD2_FAULT,
   HDD3_PRESENCE,
   HDD3_FAULT,
   HDD4_PRESENCE,
   HDD4_FAULT,
   SYSTEM_POWER,
   SYSTEM_FAULT,
   INVALID_LED
} tLeds;


typedef enum
{
   LED_OFF,
   LED_ON,
   LED_BLINK,
   LED_INVALID_STATE
} tLedStates;

typedef enum
{
   CF_OK,
   CF_FAILURE,
   CF_NOT_OPEN
} tCfStatus;


typedef struct
{
   i8	cCpuTemp;              // raw value read from HWM reg 025h, Remote Diode 1 Temp Reading/PECI
   ui8	ucCpuTempLsb;	   	// raw value of bits 0 – 3 read from HWM reg 085h A/D Converter LSbs Reg 1
   i8	cInternalTemp;		// raw value read from HWM reg 026h, Internal Temp Reading
   ui8	ucInternalTempLsb;	// raw value of bits 0 – 3 read from HWM reg 086h, A/D Converter LSbs Reg 2
   i8	cBoardTemp;	   	// raw value read from HWM reg 027h, Remote Diode 2 Temp Reading
   ui8	ucBoardTempLsb;		// raw value of bits 4 – 7 read from HWM reg 085h, A/D Converter LSbs Reg 1
   ui8	ucCpuFanSpeed;    	// raw value read from HWM reg 029h FANTACH1 Reading MSB
   ui8	ucCpuFanSpeedLsb; 	// raw value read from HWM reg 028h FANTACH1 Reading LSB
   ui8	ucRFan1Speed;	   	// raw value read from HWM reg 02bh FANTACH2 Reading MSB
   ui8	ucRFan1SpeedLsb;	// raw value read from HWM reg 02ah FANTACH2 Reading LSB
   ui8	ucRFan2Speed;	   	// raw value read from HWM reg 02dh FANTACH3 Reading MSB
   ui8	ucRFan2SpeedLsb;	// raw value read from HWM reg 02ch FANTACH3 Reading LSB
   ui8	ucPlus5Vtr;		   	// .02604167 volt increments 0v – 6.64v,  V_5P0_STBY/G 
   ui8	ucVccp;		      	// .01171875 volt increments 0v – 3.00v,  VCCP  
   ui8	ucVcc;			   	// .01718750 volt increments 0v – 4.38v,  VCC 
   ui8	ucV1_in;		    // .005859375 volt increments 0v – 1.50v, MON_V_1P5_CORE 
   ui8	ucV2_in;		    // .0625 volt increments 0v – 15.94v,     MON_12V 
   ui8	ucVtr;			   	// .01718750 volt increments 0v – 4.38v,  V_3P3_STBY\G 
   ui8	ucVbat;		      	// .01718750 volt increments 0v – 4.38v,  V_3P0_BAT_VREG 
} tHwmInfoRaw;


typedef struct
{
   float	fCpuTemp;		// CPU temp -127C to 127C
   float	fInternalTemp;	// Internal Temp -127C to 127C
   float	fBoardTemp;	    // Board Temp -127C to 127C
   float	fCpuFanSpeed;   // CPU Fan speed in RPM
   float	fRFan1Speed;	// Rear Fan 1 speed in RPM
   float	fRFan2Speed;	// Rear Fan 2 speed in RPM
   float	fPlus5Vtr;		//  0v – 6.66v,  +5VTR
   float	fVccp;		    //  0v – 3.00v,  VCCP 
   float	fVcc;			//  0v – 4.38v,  VCC
   float	fV1_in;		    //  0v – 1.50v,  +1.5Vin
   float	fV2_in;		    //  0v – 15.94v, +12Vin 
   float	fVtr;			//  0v – 4.38v,  VTR
   float	fVbat;		    //  0v – 4.38v,  VBAT
} tHwmInfo;

/** function Protypes **/

extern void      nasClose( void );
extern tCfStatus nasOpen( void );
extern tCfStatus nasLedPowerLevelSet( ui32 ulPowerLevel );
extern tCfStatus nasLedStateSet( tLeds tLed, tLedStates tLedState );

extern tCfStatus nasSysRecoveryButtonMonitor( ui32 ulMilliseconds, void( *pCallback )( void ) );
extern tCfStatus nasSysRecoveryButtonRead( ui32 *pulButton );

extern tCfStatus nasHardwareMonitorInfoGet( tHwmInfo *ptHwmInfo );
extern tCfStatus nasHardwareMonitorInfoGetRaw( tHwmInfoRaw *ptHwmInfoRaw, ui32 ulSize );

extern tCfStatus nasWatchdogSet( ui32 ulSeconds ); 

#ifdef __cplusplus
}
#endif

#endif /* _NAS_H_ */
