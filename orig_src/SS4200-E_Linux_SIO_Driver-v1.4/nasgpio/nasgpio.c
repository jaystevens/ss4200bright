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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/cdev.h>

#include <asm/uaccess.h>   /* copy_*_user */

#include "nas.h"
#include "gpio.h"


MODULE_AUTHOR( "Rodney Girod <rgirod@confocus.com>" );
MODULE_DESCRIPTION( "Intel NAS/Home Server ICH7 GPIO Driver" );
MODULE_LICENSE( "GPL" );

#define NAME         "nasGpio"

/*
** ICH7 LPC/GPIO PCI Config register offsets
*/
#define  PMBASE      0x040
#define  GPIO_BASE   0x048
#define  GPIO_CTRL   0x04c
#define  GPIO_EN     0x010

/*
** The ICH7 GPIO register block is 64 bytes in size.
*/
#define ICH7_GPIO_SIZE   64

/*
** Define register offsets within the ICH7 register block.
*/
#define   GPIO_USE_SEL   0x000
#define   GP_IO_SEL      0x004
#define   GP_LVL         0x00c
#define   GPO_BLINK      0x018
#define   GPI_INV        0x030
#define   GPIO_USE_SEL2  0x034
#define   GP_IO_SEL2     0x038
#define   GP_LVL2        0x03c

/*
** ICH7 GPIO bit assignments
*/
#define HDD1_GREEN      0x00000001      // GPIO0
#define HDD1_AMBER      0x00000002      // GPIO1
#define HDD2_GREEN      0x00000004      // GPIO2
#define HDD2_AMBER      0x00000008      // GPIO3
#define HDD3_GREEN      0x00000010      // GPIO4
#define HDD3_AMBER      0x00000020      // GPIO5
#define HDD4_GREEN      0x00000040      // GPIO6
#define HDD4_AMBER      0x00000080      // GPIO7
#define SYSS_GREEN      0x08000000      // GPIO27
#define SYSS_AMBER      0x10000000      // GPIO28
#define ALL_NAS_LED     0x180000ff

#define NAS_RECOVERY    0x00000400      // GPIO10

/*
** SMSC SCH5027D Config Register offsets
*/
#define  SMSC_CONFIG_PORT  0x004e
#define  SMSC_INDEX_PORT   0x004e
#define  SMSC_DATA_PORT    0x004f

#define  SMSC_LOGICAL_DEVICE 0x007
#define  SMSC_DEVICE_ID    0x020
#define  SMSC_BASE_ADDR_HI 0x060
#define  SMSC_BASE_ADDR_LO 0x061

/*
** SMSC SCH5027D Run-time register offsets
*/
#define  SMSC_RT_GP60      0x0047
#define  SMSC_WDT_TIMEOUT  0x0065
#define  SMSC_WDT_VAL      0x0066
#define  SMSC_WDT_CFG      0x0067
#define  SMSC_WDT_CTRL     0x0068

#define SMSC_RT_GP60_WDT   0x00c          // Select WDT as alternate function of GP60
/*
** SMSC_WDT_TIMEOUT bit definitions
*/
#define  SMSC_WDT_TIMEOUT_SECONDS  0x080  // WDT operates in seconds

/*
** PCI ID of the Intel ICH7 LPC Device within which the GPIO block lives.
*/
static struct pci_device_id ich7LpcPciId[] =
{ 
   { PCI_DEVICE( PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH7_0 ) },
   { PCI_DEVICE( PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH7_1 ) },
   { PCI_DEVICE( PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH7_30 ) },
   { } /* NULL entry */
};

MODULE_DEVICE_TABLE( pci, ich7LpcPciId );

/*
** Kernel device registration data structures.
*/
static dev_t   devId;
static struct cdev nasGpioCdev;
static struct file_operations nasGpioFops;

/*
** Base I/O address assigned to the Power Management register block
*/
static ui32 gPmIoBase = 0;

/*
** Base I/O address assigned to the ICH7 GPIO register block
*/
static ui32 gGpioIoBase = 0;

/*
** Base I/O address assgned to the SCH5027 RunTime register block
** (Logical Device A) that contains the Watchdog Timer registers.
*/
static ui32 gWdtIoBase = 0;

/*
** When we successfully register a region, we are returned a resource.  We use these to
** identify which regions we need to release on our way back out.
*/
static struct resource * gpWdtResource = NULL;
static struct resource * gpGpioResource = NULL;

/*
** This table is used to translate the tLed enum to an actual GPIO register bit position.
*/
static const ui32  gpioBit[] = { HDD1_GREEN,
                                 HDD1_AMBER,
                                 HDD2_GREEN,
                                 HDD2_AMBER,
                                 HDD3_GREEN,
                                 HDD3_AMBER,
                                 HDD4_GREEN,
                                 HDD4_AMBER,
                                 SYSS_GREEN,
                                 SYSS_AMBER };


static int ich7GpioOpen( struct inode *inode, struct file *file )
{
   printk( KERN_ALERT NAME "  Driver Open request received\n" );
   return nonseekable_open( inode, 
                            file );
}

static int nasGpioIoctl( struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg )
{
   int err = 0;
   int retval = 0;
    
   /*
   ** extract the type and number bitfields, and don't decode
   ** wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
   */
   if( _IOC_TYPE( cmd ) != NAS_TYPE )
   {
      return( -ENOTTY );
   }

   /*
   ** the direction is a bitmask, and VERIFY_WRITE catches R/W
   ** transfers. `Type' is user-oriented, while
   ** access_ok is kernel-oriented, so the concept of "read" and
   ** "write" is reversed
   */
   if( _IOC_DIR( cmd ) & _IOC_READ )
   {
      err = !access_ok( VERIFY_WRITE, 
                        ( void __user * )arg, 
                        _IOC_SIZE( cmd ) );
   }
   else if( _IOC_DIR( cmd ) & _IOC_WRITE )
   {
      err = !access_ok( VERIFY_READ, 
                        ( void __user * )arg, 
                        _IOC_SIZE( cmd ) );
   }
   if( err )
   {
      return( -EFAULT );
   }

   switch(cmd) {

     case IOCTL_NAS_FPLED_SET_STATE:
      {
         SET_STATE_INFO ledReq;
         ui32 gpioLvl;
         ui32 gpioBlink;

         printk( KERN_ALERT NAME " *LED* Set received.\n" );
         
         retval = copy_from_user( &ledReq,
                                  ( const void * )arg,
                                  sizeof( SET_STATE_INFO ) );
         /*
         ** Error if we were unable to obtain all of the data.
         */
         if( retval )
         {
            return( -EFAULT );
         }


         /*
         ** If the LED identifier or the state is invalid, we will return an 
         ** invalid parameter.
         */
         if( ledReq.tLed >= INVALID_LED ||
             ledReq.tLedState >= LED_INVALID_STATE )
         {
            return( -EFAULT );
         }
         
         printk( KERN_ALERT NAME " tLed = %2d, tLedState = 0x%02x\n",
                 ledReq.tLed,
                 ledReq.tLedState );
                  
         /*
         ** Before we can update a particular LED, we must get the current
         ** setting of everything, so that we can modify just our selected
         ** output.
         */
         gpioLvl = inl( gGpioIoBase+GP_LVL );
         gpioBlink = inl( gGpioIoBase+GPO_BLINK );
         
         printk( KERN_ALERT NAME "  Before Update GP_LVL   = 0x%08x\n",
                 gpioLvl );
         printk( KERN_ALERT NAME "                GP_BLINK = 0x%08x\n",
                 gpioBlink );

         /*
         ** Now we will update the current settings according to the requested
         ** setting.
         */
         switch( ledReq.tLedState )
         {
         case LED_ON:
            /*
            ** ON requires setting the on bit and clearing the blink bit.
            */
            gpioLvl |= gpioBit[ ledReq.tLed ];
            gpioBlink &= ~( gpioBit[ ledReq.tLed ] );
            break;

         case LED_OFF:
            /*
            ** OFF requires clearing both the on and blink bits.
            */
            gpioLvl   &= ~( gpioBit[ ledReq.tLed ] );
            gpioBlink &= ~( gpioBit[ ledReq.tLed ] );
            break;

         case LED_BLINK:
            /*
            ** BLINK requires setting botyh the on and blink bits.
            */
            gpioLvl   |= gpioBit[ ledReq.tLed ];
            gpioBlink |= gpioBit[ ledReq.tLed ];
            break;

         case LED_INVALID_STATE:
         default:
            break;
         }

         /*
         ** Now that we have the new settings, we can write them all out.
         */
         outl( gpioLvl,
               gGpioIoBase+GP_LVL );
         outl( gpioBlink,
               gGpioIoBase+GPO_BLINK );
         
         printk( KERN_ALERT NAME "  After Update GP_LVL   = 0x%08x\n",
                 gpioLvl );
         printk( KERN_ALERT NAME "               GP_BLINK = 0x%08x\n",
                 gpioBlink );
         break;
      }

      case IOCTL_NAS_READ_RECOVERY_BUTTON:
      {
         ui32 recoveryState;

         printk( KERN_ALERT NAME " *Recovery Button* Read received.\n" );

         /*
         */
         recoveryState = NAS_RECOVERY & inl( gGpioIoBase+GP_LVL );
         recoveryState ^= NAS_RECOVERY;
         printk( KERN_ALERT NAME " Recovery Button status = 0x%08x.\n",
                 recoveryState );
         retval = __put_user( recoveryState,
                              ( ui32 __user * )arg );
         break;
      }

      case IOCTL_NAS_SET_WATCHDOG:
      {
         ui32  timeValue;
         
         retval = __get_user( timeValue,
                              ( ui32 __user * )arg );
         printk( KERN_ALERT NAME " Watchdog Set Request = %d.\n",
         timeValue );
         
         if( 256 > timeValue )
         {
            printk( KERN_ALERT NAME " Watchdog Set for %d seconds.\n",
                    timeValue );
            outb( (ui8)timeValue,
                  gWdtIoBase + SMSC_WDT_VAL );
         }
         else
         {
            retval = -ENOTTY;
         }
         break;
      }


      default:  /* redundant, as cmd was checked against MAXNR */
         return( -ENOTTY );
   }
   return retval;
}


/*
** Locate the SCH5027D run-time registers and initialize the watchdog timer.
*/
static int smsc5027dInit( void )
{
   ui32  status = -1;
   ui8   dataHi;
   ui8   dataLo;
   
   /*
   ** Attempt to put the SCH5027D into config mode.  By default its config
   ** registers should be located at 0x2e/0x2f.
   */
   printk( KERN_ALERT NAME " Sending config mode request to SIO.\n" );
   outb( 0x55,
         SMSC_CONFIG_PORT );
         
   /*
   ** Offset 0x07 of the Config registers is the device ID, and before we
   ** go any further, we will verify that this is our device (i.e. = 0x89)
   */
   outb( SMSC_DEVICE_ID,
         SMSC_INDEX_PORT );
   dataLo = inb( SMSC_DATA_PORT );
   printk( KERN_ALERT NAME " Device ID = 0x%02x.\n",
           dataLo );
   if( dataLo == 0x089 )
   {
      /*
      ** It appears that we have found the SCH5027D, so lets go find the
      ** run-time registers and reserve the watchdog offsets.
      **
      ** We start by selecting logical device A, which is where the run-time
      ** registers live.
      */
      outb( SMSC_LOGICAL_DEVICE,
            SMSC_INDEX_PORT );
      outb( 0x0a,
            SMSC_DATA_PORT );

      /*
      ** We now read the base I/O address assigned to the run-time register block.
      */      
      outb( SMSC_BASE_ADDR_HI,
            SMSC_INDEX_PORT );
      dataHi = inb( SMSC_DATA_PORT );
      outb( SMSC_BASE_ADDR_LO,
            SMSC_INDEX_PORT );
      dataLo = inb( SMSC_DATA_PORT );
      gWdtIoBase = ( dataHi << 8 ) + dataLo;
      printk( KERN_ALERT NAME " gWdtIoBase = 0x%08x.\n",
              gWdtIoBase );

      /*
      ** Insure that we have exclusive access to the Watchdog Timer registers.
      */
      printk( KERN_ALERT NAME ":Requesting use of WDT I/O Address Range\n" );
      gpWdtResource = request_region( gWdtIoBase + SMSC_WDT_TIMEOUT,
                                  4,
                                  NAME );
 
      if( NULL != gpWdtResource )
      {
         printk( KERN_ALERT NAME ":Use of WDT I/O Address Range granted\n" );
         status = 0;
      }
   }
   /*
   ** Return the SCH5027D to run mode.
   */
   outb( 0xaa, SMSC_CONFIG_PORT );
   
   /*
   ** We now need to initialize the watchdog.
   */
   if( gWdtIoBase )
   {

// This code cannot be run until after our board receives the required
// hardware modification.  Execution of this code on an unmodified ALPHA
// board results in an immediate reset/lockup of the system.
   
      outb( 0, gWdtIoBase + SMSC_WDT_CTRL );
      outb( SMSC_RT_GP60_WDT, gWdtIoBase + SMSC_RT_GP60 );
      outb( SMSC_WDT_TIMEOUT_SECONDS, gWdtIoBase + SMSC_WDT_TIMEOUT );
      outb( 0, gWdtIoBase + SMSC_WDT_VAL );
   }
   return( status );
}

/*
** Initialize the ICH7 GPIO registers for NAS usage.  The BIOS should have
** already taken care of this, but we will do so in a non destructive manner
** so that we have what we need whether the BIOS did it or not.
*/
static int ich7GpioInit( void )
{
   ui32   configData = 0;
   
   /*
   ** We need to enable all of the GPIO lines used by the NAS box, so we will
   ** read the current Use Selection and add our usage to it.  This should be
   ** benign with regard to the original BIOS configuration.
   */
   configData = inl( gGpioIoBase+GPIO_USE_SEL );
   printk( KERN_ALERT NAME "  Data read from GPIO_USE_SEL = 0x%08x\n",
           configData );
   configData |= ALL_NAS_LED + NAS_RECOVERY;
   outl( configData,
         gGpioIoBase+GPIO_USE_SEL );
   configData = inl( gGpioIoBase+GPIO_USE_SEL );
   printk( KERN_ALERT NAME "                 GPIO_USE_SEL = 0x%08x\n\n",
           configData );

   /*
   ** The LED GPIO outputs need to be configured for output, so we will insure
   ** that all LED lines are cleared for output and the RECOVERY line ready for
   ** input.  This too should be benign with regard to BIOS configuration.
   */
   configData = inl( gGpioIoBase+GP_IO_SEL );
   printk( KERN_ALERT NAME "  Data read from GP_IO_SEL    = 0x%08x\n",
           configData );
   configData &= ~ALL_NAS_LED;
   configData |= NAS_RECOVERY;
   outl( configData,
         gGpioIoBase+GP_IO_SEL );
   configData = inl( gGpioIoBase+GP_IO_SEL );
   printk( KERN_ALERT NAME "                 GP_IO_SEL    = 0x%08x\n\n",
           configData );

   /*
   ** In our final system, the BIOS will initialize the state of all of the
   ** LEDs.  For now, we turn them all off (or Low).
   */
   configData = inl( gGpioIoBase+GP_LVL );
   printk( KERN_ALERT NAME "  Data read from GP_LVL       = 0x%08x\n",
           configData );
//   configData &= ~ALL_NAS_LED;
//   outl( configData,
//         gGpioIoBase+GP_LVL );
//   configData = inl( gGpioIoBase+GP_LVL );
//   printk( KERN_ALERT NAME "                 GP_LVL       = 0x%08x\n\n",
//           configData );

   /*
   ** In our final system, the BIOS will initialize the blink state of all
   ** of the LEDs.  For now, we turn blink off for all of them.
   */
   configData = inl( gGpioIoBase+GPO_BLINK );
   printk( KERN_ALERT NAME "  Data read from GPO_BLINK    = 0x%08x\n",
           configData );
//   configData &= ~ALL_NAS_LED;
//   outl( configData,
//         gGpioIoBase+GPO_BLINK );
//   configData = inl( gGpioIoBase+GPO_BLINK );
//   printk( KERN_ALERT NAME "                 GPO_BLINK    = 0x%08x\n\n",
//           configData );

   /*
   ** At this moment, I am unsure if anything needs to happen with GPI_INV
   */
   configData = inl( gGpioIoBase+GPI_INV );
   printk( KERN_ALERT NAME "  Data read from GPI_INV      = 0x%08x\n",
           configData );

   return( 0 );
}

/**
*** Send a byte through the specified serial port.
***
*** \param Port Serial port to send byte to.
*** \param ucData Byte to send.
*** \return CF_OK if the function call was successful, or
***          CF_ERR_SERIAL_PORT if there was a failure.
***/
static void ich7LpcCleanup( void )
{
   /*
   ** If a major device id has been assigned, we must return it.
   */
   if( MAJOR( devId ) ) 
   {
      printk( KERN_ALERT NAME ":Returning device major id %d\n",
              MAJOR( devId ) );
      unregister_chrdev_region( devId,
                                1 );
      devId = 0;
   }
   
   /*
   ** If we were given exclusive use of the GPIO I/O Address range, we must return it.
   */
   if( gpGpioResource )
   {
      printk( KERN_ALERT NAME ":Releasing GPIO I/O Address Range\n" );
      release_region( gGpioIoBase,
                      ICH7_GPIO_SIZE );
      gpGpioResource = NULL;      
   }
   
   /*
   ** If we were given exclusive use of the Watchdog I/O Address range, we must return it.
   */
   if( gpWdtResource )
   {
      printk( KERN_ALERT NAME ":Releasing Watchdog I/O Address Range\n" );
      release_region( gWdtIoBase + SMSC_WDT_TIMEOUT,
                      4 );
      gpWdtResource = NULL;
   }
}

/*
** The OS has determined that the LPC of the Intel ICH7 Southbridge is present
** so we can retrive the required operational information and prepare the GPIO.
*/
static int ich7LpcProbe( struct pci_dev *dev, const struct pci_device_id *id )
{
   int   status = 0;
   ui32   gc = 0;

   while( 1 )
   {
      printk( KERN_ALERT NAME ":Entering ich7LpcProbe\n" );
      /*
      ** We must enable a PCI device before we can use any other PCI functions.
      */
      pci_enable_device( dev );

      /*
      ** Read the LPC PMBASE PCI Config Register to locate the Power Management I/O Base Address.
      */
      status = pci_read_config_dword( dev,
                                      PMBASE,
                                      &gPmIoBase );
      if( 0 > status )
      {
         printk( KERN_ALERT NAME ":ERROR - Unable to read PMBASE.\n" );
         break;
      }
      printk( KERN_ALERT NAME ":    PMBASE = 0x%08x\n",
              gPmIoBase );
      gPmIoBase &= 0x00000ff80;

      /*
      ** Read the LPC GC PCI Config Register to insure that GPIO has been enabled.
      */
      status = pci_read_config_dword( dev,
                                      GPIO_CTRL,
                                      &gc );
      if( 0 > status )
      {
         printk( KERN_ALERT NAME ":ERROR - Unable to read GC.\n" );
         break;
      }
      printk( KERN_ALERT NAME ":        GC = 0x%08x\n",
              gc );
      if( !( GPIO_EN & gc ) )
      {
         status = -1;
         printk( KERN_ALERT NAME ":ERROR - The LPC GPIO Block has not been enabled.\n" );
         break;
      }
   
      /*
      ** Read the LPC GPIOBASE PCI Config Register to locate the GPIO I/O Base Address.
      */
      status = pci_read_config_dword( dev,
                                      GPIO_BASE,
                                      &gGpioIoBase );
      if( 0 > status )
      {
         printk( KERN_ALERT NAME ":ERROR - Unable to read GPIOBASE.\n" );
         break;
      }
      printk( KERN_ALERT NAME ":GPIOBASE = 0x%08x\n",
              gGpioIoBase );
      gGpioIoBase &= 0x00000ffc0;
 
      /*
      ** Locate and Initialize the SCH5027D Watchdog.
      */
      status = smsc5027dInit();
      if( 0 > status )
      {
         break;
      }
   
      /*
      ** Insure that we have exclusive access to the GPIO I/O address range.
      */
      gpGpioResource = request_region( gGpioIoBase,
                                       ICH7_GPIO_SIZE,
                                       NAME );
      if( NULL == gpGpioResource )
      {
         printk( KERN_ALERT NAME ":ERROR Unable to register GPIO I/O address range.\n" );
         status = -1;
         break;
      }

      /*
      ** Initialize the GPIO for NAS/Home Server Use
      */
      ich7GpioInit( );

      /*
      ** Obtain a dynamic major device ID for our driver so that we may communicate with 
      ** the world.
      */
      status = alloc_chrdev_region( &devId, 
                                    0, 
                                    1,
                                    NAME );
      if( 0 > status )
      {
         printk( KERN_ALERT NAME ":ERROR - Unable to obtain MAJOR device id.\n" );
         break;
      }
        printk( KERN_ALERT NAME ":major device id = %d\n",
                MAJOR( devId ) );

      /*
      ** Prepare the cdev and fpos structures for our notification to the kernel.
      */
        memset( &nasGpioCdev,
                0,
                sizeof( nasGpioCdev ) );
        memset( &nasGpioFops,
                0,
                sizeof( nasGpioFops ) );
      nasGpioFops.owner   = THIS_MODULE;
      nasGpioFops.open   = ich7GpioOpen;
      nasGpioFops.ioctl = nasGpioIoctl;
      cdev_init( &nasGpioCdev, 
                 &nasGpioFops );
      nasGpioCdev.owner = THIS_MODULE;
      nasGpioCdev.ops = &nasGpioFops;
   
      /*
      ** All is good so we will attempt to register our interface with the kernel so 
      ** that operations may begin.
      */
      status = cdev_add( &nasGpioCdev, 
                         devId, 
                         1 );
      if( 0 > status )
      {
         printk( KERN_NOTICE NAME ":ERROR %d adding nasGpio\n", 
                 status );
      }
      break;
   }
   
   /*
   ** If we had a problem with getting everything ready, we need to return to
   ** the kernel all that was his before we exit.
   */
   if( 0 > status )
   {
      ich7LpcCleanup();
   }
   return( status );
}

/*
** The driver is on the way out, so we need to return any allocated resources.
*/
static void ich7LpcRemove( struct pci_dev *dev )
{
   /* clean up any allocated resources and stuff here.
    * like call release_region();
    */
   printk( KERN_ALERT NAME ":Removing PCI device\n" );
   cdev_del( &nasGpioCdev );
   ich7LpcCleanup();
}

/*
** pci_driver structure passed to the PCI modules
*/
static struct pci_driver pci_driver = {
   .name = NAME,
   .id_table = ich7LpcPciId,
   .probe = ich7LpcProbe,
   .remove = ich7LpcRemove,
};

/*
** module load/initialization
*/
static int __init nasGpioInit( void )
{
   printk( KERN_ALERT NAME ":Registering PCI driver\n" );
   return pci_register_driver( &pci_driver );
}

/*
** module unload
*/
static void __exit nasGpioExit( void )
{
   printk( KERN_ALERT NAME ":Unregistering PCI driver\n" );
   pci_unregister_driver( &pci_driver );
}

module_init( nasGpioInit );
module_exit( nasGpioExit );
