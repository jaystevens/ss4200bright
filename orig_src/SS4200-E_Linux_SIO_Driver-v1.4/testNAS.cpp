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

#ifdef WIN32
   #include "stdafx.h"
   #include <conio.h>
   #include <windows.h>
#else
   #include <unistd.h>
   #include <sys/select.h>
#endif                                  // WIN32

#include "nas.h"
#include <stdio.h>
#include <string.h>

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

// test functions
void atpLed001StateErrorTest( void );
void atpLed002StateTest( void );
void atpLed003PowerLevelErrorTest( void );
void atpLed004PowerLevelTest( void );

void atpSysr001SysRecoveryReadErrorTest( void );
void atpSysr002SysRecoveryReadTest( void );
void atpSysr003SysRecoveryMonitorErrorTest( void );
void atpSysr004SysRecoveryMonitorTest( void );

void atpHwm001HardwareMonitorErrorTest( void );
void atpHwm002HardwareMonitorTest( void );

void atpWdt001WatchdogErrorTest( void );
void atpWdt002WatchdogTimeoutTest( void );
void atpWdt003WatchdogEnableDisableTest( void );

// test helper functions
void callback( void );
int  cfGetNumInput( void );
void cfUSleep( ui32 ulMicroseconds );
void cfWait4Input( void );

void testNasLedPowerLevelSet( ui32 ulPowerLevel );
void testNasLedStateSet( tLeds tLed, tLedStates tLedState );
void testNasSysRecoveryMonitor( ui32 ulMilliseconds, void( *pCallback )( void ) );
void testNasHardwareMonitorInfoGet( tHwmInfo *ptHwmInfo );
void testNasHardwareMonitorInfoGetRaw( tHwmInfoRaw *ptHwmInfoRaw, ui32 ulSize );
void testNasWatchdogSet( ui32 ulSeconds );

#ifndef WIN32
   int  _kbhit(void);
#endif                                  // WIN32


#ifdef WIN32
   int _tmain(int argc, _TCHAR* argv[])
#else
   int main(int argc, char* argv[])
#endif                                 // WIN32
{
   i32 lCmdId = -1;
   
   while ( 0 != lCmdId )
   {
      printf( "0 = Exit\n" );
      printf( "1 = atpLed001StateErrorTest\n" );
      printf( "2 = atpLed002StateTest\n" );
      printf( "3 = atpLed003PowerLevelErrorTest\n" );
      printf( "4 = atpLed004PowerLevelTest\n" );
      printf( "5 = atpSysr001SysRecoveryReadErrorTest\n" );
      printf( "6 = atpSysr002SysRecoveryReadTest\n" );
      printf( "7 = atpSysr003SysRecoveryMonitorErrorTest\n" );
      printf( "8 = atpSysr004SysRecoveryMonitorTest\n" );
      printf( "9 = atpHwm001HardwareMonitorErrorTest\n" );
      printf( "10= atpHwm002HardwareMonitorTest\n" );
      printf( "11= atpWdt001WatchdogErrorTest\n" );
      printf( "12= atpWdt002WatchdogTimeoutTest\n" );
      printf( "13= atpWdt003WatchdogEnableDisableTest\n" );
      printf( "\n" );
      printf( "Enter the test number to execute and press <Enter>\n" );

      // wait for input
      lCmdId = cfGetNumInput();
          
      printf( "\n" );

      switch( lCmdId )
      {
         case 0:
            // exit testing
            break;
         case 1:
            atpLed001StateErrorTest();
            break;
         case 2:
            atpLed002StateTest();
            break;
         case 3:
            atpLed003PowerLevelErrorTest();
            break;
         case 4:
            atpLed004PowerLevelTest();
            break;
         case 5:
            atpSysr001SysRecoveryReadErrorTest();
            break;
         case 6:
            atpSysr002SysRecoveryReadTest();
            break;
         case 7:
            atpSysr003SysRecoveryMonitorErrorTest();
            break;
         case 8:
            atpSysr004SysRecoveryMonitorTest();
            break;
         case 9:
            atpHwm001HardwareMonitorErrorTest();
            break;
         case 10:
            atpHwm002HardwareMonitorTest();
            break;
         case 11:
            atpWdt001WatchdogErrorTest();
            break;
         case 12:
            atpWdt002WatchdogTimeoutTest();
            break;
         case 13:
            atpWdt003WatchdogEnableDisableTest();
            break;
      }
   }

   return (0);
}

void atpHwm001HardwareMonitorErrorTest( void )
{
   tHwmInfoRaw tHwmInfoRaw;

   // test condition where the driver has not been opened 
   printf( "Test condition where the driver has not been opened.\n" );
   printf( "Verify CF_NOT_OPEN status is displayed.\n" );

   nasClose();       // ensure that the driver is closed
   testNasHardwareMonitorInfoGetRaw( &tHwmInfoRaw, sizeof( tHwmInfoRaw ) );

   // open driver 
   nasOpen();

   // test invalid parameter condition  
   printf( "Test invalid parameter condition - data buffer too small.\n" );
   printf( "Verify CF_FAILURE status is displayed.\n" );
   testNasHardwareMonitorInfoGetRaw( &tHwmInfoRaw, (sizeof( tHwmInfoRaw ) -1 ));

   nasClose();
}

void atpHwm002HardwareMonitorTest( void )
{
   tHwmInfoRaw tHwmInfoRaw;
   tHwmInfo tHwmInfo;

   // open driver 
   nasOpen();

   nasHardwareMonitorInfoGetRaw( &tHwmInfoRaw, sizeof( tHwmInfoRaw ) );

   printf( "Verify that the data reported corresponds to the actual temp and fan speed.\n" );

   printf( "raw cCpuTemp            = %i\n", tHwmInfoRaw.cCpuTemp );
   printf( "raw ucCpuTempLsb        = %i\n", tHwmInfoRaw.ucCpuTempLsb );
   printf( "raw cInternalTemp       = %i\n", tHwmInfoRaw.cInternalTemp );
   printf( "raw ucInternalTempLsb   = %i\n", tHwmInfoRaw.ucInternalTempLsb );
   printf( "raw cBoardTemp          = %i\n", tHwmInfoRaw.cBoardTemp );
   printf( "raw ucBoardTempLsb      = %i\n", tHwmInfoRaw.ucBoardTempLsb );
   printf( "raw ucCpuFanSpeed       = %i\n", tHwmInfoRaw.ucCpuFanSpeed );
   printf( "raw ucCpuFanSpeedLsb    = %i\n", tHwmInfoRaw.ucCpuFanSpeedLsb );
   printf( "raw ucRFan1Speed        = %i\n", tHwmInfoRaw.ucRFan1Speed );
   printf( "raw ucRFan1SpeedLsb     = %i\n", tHwmInfoRaw.ucRFan1SpeedLsb );
   printf( "raw ucRFan2Speed        = %i\n", tHwmInfoRaw.ucRFan2Speed );
   printf( "raw ucRFan2SpeedLsb     = %i\n", tHwmInfoRaw.ucRFan2SpeedLsb );
   printf( "raw ucPlus5Vtr          = %i\n", tHwmInfoRaw.ucPlus5Vtr );
   printf( "raw ucVccp              = %i\n", tHwmInfoRaw.ucVccp );
   printf( "raw ucVcc               = %i\n", tHwmInfoRaw.ucVcc );
   printf( "raw ucV1_in             = %i\n", tHwmInfoRaw.ucV1_in );
   printf( "raw ucV2_in             = %i\n", tHwmInfoRaw.ucV2_in );
   printf( "raw ucVtr               = %i\n", tHwmInfoRaw.ucVtr );
   printf( "raw ucVbat              = %i\n", tHwmInfoRaw.ucVbat );
   printf( "\n" );

   nasHardwareMonitorInfoGet( &tHwmInfo );

   printf( "Cpu Temp           = %3.2f\370C / %3.2f\370F\n", tHwmInfo.fCpuTemp, (tHwmInfo.fCpuTemp * 9 / 5 + 32) );
   printf( "Internal Temp      = %3.2f\370C / %3.2f\370F\n", tHwmInfo.fInternalTemp, (tHwmInfo.fInternalTemp * 9 / 5 + 32)  );
   printf( "Remote Temp        = %3.2f\370C / %3.2f\370F\n", tHwmInfo.fBoardTemp, (tHwmInfo.fBoardTemp * 9 / 5 + 32)  );
   printf( "Cpu Fan speed      = %5.0f\n", tHwmInfo.fCpuFanSpeed );
   printf( "Fan1 speed         = %5.0f\n", tHwmInfo.fRFan1Speed );
   printf( "Fan2 speed         = %5.0f\n", tHwmInfo.fRFan2Speed );

   printf( "Plus5Vtr (+5VTR)   = %3.3f\n", tHwmInfo.fPlus5Vtr );
   printf( "Vccp               = %3.3f\n", tHwmInfo.fVccp );
   printf( "Vcc                = %3.3f\n", tHwmInfo.fVcc );
   printf( "V1_in    (+1.5Vin) = %3.3f\n", tHwmInfo.fV1_in );
   printf( "V2_in    (+12Vin)  = %3.3f\n", tHwmInfo.fV2_in );
   printf( "Vtr                = %3.3f\n", tHwmInfo.fVtr );
   printf( "Vbat               = %3.3f\n", tHwmInfo.fVbat );
   printf( "\n" );            // print a blank line for test separation

   // wait for confirmation, disregard input
   cfWait4Input();

   nasClose();
}

void atpLed001StateErrorTest( void )
{
   // test condition where the driver has not been opened 
   printf( "Test condition where the driver has not been opened.\n" );
   printf( "Verify CF_NOT_OPEN status is displayed.\n" );
   
   nasClose();       // ensure that the driver is closed
   testNasLedStateSet( HDD1_PRESENCE, LED_OFF );

   nasOpen();

   // test invalid LED
   printf( "Test invalid LED specified.\n" );
   printf( "Verify CF_FAILURE status is displayed.\n" );
   testNasLedStateSet( INVALID_LED, LED_OFF );

   // test invalid invalid state
   printf( "Test invalid state specified.\n" );
   printf( "Verify CF_FAILURE status is displayed.\n" );
   testNasLedStateSet( HDD1_PRESENCE, LED_INVALID_STATE );

   nasClose();
}

void atpLed002StateTest( void )
{
   nasOpen();

   // set all LEDs to the OFF state
   testNasLedStateSet( HDD1_PRESENCE, LED_OFF );
   testNasLedStateSet( HDD1_FAULT, LED_OFF );
   testNasLedStateSet( HDD2_PRESENCE, LED_OFF );
   testNasLedStateSet( HDD2_FAULT, LED_OFF );
   testNasLedStateSet( HDD3_PRESENCE, LED_OFF );
   testNasLedStateSet( HDD3_FAULT, LED_OFF );
   testNasLedStateSet( HDD4_PRESENCE, LED_OFF );
   testNasLedStateSet( HDD4_FAULT, LED_OFF );
   testNasLedStateSet( SYSTEM_POWER, LED_OFF );
   testNasLedStateSet( SYSTEM_FAULT, LED_OFF );

   // test all LEDs and states
   testNasLedStateSet( HDD1_PRESENCE, LED_ON );
   testNasLedStateSet( HDD1_PRESENCE, LED_BLINK );
   testNasLedStateSet( HDD1_PRESENCE, LED_OFF );

   testNasLedStateSet( HDD1_FAULT, LED_ON );
   testNasLedStateSet( HDD1_FAULT, LED_BLINK );
   testNasLedStateSet( HDD1_FAULT, LED_OFF );

   testNasLedStateSet( HDD2_PRESENCE, LED_ON );
   testNasLedStateSet( HDD2_PRESENCE, LED_BLINK );
   testNasLedStateSet( HDD2_PRESENCE, LED_OFF );

   testNasLedStateSet( HDD2_FAULT, LED_ON );
   testNasLedStateSet( HDD2_FAULT, LED_BLINK );
   testNasLedStateSet( HDD2_FAULT, LED_OFF );

   testNasLedStateSet( HDD3_PRESENCE, LED_ON );
   testNasLedStateSet( HDD3_PRESENCE, LED_BLINK );
   testNasLedStateSet( HDD3_PRESENCE, LED_OFF );

   testNasLedStateSet( HDD3_FAULT, LED_ON );
   testNasLedStateSet( HDD3_FAULT, LED_BLINK );
   testNasLedStateSet( HDD3_FAULT, LED_OFF );

   testNasLedStateSet( HDD4_PRESENCE, LED_ON );
   testNasLedStateSet( HDD4_PRESENCE, LED_BLINK );
   testNasLedStateSet( HDD4_PRESENCE, LED_OFF );

   testNasLedStateSet( HDD4_FAULT, LED_ON );
   testNasLedStateSet( HDD4_FAULT, LED_BLINK );
   testNasLedStateSet( HDD4_FAULT, LED_OFF );

   testNasLedStateSet( SYSTEM_POWER, LED_ON );
   testNasLedStateSet( SYSTEM_POWER, LED_BLINK );
   testNasLedStateSet( SYSTEM_POWER, LED_OFF );

   testNasLedStateSet( SYSTEM_FAULT, LED_ON );
   testNasLedStateSet( SYSTEM_FAULT, LED_BLINK );
   testNasLedStateSet( SYSTEM_FAULT, LED_OFF );

   nasClose();
}

void atpLed003PowerLevelErrorTest( void )
{
   // test condition where the driver has not been opened 
   printf( "Test condition where the driver has not been opened.\n" );
   printf( "Verify CF_NOT_OPEN status is displayed.\n" );

   nasClose();       // ensure that the driver is closed
   testNasLedPowerLevelSet( 100 );

   // open driver 
   nasOpen();

   // test invalid parameter condition  
   printf( "Test invalid parameter condition - power level > 100\n" );
   printf( "Verify CF_FAILURE status is displayed.\n" );
   testNasLedPowerLevelSet( 101 );

   nasClose();
}

void atpLed004PowerLevelTest( void )
{
   nasOpen();

   // set all LEDs to the ON state
   nasLedStateSet( HDD1_PRESENCE, LED_ON );
   nasLedStateSet( HDD1_FAULT, LED_ON );
   nasLedStateSet( HDD2_PRESENCE, LED_ON );
   nasLedStateSet( HDD2_FAULT, LED_ON );
   nasLedStateSet( HDD3_PRESENCE, LED_ON );
   nasLedStateSet( HDD3_FAULT, LED_ON );
   nasLedStateSet( HDD4_PRESENCE, LED_ON );
   nasLedStateSet( HDD4_FAULT, LED_ON );
   nasLedStateSet( SYSTEM_POWER, LED_ON );
   nasLedStateSet( SYSTEM_FAULT, LED_ON );

   // test power levels 
   testNasLedPowerLevelSet( 100 );
   testNasLedPowerLevelSet( 90 );
   testNasLedPowerLevelSet( 80 );
   testNasLedPowerLevelSet( 70 );
   testNasLedPowerLevelSet( 60 );
   testNasLedPowerLevelSet( 50 );
   testNasLedPowerLevelSet( 40 );
   testNasLedPowerLevelSet( 30 );
   testNasLedPowerLevelSet( 20 );
   testNasLedPowerLevelSet( 10 );
   testNasLedPowerLevelSet( 0 );
   testNasLedPowerLevelSet( 100 );

   // set all LEDs to the OFF state
   nasLedStateSet( HDD1_PRESENCE, LED_OFF );
   nasLedStateSet( HDD1_FAULT, LED_OFF );
   nasLedStateSet( HDD2_PRESENCE, LED_OFF );
   nasLedStateSet( HDD2_FAULT, LED_OFF );
   nasLedStateSet( HDD3_PRESENCE, LED_OFF );
   nasLedStateSet( HDD3_FAULT, LED_OFF );
   nasLedStateSet( HDD4_PRESENCE, LED_OFF );
   nasLedStateSet( HDD4_FAULT, LED_OFF );
   nasLedStateSet( SYSTEM_POWER, LED_OFF );
   nasLedStateSet( SYSTEM_FAULT, LED_OFF );

   nasClose();
}


void atpSysr001SysRecoveryReadErrorTest( void )
{
   tCfStatus tStatus;
   unsigned Button;

   // test condition where the driver has not been opened 
   printf( "Test condition where the driver has not been opened.\n" );
   printf( "Verify CF_NOT_OPEN status is displayed.\n" );

   nasClose();            // ensure that the driver is closed
   tStatus = nasSysRecoveryButtonRead( &Button );

   if ( CF_OK == tStatus )
   {
      printf( "Driver closed test failed - CF_OK received.\n" );
   }
   else
   {
      switch ( tStatus )
      {
         case CF_NOT_OPEN:
            printf( "nasSysRecoveryButtonRead failed, status = CF_NOT_OPEN\n\n" );
            break;
         default:
            printf( "nasSysRecoveryButtonRead failed, status = unknown\n\n" );
            break;
      }
   }

   // wait for confirmation, disregard input
   cfWait4Input();
}

void atpSysr002SysRecoveryReadTest( void )
{
   tCfStatus tStatus;
   unsigned Button;

   // open driver 
   nasOpen();

   printf( "Verify that the state reported corresponds\n" );
   printf( "to the actual system recovery button state.\n" );

   while(1)
   {
      tStatus = nasSysRecoveryButtonRead( &Button );

      if ( CF_OK == tStatus )
      {
         printf( "Recovery Button state = %i\n", Button );
         if( _kbhit())  // wait for keypress, disregard input
         {
            break;
         }

         cfUSleep( 100000 );     // 100000 microseconds (0.1 seconds)
      }
      else
      {
         switch ( tStatus )
         {
            case CF_NOT_OPEN:
               printf( "nasSysRecoveryButtonRead failed, status = CF_NOT_OPEN\n\n" );
               break;
            default:
               printf( "nasSysRecoveryButtonRead failed, status = unknown\n\n" );
               break;
         }
         break;
      }
   }
    
   // wait for confirmation, disregard input
   cfWait4Input();
   printf( "\n" );            // print a blank line for test separation

   // close driver 
   nasClose();
}

void atpSysr003SysRecoveryMonitorErrorTest( void )
{
   // test condition where the driver has not been opened 
   printf( "Test condition where the driver has not been opened.\n" );
   printf( "Verify CF_NOT_OPEN status is displayed.\n\n" );

   nasClose();            // ensure that the driver is closed

   testNasSysRecoveryMonitor( 2000, callback );

   // open driver 
   nasOpen();

   // test invalid parameter condition  
   printf( "Test invalid parameter condition - milliseconds < 500.\n" );
   printf( "Verify CF_FAILURE status is displayed.\n" );

   testNasSysRecoveryMonitor( 499, callback );

   // test invalid parameter condition  
   printf( "Test invalid parameter condition - milliseconds > 20000.\n" );
   printf( "Verify CF_FAILURE status is displayed.\n" );

   testNasSysRecoveryMonitor( 20001, callback );

   nasClose();
}

void atpSysr004SysRecoveryMonitorTest( void )
{
   // open driver 
   nasOpen();

   // test condition where the button is not pressed long enough 
   printf( "Activate the system recovery button for approximately 2 seconds and release.\n" );
   printf( "Verify the system recovery callback is not activated.\n" );
   printf( "Press <Enter> to continue the test.\n\n" );

   testNasSysRecoveryMonitor( 5000, callback );

   // test condition where the button is pressed long enough 
   printf( "Activate the system recovery button for approximately 6 seconds and release.\n" );
   printf( "Verify the system recovery callback is activated.\n" );
   printf( "Press <Enter> to end the test.\n\n" );

   testNasSysRecoveryMonitor( 5000, callback );

   nasClose();
}

void atpWdt001WatchdogErrorTest( void )
{
   // test condition where the driver has not been opened 
   printf( "Test condition where the driver has not been opened.\n" );
   printf( "Verify CF_NOT_OPEN status is displayed.\n\n" );

   nasClose();            // ensure that the driver is closed

   testNasWatchdogSet( 10 ); 

   // open driver 
   nasOpen();

   // test invalid parameter condition  
   printf( "Test invalid parameter condition - seconds > 255.\n" );
   printf( "Verify CF_FAILURE status is displayed.\n" );

   testNasWatchdogSet( 256 ); 

   nasClose();
}

void atpWdt002WatchdogTimeoutTest( void )
{
   int count = 0;

   nasOpen();

   // Verify the system will not timeout while being petted, 
   // and will timeout after the set time when the petting is stopped.
   printf( "Watchdog has been activated.\n" );
   printf( "Wait at least 60 seconds and verify the system does not reset.\n" );
   printf( "Press <Enter> to continue the test.\n\n" );

   while( !_kbhit() )
   {
      nasWatchdogSet( 30 ); 

      cfUSleep( 1000000 );       // 1000000 microseconds (1.0 seconds)

      count++;
      printf( "%i seconds\n", count );
   }

   printf( "***** Verify the system resets after approximately 30 seconds. *****\n\n" );

   // wait for confirmation, disregard input
   cfWait4Input();

   count = 0;

   while( !_kbhit() )
   {
      cfUSleep( 1000000 );       // 1000000 microseconds (1.0 seconds)

      count++;
      printf( "%i seconds\n", count );
   }


   nasClose();
}

void atpWdt003WatchdogEnableDisableTest( void )
{
   int count = 0;

   nasOpen();
  
   // test condition where the watchdog set for 30 second and disabled after 15 seconds 
   nasWatchdogSet( 30 ); 

   printf( "Watchdog has been activated for a 30 second timeout.\n" );
   printf( "After 15 seconds Watchdog will be disabled.\n" );

   while( !_kbhit() )
   {
      cfUSleep( 1000000 );       // 1000000 microseconds (1.0 seconds)

      count++;
      printf( "%i seconds\n", count );

      if (15 == count )
      {
         break;
      }
   }

   if (_kbhit())
   {
      count = getchar();         // discard the results
   }

   nasWatchdogSet( 0 );          // disable watchdog

   printf( "Watchdog has been disabled.\n" );
   printf( "Wait at least 60 seconds and verify the system does not reset.\n" );
   printf( "Press <Enter> to exit the test.\n\n" );

   count = 0;

   while( !_kbhit() )
   {
      cfUSleep( 1000000 );       // 1000000 microseconds (1.0 seconds)

      count++;
      printf( "%i seconds\n", count );
   }

   nasClose();
}

///////////////////////////////////////////////////////////////////////////////////////////////

void callback( void )
{
   printf( "\nThe system recovery callback has been activated.\n\n" );
}

int cfGetNumInput( void )
{
	size_t input;
   char string [256];

   fgets ( string, 255, stdin );
   input = strlen(string);
   if ( input > 1 )
   {
#ifdef WIN32
      sscanf_s( string, "%d", &input );
#else
      sscanf( string, "%d", &input );
#endif                                 // WIN32
   }
   else
   {
      input = -1;
   }

   return ( (int)input );
}


void cfUSleep( ui32 ulMicroseconds )
{
   // Windows sleep() is in milliseconds) Linux usleep is in microseconds
#ifdef WIN32
   Sleep(ulMicroseconds / 1000);       // Windows version
#else
   usleep( ulMicroseconds );           // Linux version
#endif                                 // WIN32
}


void cfWait4Input( void )
{
   int input = 0;

   // wait for confirmation, disregard input
   printf( "Press <Enter> to continue.\n\n" );
 
   input = cfGetNumInput();

}


void testNasLedStateSet( tLeds tLed, tLedStates tLedState )
{
   tCfStatus tStatus;

   // set the LED state
   printf( "%s state = %s\n", cLedName[tLed], cLedState[tLedState] );
   tStatus = nasLedStateSet( tLed, tLedState );
   
   if ( CF_OK == tStatus )
   {
      printf( "Verify that the led specified is in the state specified.\n" );
   }
   else
   {
      switch ( tStatus )
      {
         case CF_NOT_OPEN:
            printf( "nasLedStateSet failed, status = CF_NOT_OPEN\n\n" );
            break;
         case CF_FAILURE:
            printf( "nasLedStateSet failed, status = CF_FAILURE\n\n" );
            break;
         default:
            printf( "nasLedStateSet failed, status = unknown\n\n" );
            break;
      }
   }

   // wait for confirmation, disregard input
   cfWait4Input();
}

void testNasLedPowerLevelSet( ui32 ulPowerLevel )
{
   tCfStatus tStatus;

   // set the LED state
   printf( "power level = %i\n", ulPowerLevel );
   tStatus = nasLedPowerLevelSet( ulPowerLevel );
   
   if ( CF_OK == tStatus )
   {
      printf( "Verify that the led brightness corresponds to the power level set.\n" );
   }
   else
   {
      switch ( tStatus )
      {
         case CF_NOT_OPEN:
            printf( "nasLedStateSet failed, status = CF_NOT_OPEN\n\n" );
            break;
         case CF_FAILURE:
            printf( "nasLedStateSet failed, status = CF_FAILURE\n\n" );
            break;
         default:
            printf( "nasLedStateSet failed, status = unknown\n\n" );
            break;
      }
   }

   // wait for confirmation, disregard input
   cfWait4Input();
}

void testNasSysRecoveryMonitor( ui32 ulMilliseconds, void( *pCallback )( void ) )
{
   tCfStatus tStatus;

   // set the monitor time
   printf( "monitor time = %i milliseconds\n", ulMilliseconds );
   tStatus = nasSysRecoveryButtonMonitor( ulMilliseconds, pCallback );
   
   if ( CF_OK != tStatus )
   {
      switch ( tStatus )
      {
         case CF_NOT_OPEN:
            printf( "nasSysRecoveryButtonMonitor failed, status = CF_NOT_OPEN\n\n" );
            break;
         case CF_FAILURE:
            printf( "nasSysRecoveryButtonMonitor failed, status = CF_FAILURE\n\n" );
            break;
         default:
            printf( "nasSysRecoveryButtonMonitor failed, status = unknown\n\n" );
            break;
      }
   }

   // wait for confirmation, disregard input
   cfWait4Input();
}


void testNasHardwareMonitorInfoGet( tHwmInfo *ptHwmInfo )
{
   tCfStatus tStatus;

   tStatus = nasHardwareMonitorInfoGet( ptHwmInfo );
   
   if ( CF_OK != tStatus )
   {
      switch ( tStatus )
      {
         case CF_NOT_OPEN:
            printf( "nasHardwareMonitorInfoGet failed, status = CF_NOT_OPEN\n\n" );
            break;
         case CF_FAILURE:
            printf( "nasHardwareMonitorInfoGet failed, status = CF_FAILURE\n\n" );
            break;
         default:
            printf( "nasHardwareMonitorInfoGet failed, status = unknown\n\n" );
            break;
      }
   }

   // wait for confirmation, disregard input
   cfWait4Input();
}


void testNasHardwareMonitorInfoGetRaw( tHwmInfoRaw *ptHwmInfoRaw, ui32 ulSize )
{
   tCfStatus tStatus;

   tStatus = nasHardwareMonitorInfoGetRaw( ptHwmInfoRaw, ulSize );
   
   if ( CF_OK != tStatus )
   {
      switch ( tStatus )
      {
         case CF_NOT_OPEN:
            printf( "nasHardwareMonitorInfoGetRaw failed, status = CF_NOT_OPEN\n\n" );
            break;
         case CF_FAILURE:
            printf( "nasHardwareMonitorInfoGetRaw failed, status = CF_FAILURE\n\n" );
            break;
         default:
            printf( "nasHardwareMonitorInfoGetRaw failed, status = unknown\n\n" );
            break;
      }
   }

   // wait for confirmation, disregard input
   cfWait4Input();
}


void testNasWatchdogSet( ui32 ulSeconds )
{
   tCfStatus tStatus;

   // set the LED state
   printf( "watchdog time in seconds = %i\n", ulSeconds );
   tStatus = nasWatchdogSet( ulSeconds );
   
   if ( CF_OK != tStatus )
   {
      switch ( tStatus )
      {
         case CF_NOT_OPEN:
            printf( "nasWatchdogSet failed, status = CF_NOT_OPEN\n\n" );
            break;
         case CF_FAILURE:
            printf( "nasWatchdogSet failed, status = CF_FAILURE\n\n" );
            break;
         default:
            printf( "nasWatchdogSet failed, status = unknown\n\n" );
            break;
      }
   }

   // wait for confirmation, disregard input
   cfWait4Input();
}

#ifndef WIN32
int _kbhit(void)
{
   struct timeval tv;
   fd_set read_fd;

   tv.tv_sec=0;
   tv.tv_usec=0;
   FD_ZERO(&read_fd);
   FD_SET(0,&read_fd);

   if(select(1, &read_fd, NULL, NULL, &tv) == -1)
   return 0;

   if(FD_ISSET(0,&read_fd))
   return 1;

   return 0;
}
#endif                                  // WIN32
