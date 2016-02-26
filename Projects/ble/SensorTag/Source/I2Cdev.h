// I2Cdev library collection - Main I2C device class header file
// Abstracts bit and byte I2C R/W functions into a convenient class
// 6/9/2012 by Jeff Rowberg <jeff@rowberg.net>
//
// Changelog:
//     2012-06-09 - fix major issue with reading > 32 bytes at a time with Arduino Wire
//                - add compiler warnings when using outdated or IDE or limited I2Cdev implementation
//     2011-11-01 - fix write*Bits mask calculation (thanks sasquatch @ Arduino forums)
//     2011-10-03 - added automatic Arduino version detection for ease of use
//     2011-10-02 - added Gene Knight's NBWire TwoWire class implementation with small modifications
//     2011-08-31 - added support for Arduino 1.0 Wire library (methods are different from 0.x)
//     2011-08-03 - added optional timeout parameter to read* methods to easily change from default
//     2011-08-02 - added support for 16-bit registers
//                - fixed incorrect Doxygen comments on some methods
//                - added timeout value for read operations (thanks mem @ Arduino forums)
//     2011-07-30 - changed read/write function structures to return success or byte counts
//                - made all methods static for multi-device memory savings
//     2011-07-28 - initial release

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#ifndef _I2CDEV_H_
#define _I2CDEV_H_

#include "comdef.h"
typedef int8   int8_t;     //!< Signed 8 bit integer
typedef uint8   uint8_t;    //!< Unsigned 8 bit integer

typedef int16  int16_t;    //!< Signed 16 bit integer
typedef uint16  uint16_t;   //!< Unsigned 16 bit integer

typedef int32  int32_t;    //!< Signed 32 bit integer
typedef uint32   uint32_t;   //!< Unsigned 32 bit integer

//typedef unsigned char   bool;     //!< Boolean data type

typedef int8_t (* readBit_t)(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data);
typedef int8_t (* readBitW_t)(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t *data);
typedef int8_t (* readBits_t)(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data);
typedef int8_t (* readBitsW_t)(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t *data);
typedef int8_t (* readByte_t)(uint8_t devAddr, uint8_t regAddr, uint8_t *data);
typedef int8_t (* readWord_t)(uint8_t devAddr, uint8_t regAddr, uint16_t *data);
typedef int8_t (* readBytes_t)(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
typedef int8_t (* readWords_t)(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data);

typedef bool (* writeBit_t)(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data);
typedef bool (* writeBitW_t)(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t data);
typedef bool (* writeBits_t)(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
typedef bool (* writeBitsW_t)(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t data);
typedef bool (* writeByte_t)(uint8_t devAddr, uint8_t regAddr, uint8_t data);
typedef bool (* writeWord_t)(uint8_t devAddr, uint8_t regAddr, uint16_t data);
typedef bool (* writeBytes_t)(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
typedef bool (* writeWords_t)(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data);

typedef struct
{
  readBit_t    readBit;
  readBitW_t   readBitW;
  readBits_t   readBits;
  readBitsW_t  readBitsW;
  readByte_t   readByte;
  readWord_t   readWord;
  readBytes_t  readBytes;
  readWords_t  readWords;

  writeBit_t   writeBit;
  writeBitW_t  writeBitW;
  writeBits_t  writeBits;
  writeBitsW_t writeBitsW;
  writeByte_t  writeByte;
  writeWord_t  writeWord;
  writeBytes_t writeBytes;
  writeWords_t writeWords;
} i2cDevCBs_t;

//static uint16_t readTimeout = 1000;
extern i2cDevCBs_t gI2Cdev_t;
extern void I2CInit();
#endif /* _I2CDEV_H_ */