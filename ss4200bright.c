/*
 * SS4200-E Hardware API
 * Copyright (c) 2009, Intel Corporation.
 * Copyright (c) 2013, Jason Stevens
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
 * Author: Jason Stevens <jay@wizardofthenet.com>
 * 
 * Intel NAS/Home Server (ss4200) tool to set the brightness of the leds
 * using the smbus. this tool does not work if the lm85 hwmon kernel module is loaded.
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

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/** Defines **/
/* on 3.8.11, 3.9.1 kernels, the smbus device comes up as i2c-6,
 on Intel/EMC firmware it came up as i2c-0.
*/
#define SMBUS_DEV         "/dev/i2c-6"  // change me, aka lm85, aka emc6d103-i2c-6-2e
#define HWMON_SLAVE       ( 0x2e )

#define PWMA_CYCLE_REG    ( 0xa5 )
#define PWMA_FREQ_REG     ( 0xa7 )

static int                smbusFd = -1;

void ledPowerLevelSet( unsigned int ulPowerLevel );


int main(int argc, char* argv[])
{
    /* if no arguments given, print usage */
    if( argc < 2 )
    {
        printf("Usage: ss4200bright <number 0-100>\n");
        return;
    }

    /* convert 1st argument to an int */
    unsigned int ulLevel = strtol(argv[1], NULL, 10);
    printf("level: %d\n", ulLevel);
    
    /* check if less than 0 */
    if (ulLevel < 0)
    {
        perror("level must be >= 0");
        return;
    }
    /* check if greater than 100 */
    if (ulLevel > 100)
    {
        perror("level must be <= 100");
        return;
    }
    
    /* set power level */
    ledPowerLevelSet(ulLevel);
}

void ledPowerLevelSet( unsigned int ulPowerLevel )
{
    if( ulPowerLevel >= 0 && ulPowerLevel <= 100 )
    {
        /* open i2c smbus device */
        smbusFd = open( SMBUS_DEV, O_RDWR );
        if( -1 == smbusFd )
        {
            perror( "smbus failed to open, is i2c-dev module loaded?" );
            return;
        }
        
        /* Set destination slave address for write message */
        if( -1 == ioctl( smbusFd, I2C_SLAVE, HWMON_SLAVE ) )
        {
            perror( "Error setting smbus target slave address, is lm85 module loaded?" );
            return;
        }
        
        
        /* not sure what this smbus write does, but it was in the original Intel Code */
        /* original comment from <nas.c>:  temp until set in BIOS */
        struct i2c_smbus_ioctl_data cmd1;
        union i2c_smbus_data cmdData1;
        
        cmd1.read_write = I2C_SMBUS_WRITE;
        cmd1.command = PWMA_FREQ_REG;
        cmd1.size = I2C_SMBUS_BYTE_DATA;
        cmd1.data = &cmdData1;
        cmdData1.byte = 0x07;
        if( -1 == ioctl( smbusFd, I2C_SMBUS, &cmd1) )
        {
            perror( "Error on smbus write byte 1 (temp until set in BIOS)" );
            return;
        }

        
        /* Set the power level on smbus */
        struct i2c_smbus_ioctl_data cmd2;
        union i2c_smbus_data cmdData2;
        ulPowerLevel *= 2.55;      /* percent * resolution */
        
        cmd2.read_write = I2C_SMBUS_WRITE;
        cmd2.command = PWMA_CYCLE_REG;
        cmd2.size = I2C_SMBUS_BYTE_DATA;
        cmd2.data = &cmdData2;
        cmdData2.byte = ulPowerLevel;
        if( -1 == ioctl (smbusFd, I2C_SMBUS, &cmd2) )
        {
            perror( "Error on smbus write byte 2 (power level)" );
            return;
        }
        
        // Close smbus device file
        close( smbusFd );
        smbusFd = -1;
    }
}
        
         
        