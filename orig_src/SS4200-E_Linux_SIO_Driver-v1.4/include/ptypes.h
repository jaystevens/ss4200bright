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

#ifndef __PTYPES_H__
#define __PTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
** Types used to mantain processor
** independence.
*/
typedef unsigned char       ui8;
typedef unsigned short      ui16;
typedef unsigned int        ui32;
typedef unsigned long long  ui64;

typedef char                i8;
typedef signed short        i16;
typedef signed int          i32;
typedef signed long long    i64;

typedef volatile unsigned char       io8;
typedef volatile unsigned short      io16;
typedef volatile unsigned int        io32;
typedef volatile unsigned long long  io64;

typedef float  f32;
typedef double f64;

#ifndef __cplusplus
#ifndef false
   #define false ((tCfBool)0)
   #define true  ((tCfBool)1)
#endif
#endif

typedef ui8 tCfBool;

#ifdef __cplusplus
}
#endif

#endif /* __PTYPES_H__ */

