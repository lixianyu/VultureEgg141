// I2Cdev library collection - MPU6050 I2C device class
// Based on InvenSense MPU-6050 register map document rev. 2.0, 5/19/2011 (RM-MPU-6000A-00)
// 10/3/2011 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     ... - ongoing debug release

// NOTE: THIS IS ONLY A PARIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY UNDERGOING ACTIVE
// DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES. PLEASE KEEP THIS IN MIND IF
// YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.

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

#ifndef _MPU6050_H_
#define _MPU6050_H_

#include "I2Cdev.h"
//#include <avr/pgmspace.h>

#define MPU6050_ADDRESS_AD0_LOW     0x68 // address pin low (GND), default for InvenSense evaluation board
#define MPU6050_ADDRESS_AD0_HIGH    0x69 // address pin high (VCC)
#define MPU6050_DEFAULT_ADDRESS     MPU6050_ADDRESS_AD0_LOW

#define MPU6050_RA_XG_OFFS_TC       0x00 //[7] PWR_MODE, [6:1] XG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU6050_RA_YG_OFFS_TC       0x01 //[7] PWR_MODE, [6:1] YG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU6050_RA_ZG_OFFS_TC       0x02 //[7] PWR_MODE, [6:1] ZG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU6050_RA_X_FINE_GAIN      0x03 //[7:0] X_FINE_GAIN
#define MPU6050_RA_Y_FINE_GAIN      0x04 //[7:0] Y_FINE_GAIN
#define MPU6050_RA_Z_FINE_GAIN      0x05 //[7:0] Z_FINE_GAIN
#define MPU6050_RA_XA_OFFS_H        0x06 //[15:0] XA_OFFS
#define MPU6050_RA_XA_OFFS_L_TC     0x07
#define MPU6050_RA_YA_OFFS_H        0x08 //[15:0] YA_OFFS
#define MPU6050_RA_YA_OFFS_L_TC     0x09
#define MPU6050_RA_ZA_OFFS_H        0x0A //[15:0] ZA_OFFS
#define MPU6050_RA_ZA_OFFS_L_TC     0x0B
#define MPU6050_RA_XG_OFFS_USRH     0x13 //[15:0] XG_OFFS_USR
#define MPU6050_RA_XG_OFFS_USRL     0x14
#define MPU6050_RA_YG_OFFS_USRH     0x15 //[15:0] YG_OFFS_USR
#define MPU6050_RA_YG_OFFS_USRL     0x16
#define MPU6050_RA_ZG_OFFS_USRH     0x17 //[15:0] ZG_OFFS_USR
#define MPU6050_RA_ZG_OFFS_USRL     0x18
#define MPU6050_RA_SMPLRT_DIV       0x19
#define MPU6050_RA_CONFIG           0x1A
#define MPU6050_RA_GYRO_CONFIG      0x1B
#define MPU6050_RA_ACCEL_CONFIG     0x1C
#define MPU6050_RA_FF_THR           0x1D
#define MPU6050_RA_FF_DUR           0x1E
#define MPU6050_RA_MOT_THR          0x1F
#define MPU6050_RA_MOT_DUR          0x20
#define MPU6050_RA_ZRMOT_THR        0x21
#define MPU6050_RA_ZRMOT_DUR        0x22
#define MPU6050_RA_FIFO_EN          0x23
#define MPU6050_RA_I2C_MST_CTRL     0x24
#define MPU6050_RA_I2C_SLV0_ADDR    0x25
#define MPU6050_RA_I2C_SLV0_REG     0x26
#define MPU6050_RA_I2C_SLV0_CTRL    0x27
#define MPU6050_RA_I2C_SLV1_ADDR    0x28
#define MPU6050_RA_I2C_SLV1_REG     0x29
#define MPU6050_RA_I2C_SLV1_CTRL    0x2A
#define MPU6050_RA_I2C_SLV2_ADDR    0x2B
#define MPU6050_RA_I2C_SLV2_REG     0x2C
#define MPU6050_RA_I2C_SLV2_CTRL    0x2D
#define MPU6050_RA_I2C_SLV3_ADDR    0x2E
#define MPU6050_RA_I2C_SLV3_REG     0x2F
#define MPU6050_RA_I2C_SLV3_CTRL    0x30
#define MPU6050_RA_I2C_SLV4_ADDR    0x31
#define MPU6050_RA_I2C_SLV4_REG     0x32
#define MPU6050_RA_I2C_SLV4_DO      0x33
#define MPU6050_RA_I2C_SLV4_CTRL    0x34
#define MPU6050_RA_I2C_SLV4_DI      0x35
#define MPU6050_RA_I2C_MST_STATUS   0x36
#define MPU6050_RA_INT_PIN_CFG      0x37
#define MPU6050_RA_INT_ENABLE       0x38
#define MPU6050_RA_DMP_INT_STATUS   0x39
#define MPU6050_RA_INT_STATUS       0x3A
#define MPU6050_RA_ACCEL_XOUT_H     0x3B
#define MPU6050_RA_ACCEL_XOUT_L     0x3C
#define MPU6050_RA_ACCEL_YOUT_H     0x3D
#define MPU6050_RA_ACCEL_YOUT_L     0x3E
#define MPU6050_RA_ACCEL_ZOUT_H     0x3F
#define MPU6050_RA_ACCEL_ZOUT_L     0x40
#define MPU6050_RA_TEMP_OUT_H       0x41
#define MPU6050_RA_TEMP_OUT_L       0x42
#define MPU6050_RA_GYRO_XOUT_H      0x43
#define MPU6050_RA_GYRO_XOUT_L      0x44
#define MPU6050_RA_GYRO_YOUT_H      0x45
#define MPU6050_RA_GYRO_YOUT_L      0x46
#define MPU6050_RA_GYRO_ZOUT_H      0x47
#define MPU6050_RA_GYRO_ZOUT_L      0x48
#define MPU6050_RA_EXT_SENS_DATA_00 0x49
#define MPU6050_RA_EXT_SENS_DATA_01 0x4A
#define MPU6050_RA_EXT_SENS_DATA_02 0x4B
#define MPU6050_RA_EXT_SENS_DATA_03 0x4C
#define MPU6050_RA_EXT_SENS_DATA_04 0x4D
#define MPU6050_RA_EXT_SENS_DATA_05 0x4E
#define MPU6050_RA_EXT_SENS_DATA_06 0x4F
#define MPU6050_RA_EXT_SENS_DATA_07 0x50
#define MPU6050_RA_EXT_SENS_DATA_08 0x51
#define MPU6050_RA_EXT_SENS_DATA_09 0x52
#define MPU6050_RA_EXT_SENS_DATA_10 0x53
#define MPU6050_RA_EXT_SENS_DATA_11 0x54
#define MPU6050_RA_EXT_SENS_DATA_12 0x55
#define MPU6050_RA_EXT_SENS_DATA_13 0x56
#define MPU6050_RA_EXT_SENS_DATA_14 0x57
#define MPU6050_RA_EXT_SENS_DATA_15 0x58
#define MPU6050_RA_EXT_SENS_DATA_16 0x59
#define MPU6050_RA_EXT_SENS_DATA_17 0x5A
#define MPU6050_RA_EXT_SENS_DATA_18 0x5B
#define MPU6050_RA_EXT_SENS_DATA_19 0x5C
#define MPU6050_RA_EXT_SENS_DATA_20 0x5D
#define MPU6050_RA_EXT_SENS_DATA_21 0x5E
#define MPU6050_RA_EXT_SENS_DATA_22 0x5F
#define MPU6050_RA_EXT_SENS_DATA_23 0x60
#define MPU6050_RA_MOT_DETECT_STATUS    0x61
#define MPU6050_RA_I2C_SLV0_DO      0x63
#define MPU6050_RA_I2C_SLV1_DO      0x64
#define MPU6050_RA_I2C_SLV2_DO      0x65
#define MPU6050_RA_I2C_SLV3_DO      0x66
#define MPU6050_RA_I2C_MST_DELAY_CTRL   0x67
#define MPU6050_RA_SIGNAL_PATH_RESET    0x68
#define MPU6050_RA_MOT_DETECT_CTRL      0x69
#define MPU6050_RA_USER_CTRL        0x6A
#define MPU6050_RA_PWR_MGMT_1       0x6B
#define MPU6050_RA_PWR_MGMT_2       0x6C
#define MPU6050_RA_BANK_SEL         0x6D
#define MPU6050_RA_MEM_START_ADDR   0x6E
#define MPU6050_RA_MEM_R_W          0x6F
#define MPU6050_RA_DMP_CFG_1        0x70
#define MPU6050_RA_DMP_CFG_2        0x71
#define MPU6050_RA_FIFO_COUNTH      0x72
#define MPU6050_RA_FIFO_COUNTL      0x73
#define MPU6050_RA_FIFO_R_W         0x74
#define MPU6050_RA_WHO_AM_I         0x75

#define MPU6050_TC_PWR_MODE_BIT     7
#define MPU6050_TC_OFFSET_BIT       6
#define MPU6050_TC_OFFSET_LENGTH    6
#define MPU6050_TC_OTP_BNK_VLD_BIT  0

#define MPU6050_VDDIO_LEVEL_VLOGIC  0
#define MPU6050_VDDIO_LEVEL_VDD     1

#define MPU6050_CFG_EXT_SYNC_SET_BIT    5
#define MPU6050_CFG_EXT_SYNC_SET_LENGTH 3
#define MPU6050_CFG_DLPF_CFG_BIT    2
#define MPU6050_CFG_DLPF_CFG_LENGTH 3

#define MPU6050_EXT_SYNC_DISABLED       0x0
#define MPU6050_EXT_SYNC_TEMP_OUT_L     0x1
#define MPU6050_EXT_SYNC_GYRO_XOUT_L    0x2
#define MPU6050_EXT_SYNC_GYRO_YOUT_L    0x3
#define MPU6050_EXT_SYNC_GYRO_ZOUT_L    0x4
#define MPU6050_EXT_SYNC_ACCEL_XOUT_L   0x5
#define MPU6050_EXT_SYNC_ACCEL_YOUT_L   0x6
#define MPU6050_EXT_SYNC_ACCEL_ZOUT_L   0x7

#define MPU6050_DLPF_BW_256         0x00
#define MPU6050_DLPF_BW_188         0x01
#define MPU6050_DLPF_BW_98          0x02
#define MPU6050_DLPF_BW_42          0x03
#define MPU6050_DLPF_BW_20          0x04
#define MPU6050_DLPF_BW_10          0x05
#define MPU6050_DLPF_BW_5           0x06

#define MPU6050_GCONFIG_FS_SEL_BIT      4
#define MPU6050_GCONFIG_FS_SEL_LENGTH   2

#define MPU6050_GYRO_FS_250         0x00
#define MPU6050_GYRO_FS_500         0x01
#define MPU6050_GYRO_FS_1000        0x02
#define MPU6050_GYRO_FS_2000        0x03

#define MPU6050_ACONFIG_XA_ST_BIT           7
#define MPU6050_ACONFIG_YA_ST_BIT           6
#define MPU6050_ACONFIG_ZA_ST_BIT           5
#define MPU6050_ACONFIG_AFS_SEL_BIT         4
#define MPU6050_ACONFIG_AFS_SEL_LENGTH      2
#define MPU6050_ACONFIG_ACCEL_HPF_BIT       2
#define MPU6050_ACONFIG_ACCEL_HPF_LENGTH    3

#define MPU6050_ACCEL_FS_2          0x00
#define MPU6050_ACCEL_FS_4          0x01
#define MPU6050_ACCEL_FS_8          0x02
#define MPU6050_ACCEL_FS_16         0x03

#define MPU6050_DHPF_RESET          0x00
#define MPU6050_DHPF_5              0x01
#define MPU6050_DHPF_2P5            0x02
#define MPU6050_DHPF_1P25           0x03
#define MPU6050_DHPF_0P63           0x04
#define MPU6050_DHPF_HOLD           0x07

#define MPU6050_TEMP_FIFO_EN_BIT    7
#define MPU6050_XG_FIFO_EN_BIT      6
#define MPU6050_YG_FIFO_EN_BIT      5
#define MPU6050_ZG_FIFO_EN_BIT      4
#define MPU6050_ACCEL_FIFO_EN_BIT   3
#define MPU6050_SLV2_FIFO_EN_BIT    2
#define MPU6050_SLV1_FIFO_EN_BIT    1
#define MPU6050_SLV0_FIFO_EN_BIT    0

#define MPU6050_MULT_MST_EN_BIT     7
#define MPU6050_WAIT_FOR_ES_BIT     6
#define MPU6050_SLV_3_FIFO_EN_BIT   5
#define MPU6050_I2C_MST_P_NSR_BIT   4
#define MPU6050_I2C_MST_CLK_BIT     3
#define MPU6050_I2C_MST_CLK_LENGTH  4

#define MPU6050_CLOCK_DIV_348       0x0
#define MPU6050_CLOCK_DIV_333       0x1
#define MPU6050_CLOCK_DIV_320       0x2
#define MPU6050_CLOCK_DIV_308       0x3
#define MPU6050_CLOCK_DIV_296       0x4
#define MPU6050_CLOCK_DIV_286       0x5
#define MPU6050_CLOCK_DIV_276       0x6
#define MPU6050_CLOCK_DIV_267       0x7
#define MPU6050_CLOCK_DIV_258       0x8
#define MPU6050_CLOCK_DIV_500       0x9
#define MPU6050_CLOCK_DIV_471       0xA
#define MPU6050_CLOCK_DIV_444       0xB
#define MPU6050_CLOCK_DIV_421       0xC
#define MPU6050_CLOCK_DIV_400       0xD
#define MPU6050_CLOCK_DIV_381       0xE
#define MPU6050_CLOCK_DIV_364       0xF

#define MPU6050_I2C_SLV_RW_BIT      7
#define MPU6050_I2C_SLV_ADDR_BIT    6
#define MPU6050_I2C_SLV_ADDR_LENGTH 7
#define MPU6050_I2C_SLV_EN_BIT      7
#define MPU6050_I2C_SLV_BYTE_SW_BIT 6
#define MPU6050_I2C_SLV_REG_DIS_BIT 5
#define MPU6050_I2C_SLV_GRP_BIT     4
#define MPU6050_I2C_SLV_LEN_BIT     3
#define MPU6050_I2C_SLV_LEN_LENGTH  4

#define MPU6050_I2C_SLV4_RW_BIT         7
#define MPU6050_I2C_SLV4_ADDR_BIT       6
#define MPU6050_I2C_SLV4_ADDR_LENGTH    7
#define MPU6050_I2C_SLV4_EN_BIT         7
#define MPU6050_I2C_SLV4_INT_EN_BIT     6
#define MPU6050_I2C_SLV4_REG_DIS_BIT    5
#define MPU6050_I2C_SLV4_MST_DLY_BIT    4
#define MPU6050_I2C_SLV4_MST_DLY_LENGTH 5

#define MPU6050_MST_PASS_THROUGH_BIT    7
#define MPU6050_MST_I2C_SLV4_DONE_BIT   6
#define MPU6050_MST_I2C_LOST_ARB_BIT    5
#define MPU6050_MST_I2C_SLV4_NACK_BIT   4
#define MPU6050_MST_I2C_SLV3_NACK_BIT   3
#define MPU6050_MST_I2C_SLV2_NACK_BIT   2
#define MPU6050_MST_I2C_SLV1_NACK_BIT   1
#define MPU6050_MST_I2C_SLV0_NACK_BIT   0

#define MPU6050_INTCFG_INT_LEVEL_BIT        7
#define MPU6050_INTCFG_INT_OPEN_BIT         6
#define MPU6050_INTCFG_LATCH_INT_EN_BIT     5
#define MPU6050_INTCFG_INT_RD_CLEAR_BIT     4
#define MPU6050_INTCFG_FSYNC_INT_LEVEL_BIT  3
#define MPU6050_INTCFG_FSYNC_INT_EN_BIT     2
#define MPU6050_INTCFG_I2C_BYPASS_EN_BIT    1
#define MPU6050_INTCFG_CLKOUT_EN_BIT        0

#define MPU6050_INTMODE_ACTIVEHIGH  0x00
#define MPU6050_INTMODE_ACTIVELOW   0x01

#define MPU6050_INTDRV_PUSHPULL     0x00
#define MPU6050_INTDRV_OPENDRAIN    0x01

#define MPU6050_INTLATCH_50USPULSE  0x00
#define MPU6050_INTLATCH_WAITCLEAR  0x01

#define MPU6050_INTCLEAR_STATUSREAD 0x00
#define MPU6050_INTCLEAR_ANYREAD    0x01

#define MPU6050_INTERRUPT_FF_BIT            7
#define MPU6050_INTERRUPT_MOT_BIT           6
#define MPU6050_INTERRUPT_ZMOT_BIT          5
#define MPU6050_INTERRUPT_FIFO_OFLOW_BIT    4
#define MPU6050_INTERRUPT_I2C_MST_INT_BIT   3
#define MPU6050_INTERRUPT_PLL_RDY_INT_BIT   2
#define MPU6050_INTERRUPT_DMP_INT_BIT       1
#define MPU6050_INTERRUPT_DATA_RDY_BIT      0

// TODO: figure out what these actually do
// UMPL source code is not very obivous
#define MPU6050_DMPINT_5_BIT            5
#define MPU6050_DMPINT_4_BIT            4
#define MPU6050_DMPINT_3_BIT            3
#define MPU6050_DMPINT_2_BIT            2
#define MPU6050_DMPINT_1_BIT            1
#define MPU6050_DMPINT_0_BIT            0

#define MPU6050_MOTION_MOT_XNEG_BIT     7
#define MPU6050_MOTION_MOT_XPOS_BIT     6
#define MPU6050_MOTION_MOT_YNEG_BIT     5
#define MPU6050_MOTION_MOT_YPOS_BIT     4
#define MPU6050_MOTION_MOT_ZNEG_BIT     3
#define MPU6050_MOTION_MOT_ZPOS_BIT     2
#define MPU6050_MOTION_MOT_ZRMOT_BIT    0

#define MPU6050_DELAYCTRL_DELAY_ES_SHADOW_BIT   7
#define MPU6050_DELAYCTRL_I2C_SLV4_DLY_EN_BIT   4
#define MPU6050_DELAYCTRL_I2C_SLV3_DLY_EN_BIT   3
#define MPU6050_DELAYCTRL_I2C_SLV2_DLY_EN_BIT   2
#define MPU6050_DELAYCTRL_I2C_SLV1_DLY_EN_BIT   1
#define MPU6050_DELAYCTRL_I2C_SLV0_DLY_EN_BIT   0

#define MPU6050_PATHRESET_GYRO_RESET_BIT    2
#define MPU6050_PATHRESET_ACCEL_RESET_BIT   1
#define MPU6050_PATHRESET_TEMP_RESET_BIT    0

#define MPU6050_DETECT_ACCEL_ON_DELAY_BIT       5
#define MPU6050_DETECT_ACCEL_ON_DELAY_LENGTH    2
#define MPU6050_DETECT_FF_COUNT_BIT             3
#define MPU6050_DETECT_FF_COUNT_LENGTH          2
#define MPU6050_DETECT_MOT_COUNT_BIT            1
#define MPU6050_DETECT_MOT_COUNT_LENGTH         2

#define MPU6050_DETECT_DECREMENT_RESET  0x0
#define MPU6050_DETECT_DECREMENT_1      0x1
#define MPU6050_DETECT_DECREMENT_2      0x2
#define MPU6050_DETECT_DECREMENT_4      0x3

#define MPU6050_USERCTRL_DMP_EN_BIT             7
#define MPU6050_USERCTRL_FIFO_EN_BIT            6
#define MPU6050_USERCTRL_I2C_MST_EN_BIT         5
#define MPU6050_USERCTRL_I2C_IF_DIS_BIT         4
#define MPU6050_USERCTRL_DMP_RESET_BIT          3
#define MPU6050_USERCTRL_FIFO_RESET_BIT         2
#define MPU6050_USERCTRL_I2C_MST_RESET_BIT      1
#define MPU6050_USERCTRL_SIG_COND_RESET_BIT     0

#define MPU6050_PWR1_DEVICE_RESET_BIT   7
#define MPU6050_PWR1_SLEEP_BIT          6
#define MPU6050_PWR1_CYCLE_BIT          5
#define MPU6050_PWR1_TEMP_DIS_BIT       3
#define MPU6050_PWR1_CLKSEL_BIT         2
#define MPU6050_PWR1_CLKSEL_LENGTH      3

#define MPU6050_CLOCK_INTERNAL          0x00
#define MPU6050_CLOCK_PLL_XGYRO         0x01
#define MPU6050_CLOCK_PLL_YGYRO         0x02
#define MPU6050_CLOCK_PLL_ZGYRO         0x03
#define MPU6050_CLOCK_PLL_EXT32K        0x04
#define MPU6050_CLOCK_PLL_EXT19M        0x05
#define MPU6050_CLOCK_KEEP_RESET        0x07

#define MPU6050_PWR2_LP_WAKE_CTRL_BIT       7
#define MPU6050_PWR2_LP_WAKE_CTRL_LENGTH    2
#define MPU6050_PWR2_STBY_XA_BIT            5
#define MPU6050_PWR2_STBY_YA_BIT            4
#define MPU6050_PWR2_STBY_ZA_BIT            3
#define MPU6050_PWR2_STBY_XG_BIT            2
#define MPU6050_PWR2_STBY_YG_BIT            1
#define MPU6050_PWR2_STBY_ZG_BIT            0

#define MPU6050_WAKE_FREQ_1P25      0x0
#define MPU6050_WAKE_FREQ_2P5       0x1
#define MPU6050_WAKE_FREQ_5         0x2
#define MPU6050_WAKE_FREQ_10        0x3

#define MPU6050_BANKSEL_PRFTCH_EN_BIT       6
#define MPU6050_BANKSEL_CFG_USER_BANK_BIT   5
#define MPU6050_BANKSEL_MEM_SEL_BIT         4
#define MPU6050_BANKSEL_MEM_SEL_LENGTH      5

#define MPU6050_WHO_AM_I_BIT        6
#define MPU6050_WHO_AM_I_LENGTH     6

#define MPU6050_DMP_MEMORY_BANKS        8
#define MPU6050_DMP_MEMORY_BANK_SIZE    256
#define MPU6050_DMP_MEMORY_CHUNK_SIZE   16

// note: DMP code memory blocks defined at end of header file
extern void HalMPU6050initialize();
extern bool HalMPU6050testConnection();

        // AUX_VDDIO register
extern uint8_t HalMPU6050getAuxVDDIOLevel();
extern void HalMPU6050setAuxVDDIOLevel(uint8_t level);

        // SMPLRT_DIV register
extern uint8_t HalMPU6050getRate();
extern void HalMPU6050setRate(uint8_t rate);

        // CONFIG register
extern uint8_t HalMPU6050getExternalFrameSync();
extern void HalMPU6050setExternalFrameSync(uint8_t sync);
extern uint8_t HalMPU6050getDLPFMode();
extern void HalMPU6050setDLPFMode(uint8_t bandwidth);

        // GYRO_CONFIG register
extern uint8_t HalMPU6050getFullScaleGyroRange();
extern void HalMPU6050setFullScaleGyroRange(uint8_t range);

        // ACCEL_CONFIG register
extern bool HalMPU6050getAccelXSelfTest();
extern void HalMPU6050setAccelXSelfTest(bool enabled);
extern bool HalMPU6050getAccelYSelfTest();
extern void HalMPU6050setAccelYSelfTest(bool enabled);
extern bool HalMPU6050getAccelZSelfTest();
extern void HalMPU6050setAccelZSelfTest(bool enabled);
extern uint8_t HalMPU6050getFullScaleAccelRange();
extern void HalMPU6050setFullScaleAccelRange(uint8_t range);
extern uint8_t HalMPU6050getDHPFMode();
extern void HalMPU6050setDHPFMode(uint8_t mode);

        // FF_THR register
extern uint8_t HalMPU6050getFreefallDetectionThreshold();
extern void HalMPU6050setFreefallDetectionThreshold(uint8_t threshold);

        // FF_DUR register
extern uint8_t HalMPU6050getFreefallDetectionDuration();
extern void HalMPU6050setFreefallDetectionDuration(uint8_t duration);

        // MOT_THR register
extern uint8_t HalMPU6050getMotionDetectionThreshold();
extern void HalMPU6050setMotionDetectionThreshold(uint8_t threshold);

        // MOT_DUR register
extern uint8_t HalMPU6050getMotionDetectionDuration();
extern void HalMPU6050setMotionDetectionDuration(uint8_t duration);

        // ZRMOT_THR register
extern uint8_t HalMPU6050getZeroMotionDetectionThreshold();
extern void HalMPU6050setZeroMotionDetectionThreshold(uint8_t threshold);

        // ZRMOT_DUR register
extern uint8_t HalMPU6050getZeroMotionDetectionDuration();
extern void HalMPU6050setZeroMotionDetectionDuration(uint8_t duration);

        // FIFO_EN register
extern bool HalMPU6050getTempFIFOEnabled();
extern void HalMPU6050setTempFIFOEnabled(bool enabled);
extern bool HalMPU6050getXGyroFIFOEnabled();
extern void HalMPU6050setXGyroFIFOEnabled(bool enabled);
extern bool HalMPU6050getYGyroFIFOEnabled();
extern void HalMPU6050setYGyroFIFOEnabled(bool enabled);
extern bool HalMPU6050getZGyroFIFOEnabled();
extern void HalMPU6050setZGyroFIFOEnabled(bool enabled);
extern bool HalMPU6050getAccelFIFOEnabled();
extern void HalMPU6050setAccelFIFOEnabled(bool enabled);
extern bool HalMPU6050getSlave2FIFOEnabled();
extern void HalMPU6050setSlave2FIFOEnabled(bool enabled);
extern bool HalMPU6050getSlave1FIFOEnabled();
extern void HalMPU6050setSlave1FIFOEnabled(bool enabled);
extern bool HalMPU6050getSlave0FIFOEnabled();
extern void HalMPU6050setSlave0FIFOEnabled(bool enabled);

        // I2C_MST_CTRL register
extern bool HalMPU6050getMultiMasterEnabled();
extern void HalMPU6050setMultiMasterEnabled(bool enabled);
extern bool HalMPU6050getWaitForExternalSensorEnabled();
extern void HalMPU6050setWaitForExternalSensorEnabled(bool enabled);
extern bool HalMPU6050getSlave3FIFOEnabled();
extern void HalMPU6050setSlave3FIFOEnabled(bool enabled);
extern bool HalMPU6050getSlaveReadWriteTransitionEnabled();
extern void HalMPU6050setSlaveReadWriteTransitionEnabled(bool enabled);
extern uint8_t HalMPU6050getMasterClockSpeed();
extern void HalMPU6050setMasterClockSpeed(uint8_t speed);

        // I2C_SLV* registers (Slave 0-3)
extern uint8_t HalMPU6050getSlaveAddress(uint8_t num);
extern void HalMPU6050setSlaveAddress(uint8_t num, uint8_t address);
extern uint8_t HalMPU6050getSlaveRegister(uint8_t num);
extern void HalMPU6050setSlaveRegister(uint8_t num, uint8_t reg);
extern bool HalMPU6050getSlaveEnabled(uint8_t num);
extern void HalMPU6050setSlaveEnabled(uint8_t num, bool enabled);
extern bool HalMPU6050getSlaveWordByteSwap(uint8_t num);
extern void HalMPU6050setSlaveWordByteSwap(uint8_t num, bool enabled);
extern bool HalMPU6050getSlaveWriteMode(uint8_t num);
extern void HalMPU6050setSlaveWriteMode(uint8_t num, bool mode);
extern bool HalMPU6050getSlaveWordGroupOffset(uint8_t num);
extern void HalMPU6050setSlaveWordGroupOffset(uint8_t num, bool enabled);
extern uint8_t HalMPU6050getSlaveDataLength(uint8_t num);
extern void HalMPU6050setSlaveDataLength(uint8_t num, uint8_t length);

        // I2C_SLV* registers (Slave 4)
extern uint8_t HalMPU6050getSlave4Address();
extern void HalMPU6050setSlave4Address(uint8_t address);
extern uint8_t HalMPU6050getSlave4Register();
extern void HalMPU6050setSlave4Register(uint8_t reg);
extern void HalMPU6050setSlave4OutputByte(uint8_t data);
extern bool HalMPU6050getSlave4Enabled();
extern void HalMPU6050setSlave4Enabled(bool enabled);
extern bool HalMPU6050getSlave4InterruptEnabled();
extern void HalMPU6050setSlave4InterruptEnabled(bool enabled);
extern bool HalMPU6050getSlave4WriteMode();
extern void HalMPU6050setSlave4WriteMode(bool mode);
extern uint8_t HalMPU6050getSlave4MasterDelay();
extern void HalMPU6050setSlave4MasterDelay(uint8_t delay);
extern uint8_t HalMPU6050getSlate4InputByte();

        // I2C_MST_STATUS register
extern bool HalMPU6050getPassthroughStatus();
extern bool HalMPU6050getSlave4IsDone();
extern bool HalMPU6050getLostArbitration();
extern bool HalMPU6050getSlave4Nack();
extern bool HalMPU6050getSlave3Nack();
extern bool HalMPU6050getSlave2Nack();
extern bool HalMPU6050getSlave1Nack();
extern bool HalMPU6050getSlave0Nack();

        // INT_PIN_CFG register
extern bool HalMPU6050getInterruptMode();
extern void HalMPU6050setInterruptMode(bool mode);
extern bool HalMPU6050getInterruptDrive();
extern void HalMPU6050setInterruptDrive(bool drive);
extern bool HalMPU6050getInterruptLatch();
extern void HalMPU6050setInterruptLatch(bool latch);
extern bool HalMPU6050getInterruptLatchClear();
extern void HalMPU6050setInterruptLatchClear(bool clear);
extern bool HalMPU6050getFSyncInterruptLevel();
extern void HalMPU6050setFSyncInterruptLevel(bool level);
extern bool HalMPU6050getFSyncInterruptEnabled();
extern void HalMPU6050setFSyncInterruptEnabled(bool enabled);
extern bool HalMPU6050getI2CBypassEnabled();
extern void HalMPU6050setI2CBypassEnabled(bool enabled);
extern bool HalMPU6050getClockOutputEnabled();
extern void HalMPU6050setClockOutputEnabled(bool enabled);

        // INT_ENABLE register
extern uint8_t HalMPU6050getIntEnabled();
extern void HalMPU6050setIntEnabled(uint8_t enabled);
extern bool HalMPU6050getIntFreefallEnabled();
extern void HalMPU6050setIntFreefallEnabled(bool enabled);
extern bool HalMPU6050getIntMotionEnabled();
extern void HalMPU6050setIntMotionEnabled(bool enabled);
extern bool HalMPU6050getIntZeroMotionEnabled();
extern void HalMPU6050setIntZeroMotionEnabled(bool enabled);
extern bool HalMPU6050getIntFIFOBufferOverflowEnabled();
extern void HalMPU6050setIntFIFOBufferOverflowEnabled(bool enabled);
extern bool HalMPU6050getIntI2CMasterEnabled();
extern void HalMPU6050setIntI2CMasterEnabled(bool enabled);
extern bool HalMPU6050getIntDataReadyEnabled();
extern void HalMPU6050setIntDataReadyEnabled(bool enabled);

        // INT_STATUS register
extern uint8_t HalMPU6050getIntStatus();
extern bool HalMPU6050getIntFreefallStatus();
extern bool HalMPU6050getIntMotionStatus();
extern bool HalMPU6050getIntZeroMotionStatus();
extern bool HalMPU6050getIntFIFOBufferOverflowStatus();
extern bool HalMPU6050getIntI2CMasterStatus();
extern bool HalMPU6050getIntDataReadyStatus();

        // ACCEL_*OUT_* registers
extern void HalMPU6050getMotion9(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz, int16_t* mx, int16_t* my, int16_t* mz);
extern void HalMPU6050getMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);
extern void HalMPU6050getAcceleration(int16_t* x, int16_t* y, int16_t* z);
extern int16_t HalMPU6050getAccelerationX();
extern int16_t HalMPU6050getAccelerationY();
extern int16_t HalMPU6050getAccelerationZ();

        // TEMP_OUT_* registers
extern int16_t HalMPU6050getTemperature();

        // GYRO_*OUT_* registers
extern void HalMPU6050getRotation(int16_t* x, int16_t* y, int16_t* z);
extern int16_t HalMPU6050getRotationX();
extern int16_t HalMPU6050getRotationY();
extern int16_t HalMPU6050getRotationZ();

        // EXT_SENS_DATA_* registers
extern uint8_t HalMPU6050getExternalSensorByte(int position);
extern uint16_t HalMPU6050getExternalSensorWord(int position);
extern uint32_t HalMPU6050getExternalSensorDWord(int position);

        // MOT_DETECT_STATUS register
extern bool HalMPU6050getXNegMotionDetected();
extern bool HalMPU6050getXPosMotionDetected();
extern bool HalMPU6050getYNegMotionDetected();
extern bool HalMPU6050getYPosMotionDetected();
extern bool HalMPU6050getZNegMotionDetected();
extern bool HalMPU6050getZPosMotionDetected();
extern bool HalMPU6050getZeroMotionDetected();

        // I2C_SLV*_DO register
extern void HalMPU6050setSlaveOutputByte(uint8_t num, uint8_t data);

        // I2C_MST_DELAY_CTRL register
extern bool HalMPU6050getExternalShadowDelayEnabled();
extern void HalMPU6050setExternalShadowDelayEnabled(bool enabled);
extern bool HalMPU6050getSlaveDelayEnabled(uint8_t num);
extern void HalMPU6050setSlaveDelayEnabled(uint8_t num, bool enabled);

        // SIGNAL_PATH_RESET register
extern void HalMPU6050resetGyroscopePath();
extern void HalMPU6050resetAccelerometerPath();
extern void HalMPU6050resetTemperaturePath();

        // MOT_DETECT_CTRL register
extern uint8_t HalMPU6050getAccelerometerPowerOnDelay();
extern void HalMPU6050setAccelerometerPowerOnDelay(uint8_t delay);
extern uint8_t HalMPU6050getFreefallDetectionCounterDecrement();
extern void HalMPU6050setFreefallDetectionCounterDecrement(uint8_t decrement);
extern uint8_t HalMPU6050getMotionDetectionCounterDecrement();
extern void HalMPU6050setMotionDetectionCounterDecrement(uint8_t decrement);

        // USER_CTRL register
extern bool HalMPU6050getFIFOEnabled();
extern void HalMPU6050setFIFOEnabled(bool enabled);
extern bool HalMPU6050getI2CMasterModeEnabled();
extern void HalMPU6050setI2CMasterModeEnabled(bool enabled);
extern void HalMPU6050switchSPIEnabled(bool enabled);
extern void HalMPU6050resetFIFO();
extern void HalMPU6050resetI2CMaster();
extern void HalMPU6050resetSensors();

        // PWR_MGMT_1 register
extern void HalMPU6050reset();
extern bool HalMPU6050getSleepEnabled();
extern void HalMPU6050setSleepEnabled(bool enabled);
extern bool HalMPU6050getWakeCycleEnabled();
extern void HalMPU6050setWakeCycleEnabled(bool enabled);
extern bool HalMPU6050getTempSensorEnabled();
extern void HalMPU6050setTempSensorEnabled(bool enabled);
extern uint8_t HalMPU6050getClockSource();
extern void HalMPU6050setClockSource(uint8_t source);

        // PWR_MGMT_2 register
extern uint8_t HalMPU6050getWakeFrequency();
extern void HalMPU6050setWakeFrequency(uint8_t frequency);
extern bool HalMPU6050getStandbyXAccelEnabled();
extern void HalMPU6050setStandbyXAccelEnabled(bool enabled);
extern bool HalMPU6050getStandbyYAccelEnabled();
extern void HalMPU6050setStandbyYAccelEnabled(bool enabled);
extern bool HalMPU6050getStandbyZAccelEnabled();
extern void HalMPU6050setStandbyZAccelEnabled(bool enabled);
extern bool HalMPU6050getStandbyXGyroEnabled();
extern void HalMPU6050setStandbyXGyroEnabled(bool enabled);
extern bool HalMPU6050getStandbyYGyroEnabled();
extern void HalMPU6050setStandbyYGyroEnabled(bool enabled);
extern bool HalMPU6050getStandbyZGyroEnabled();
extern void HalMPU6050setStandbyZGyroEnabled(bool enabled);

        // FIFO_COUNT_* registers
extern uint16_t HalMPU6050getFIFOCount();

        // FIFO_R_W register
extern uint8_t HalMPU6050getFIFOByte();
extern void HalMPU6050setFIFOByte(uint8_t data);
extern void HalMPU6050getFIFOBytes(uint8_t *data, uint8_t length);

        // WHO_AM_I register
extern uint8_t HalMPU6050getDeviceID();
extern void HalMPU6050setDeviceID(uint8_t id);

        // ======== UNDOCUMENTED/DMP REGISTERS/METHODS ========

        // XG_OFFS_TC register
extern uint8_t HalMPU6050getOTPBankValid();
extern void HalMPU6050setOTPBankValid(bool enabled);
        extern int8_t HalMPU6050getXGyroOffsetTC();
extern void HalMPU6050setXGyroOffsetTC(int8_t offset);

        // YG_OFFS_TC register
extern int8_t HalMPU6050getYGyroOffsetTC();
extern void HalMPU6050setYGyroOffsetTC(int8_t offset);

        // ZG_OFFS_TC register
extern int8_t HalMPU6050getZGyroOffsetTC();
extern void HalMPU6050setZGyroOffsetTC(int8_t offset);

        // X_FINE_GAIN register
extern int8_t HalMPU6050getXFineGain();
extern void HalMPU6050setXFineGain(int8_t gain);

        // Y_FINE_GAIN register
extern int8_t HalMPU6050getYFineGain();
extern void HalMPU6050setYFineGain(int8_t gain);

        // Z_FINE_GAIN register
extern int8_t HalMPU6050getZFineGain();
extern void HalMPU6050setZFineGain(int8_t gain);

        // XA_OFFS_* registers
extern int16_t HalMPU6050getXAccelOffset();
extern void HalMPU6050setXAccelOffset(int16_t offset);

        // YA_OFFS_* register
extern int16_t HalMPU6050getYAccelOffset();
extern void HalMPU6050setYAccelOffset(int16_t offset);

        // ZA_OFFS_* register
extern int16_t HalMPU6050getZAccelOffset();
extern void HalMPU6050setZAccelOffset(int16_t offset);

        // XG_OFFS_USR* registers
extern int16_t HalMPU6050getXGyroOffset();
extern void HalMPU6050setXGyroOffset(int16_t offset);

        // YG_OFFS_USR* register
extern int16_t HalMPU6050getYGyroOffset();
extern void HalMPU6050setYGyroOffset(int16_t offset);

        // ZG_OFFS_USR* register
extern int16_t HalMPU6050getZGyroOffset();
extern void HalMPU6050setZGyroOffset(int16_t offset);

        // INT_ENABLE register (DMP functions)
extern bool HalMPU6050getIntPLLReadyEnabled();
extern void HalMPU6050setIntPLLReadyEnabled(bool enabled);
extern bool HalMPU6050getIntDMPEnabled();
extern void HalMPU6050setIntDMPEnabled(bool enabled);

        // DMP_INT_STATUS
extern bool HalMPU6050getDMPInt5Status();
extern bool HalMPU6050getDMPInt4Status();
extern bool HalMPU6050getDMPInt3Status();
extern bool HalMPU6050getDMPInt2Status();
extern bool HalMPU6050getDMPInt1Status();
extern bool HalMPU6050getDMPInt0Status();

        // INT_STATUS register (DMP functions)
extern bool HalMPU6050getIntPLLReadyStatus();
extern bool HalMPU6050getIntDMPStatus();

        // USER_CTRL register (DMP functions)
extern bool HalMPU6050getDMPEnabled();
extern void HalMPU6050setDMPEnabled(bool enabled);
extern void HalMPU6050resetDMP();

        // BANK_SEL register
extern void HalMPU6050setMemoryBank(uint8_t bank, bool prefetchEnabled, bool userBank);

        // MEM_START_ADDR register
extern void HalMPU6050setMemoryStartAddress(uint8_t address);

        // MEM_R_W register
extern uint8_t HalMPU6050readMemoryByte();
extern void HalMPU6050writeMemoryByte(uint8_t data);

//extern void HalMPU6050readMemoryBlock(uint8_t *data, uint16_t dataSize, uint8_t bank=0, uint8_t address=0);
extern void HalMPU6050readMemoryBlock(uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address);

//extern bool HalMPU6050writeMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank=0, uint8_t address=0, bool verify=true, bool useProgMem=false);
extern bool HalMPU6050writeMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify, bool useProgMem);

//extern bool HalMPU6050writeProgMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank=0, uint8_t address=0, bool verify=true);
extern bool HalMPU6050writeProgMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify);

//extern bool HalMPU6050writeDMPConfigurationSet(const uint8_t *data, uint16_t dataSize, bool useProgMem=false);
extern bool HalMPU6050writeDMPConfigurationSet(const uint8_t *data, uint16_t dataSize, bool useProgMem);

extern bool HalMPU6050writeProgDMPConfigurationSet(const uint8_t *data, uint16_t dataSize);

        // DMP_CFG_1 register
extern uint8_t HalMPU6050getDMPConfig1();
extern void HalMPU6050setDMPConfig1(uint8_t config);

        // DMP_CFG_2 register
extern uint8_t HalMPU6050getDMPConfig2();
extern void HalMPU6050setDMPConfig2(uint8_t config);

// special methods for MotionApps 2.0 implementation
#ifdef MPU6050_INCLUDE_DMP_MOTIONAPPS20
    uint8_t *dmpPacketBuffer;
    //static uint16_t dmpPacketSize = 42;

    extern uint8_t HalMPU6050dmpInitialize();
    extern bool HalMPU6050dmpPacketAvailable();

    extern uint8_t HalMPU6050dmpSetFIFORate(uint8_t fifoRate);
    extern uint8_t HalMPU6050dmpGetFIFORate();
    extern uint8_t HalMPU6050dmpGetSampleStepSizeMS();
    extern uint8_t HalMPU6050dmpGetSampleFrequency();
    extern int32_t HalMPU6050dmpDecodeTemperature(int8_t tempReg);

            // Register callbacks after a packet of FIFO data is processed
            //uint8_t dmpRegisterFIFORateProcess(inv_obj_func func, int16_t priority);
            //uint8_t dmpUnregisterFIFORateProcess(inv_obj_func func);
    extern uint8_t HalMPU6050dmpRunFIFORateProcesses();

/*
            // Setup FIFO for various output
    extern uint8_t HalMPU6050dmpSendQuaternion(uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendGyro(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendAccel(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendLinearAccel(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendLinearAccelInWorld(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendControlData(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendSensorData(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendExternalSensorData(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendGravity(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendPacketNumber(uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendQuantizedAccel(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendEIS(uint_fast16_t elements, uint_fast16_t accuracy);
*/
            // Get Fixed Point data from FIFO
            // All packet default value is 0 --- by Phobos.
    //extern uint8_t HalMPU6050dmpGetAccel(int32_t *data, const uint8_t* packet);
    //extern uint8_t HalMPU6050dmpGetAccel(int16_t *data, const uint8_t* packet);
    //extern uint8_t HalMPU6050dmpGetAccel(VectorInt16 *v, const uint8_t* packet);
    //extern uint8_t HalMPU6050dmpGetQuaternion(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetQuaternion(int16_t *data, const uint8_t* packet);
/*
    extern uint8_t HalMPU6050dmpGetQuaternion(Quaternion *q, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGet6AxisQuaternion(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGet6AxisQuaternion(int16_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGet6AxisQuaternion(Quaternion *q, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetRelativeQuaternion(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetRelativeQuaternion(int16_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetRelativeQuaternion(Quaternion *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGyro(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGyro(int16_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGyro(VectorInt16 *v, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpSetLinearAccelFilterCoefficient(float coef);
    extern uint8_t HalMPU6050dmpGetLinearAccel(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetLinearAccel(int16_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetLinearAccel(VectorInt16 *v, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetLinearAccel(VectorInt16 *v, VectorInt16 *vRaw, VectorFloat *gravity);
    extern uint8_t HalMPU6050dmpGetLinearAccelInWorld(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetLinearAccelInWorld(int16_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetLinearAccelInWorld(VectorInt16 *v, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetLinearAccelInWorld(VectorInt16 *v, VectorInt16 *vReal, Quaternion *q);
    extern uint8_t HalMPU6050dmpGetGyroAndAccelSensor(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGyroAndAccelSensor(int16_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGyroAndAccelSensor(VectorInt16 *g, VectorInt16 *a, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGyroSensor(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGyroSensor(int16_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGyroSensor(VectorInt16 *v, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetControlData(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetTemperature(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGravity(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGravity(int16_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGravity(VectorInt16 *v, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetGravity(VectorFloat *v, Quaternion *q);
    extern uint8_t HalMPU6050dmpGetUnquantizedAccel(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetUnquantizedAccel(int16_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetUnquantizedAccel(VectorInt16 *v, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetQuantizedAccel(int32_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetQuantizedAccel(int16_t *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetQuantizedAccel(VectorInt16 *v, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetExternalSensorData(int32_t *data, uint16_t size, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetEIS(int32_t *data, const uint8_t* packet);

    extern uint8_t HalMPU6050dmpGetEuler(float *data, Quaternion *q);
    extern uint8_t HalMPU6050dmpGetYawPitchRoll(float *data, Quaternion *q, VectorFloat *gravity);
*/
            // Get Floating Point data from FIFO
    extern uint8_t HalMPU6050dmpGetAccelFloat(float *data, const uint8_t* packet);
    extern uint8_t HalMPU6050dmpGetQuaternionFloat(float *data, const uint8_t* packet);

    extern uint8_t HalMPU6050dmpProcessFIFOPacket(const unsigned char *dmpData);
    //extern uint8_t HalMPU6050dmpReadAndProcessFIFOPacket(uint8_t numPackets, uint8_t *processed=NULL);
    extern uint8_t HalMPU6050dmpReadAndProcessFIFOPacket(uint8_t numPackets, uint8_t *processed);
//    extern uint8_t HalMPU6050dmpSetFIFOProcessedCallback(extern void HalMPU6050(*func) (void));

    extern uint8_t HalMPU6050dmpInitFIFOParam();
    extern uint8_t HalMPU6050dmpCloseFIFO();
    extern uint8_t HalMPU6050dmpSetGyroDataSource(uint8_t source);
    extern uint8_t HalMPU6050dmpDecodeQuantizedAccel();
    extern uint32_t HalMPU6050dmpGetGyroSumOfSquare();
    extern uint32_t HalMPU6050dmpGetAccelSumOfSquare();
    extern void HalMPU6050dmpOverrideQuaternion(long *q);
    extern uint16_t HalMPU6050dmpGetFIFOPacketSize();
#endif

// special methods for MotionApps 4.1 implementation
#ifdef MPU6050_INCLUDE_DMP_MOTIONAPPS41
    uint8_t *dmpPacketBuffer;
    uint16_t dmpPacketSize;

    extern uint8_t HalMPU6050dmpInitialize();
    extern bool HalMPU6050dmpPacketAvailable();

    extern uint8_t HalMPU6050dmpSetFIFORate(uint8_t fifoRate);
    extern uint8_t HalMPU6050dmpGetFIFORate();
    extern uint8_t HalMPU6050dmpGetSampleStepSizeMS();
    extern uint8_t HalMPU6050dmpGetSampleFrequency();
    extern int32_t HalMPU6050dmpDecodeTemperature(int8_t tempReg);

            // Register callbacks after a packet of FIFO data is processed
            //uint8_t dmpRegisterFIFORateProcess(inv_obj_func func, int16_t priority);
            //uint8_t dmpUnregisterFIFORateProcess(inv_obj_func func);
    extern uint8_t HalMPU6050dmpRunFIFORateProcesses();
/*
            // Setup FIFO for various output
    extern uint8_t HalMPU6050dmpSendQuaternion(uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendGyro(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendAccel(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendLinearAccel(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendLinearAccelInWorld(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendControlData(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendSensorData(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendExternalSensorData(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendGravity(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendPacketNumber(uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendQuantizedAccel(uint_fast16_t elements, uint_fast16_t accuracy);
    extern uint8_t HalMPU6050dmpSendEIS(uint_fast16_t elements, uint_fast16_t accuracy);
*/
            // Get Fixed Point data from FIFO
    extern uint8_t HalMPU6050dmpGetAccel(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetAccel(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetAccel(VectorInt16 *v, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetQuaternion(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetQuaternion(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetQuaternion(Quaternion *q, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGet6AxisQuaternion(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGet6AxisQuaternion(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGet6AxisQuaternion(Quaternion *q, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetRelativeQuaternion(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetRelativeQuaternion(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetRelativeQuaternion(Quaternion *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGyro(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGyro(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGyro(VectorInt16 *v, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetMag(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpSetLinearAccelFilterCoefficient(float coef);
    extern uint8_t HalMPU6050dmpGetLinearAccel(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetLinearAccel(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetLinearAccel(VectorInt16 *v, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetLinearAccel(VectorInt16 *v, VectorInt16 *vRaw, VectorFloat *gravity);
    extern uint8_t HalMPU6050dmpGetLinearAccelInWorld(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetLinearAccelInWorld(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetLinearAccelInWorld(VectorInt16 *v, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetLinearAccelInWorld(VectorInt16 *v, VectorInt16 *vReal, Quaternion *q);
    extern uint8_t HalMPU6050dmpGetGyroAndAccelSensor(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGyroAndAccelSensor(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGyroAndAccelSensor(VectorInt16 *g, VectorInt16 *a, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGyroSensor(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGyroSensor(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGyroSensor(VectorInt16 *v, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetControlData(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetTemperature(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGravity(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGravity(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGravity(VectorInt16 *v, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetGravity(VectorFloat *v, Quaternion *q);
    extern uint8_t HalMPU6050dmpGetUnquantizedAccel(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetUnquantizedAccel(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetUnquantizedAccel(VectorInt16 *v, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetQuantizedAccel(int32_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetQuantizedAccel(int16_t *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetQuantizedAccel(VectorInt16 *v, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetExternalSensorData(int32_t *data, uint16_t size, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetEIS(int32_t *data, const uint8_t* packet=0);

    extern uint8_t HalMPU6050dmpGetEuler(float *data, Quaternion *q);
    extern uint8_t HalMPU6050dmpGetYawPitchRoll(float *data, Quaternion *q, VectorFloat *gravity);

            // Get Floating Point data from FIFO
    extern uint8_t HalMPU6050dmpGetAccelFloat(float *data, const uint8_t* packet=0);
    extern uint8_t HalMPU6050dmpGetQuaternionFloat(float *data, const uint8_t* packet=0);

    extern uint8_t HalMPU6050dmpProcessFIFOPacket(const unsigned char *dmpData);
    extern uint8_t HalMPU6050dmpReadAndProcessFIFOPacket(uint8_t numPackets, uint8_t *processed=NULL);

    extern uint8_t HalMPU6050dmpSetFIFOProcessedCallback(extern void HalMPU6050(*func) (void));

    extern uint8_t HalMPU6050dmpInitFIFOParam();
    extern uint8_t HalMPU6050dmpCloseFIFO();
    extern uint8_t HalMPU6050dmpSetGyroDataSource(uint8_t source);
    extern uint8_t HalMPU6050dmpDecodeQuantizedAccel();
    extern uint32_t HalMPU6050dmpGetGyroSumOfSquare();
    extern uint32_t HalMPU6050dmpGetAccelSumOfSquare();
    extern void HalMPU6050dmpOverrideQuaternion(long *q);
    extern uint16_t HalMPU6050dmpGetFIFOPacketSize();
#endif
#endif /* _MPU6050_H_ */