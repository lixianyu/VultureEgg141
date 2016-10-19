/**************************************************************************************************
  Filename:       hal_irtemp.c
  Revised:        $Date: 2013-04-05 07:25:57 -0700 (Fri, 05 Apr 2013) $
  Revision:       $Revision: 33773 $

  Description:    Driver for the TI TMP06 infrared thermophile sensor.


  Copyright 2012-2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/* ------------------------------------------------------------------------------------------------
*                                          Includes
* ------------------------------------------------------------------------------------------------
*/
#include "hal_LM75A_nxp.h"
#include "hal_i2c.h"
#include "hal_sensor.h"

/* ------------------------------------------------------------------------------------------------
*                                           Constants
* ------------------------------------------------------------------------------------------------
*/
//#define MOVE_RIGHT5

#define LM75A_NUMBER                    8

/* Slave address */
#define LM75A_I2C_ADDRESS0              0x48
#define LM75A_I2C_ADDRESS1              0x49
#define LM75A_I2C_ADDRESS2              0x4A
#define LM75A_I2C_ADDRESS3              0x4B
#define LM75A_I2C_ADDRESS4              0x4C
#define LM75A_I2C_ADDRESS5              0x4D
#define LM75A_I2C_ADDRESS6              0x4E
#define LM75A_I2C_ADDRESS7              0x4F

/* LM75A register addresses */
#define LM75A_REG_ADDR_TEMPERATURE     0x00
#define LM75A_REG_ADDR_CONFIG          0x01
#define LM75A_REG_ADDR_THYST           0x02
#define LM75A_REG_ADDR_TOS             0x03

/* LM75A register values */
/*#define TMP006_VAL_CONFIG_RESET         0x7400  // Sensor reset state
#define TMP006_VAL_CONFIG_ON            0x7000  // Sensor on state
#define TMP006_VAL_CONFIG_OFF           0x0000  // Sensor off state
#define TMP006_VAL_MANF_ID              0x5449  // Manufacturer ID
#define TMP006_VAL_PROD_ID              0x0067  // Product ID
*/

/* Bit values */
//#define DATA_RDY_BIT                    0x8000 // Data ready

/* Register length */
//#define IRTEMP_REG_LEN                  2


/* ------------------------------------------------------------------------------------------------
*                                           Local Functions
* ------------------------------------------------------------------------------------------------
*/
static void HalLM75ATempSelect(uint8 id);


/* ------------------------------------------------------------------------------------------------
*                                           Local Variables
* ------------------------------------------------------------------------------------------------
*/
static LM75ATemperature_States_t irtSensorState = LM75A_OFF;

//static uint8 configSensorReset[2] = {0x80, 0x00};  // Sensor reset
static uint8 configLM75AOff[1] = {0x01};    // LM75A shutdown
static uint8 configLM75AOn[1] =  {0x00};    // LM75A normal

/* ------------------------------------------------------------------------------------------------
*                                           Public functions
* -------------------------------------------------------------------------------------------------
*/


/**************************************************************************************************
 * @fn          HALLM75ATempInit
 *
 * @brief       Initialise the temperature sensor driver
 *
 * @return      none
 **************************************************************************************************/
void HALLM75ATempInit(void)
{
  irtSensorState = LM75A_NORMAL;
  for (int i = 0; i < LM75A_NUMBER; i++)
  {
    HalLM75ATempTurnOff(i);
  }
}


/**************************************************************************************************
 * @fn          HalLM75ATempTurnOn
 *
 * @brief       Turn the sensor on
 *
 * @return      none
 **************************************************************************************************/
void HalLM75ATempTurnOn(uint8 id)
{
  //HalDcDcControl(ST_IRTEMP,true);
  HalLM75ATempSelect(id);

  if (HalSensorWriteReg(LM75A_REG_ADDR_CONFIG, configLM75AOn, 1))
  {
    irtSensorState = LM75A_NORMAL;
  }
}

/**************************************************************************************************
 * @fn          HalLM75ATempTurnOff
 *
 * @brief       Turn the sensor off
 *
 * @return      none
 **************************************************************************************************/
void HalLM75ATempTurnOff(uint8 id)
{
  HalLM75ATempSelect(id);

  if (HalSensorWriteReg(LM75A_REG_ADDR_CONFIG, configLM75AOff, 1))
  {
    irtSensorState = LM75A_OFF;
  }
  //HalDcDcControl(ST_IRTEMP,false);
}

/**************************************************************************************************
 * @fn          HalLM75ATempRead
 *
 * @brief       Read the sensor voltage and sensor temperature registers
 *
 * @param       Voltage and temperature in raw format (2 + 2 bytes)
 *
 * @return      TRUE if valid data
 **************************************************************************************************/
bool HalLM75ATempRead(uint8 id, uint8 *pBuf)
{
  uint16 t = 0;
  bool success;
  uint8 temp[2] = {0};
/*
  if (irtSensorState != LM75A_NORMAL)
  {
    return FALSE;
  }
*/
  HalLM75ATempSelect(id);
  HalSensorWriteReg(LM75A_REG_ADDR_CONFIG, configLM75AOn, 1);
  ST_HAL_DELAY(12500);//100ms
  // Read the sensor registers

  success = HalSensorReadReg(LM75A_REG_ADDR_TEMPERATURE, temp, 2 );

  if (success)
  {
    // Store values
#if defined(MOVE_RIGHT5)
    t = BUILD_UINT16(temp[1], temp[0]);
    t = t >> 5;
    pBuf[0] = HI_UINT16( t );
    pBuf[1] = LO_UINT16( t );
#else
    pBuf[0] = temp[1];
    pBuf[1] = temp[0];
#endif
  }

  // Turn off sensor
  if (HalSensorWriteReg(LM75A_REG_ADDR_CONFIG, configLM75AOff, 1))
  {
    irtSensorState = LM75A_OFF;
  }
  //HalDcDcControl(ST_IRTEMP,false);

  return success;
}

/**************************************************************************************************
 * @fn          HalLM75ATempReadAll
 *
 * @brief       Read all LM75A temperature
 *
 * @param       Temperature in raw format (8*2=16 bytes)
 *
 * @return      0 if valid data
 **************************************************************************************************/
int8 HalLM75ATempReadAll(uint8 *pBuf)
{
    uint16 t = 0;
    uint8 temp[2] = {0};
    uint8 *p = pBuf;
    bool success;
    for (int id = 0; id < LM75A_NUMBER; id++)
    {
        HalLM75ATempSelect(id);
        HalSensorWriteReg(LM75A_REG_ADDR_CONFIG, configLM75AOn, 1);
        ST_HAL_DELAY(12500);
        //success = HalSensorReadReg(LM75A_REG_ADDR_TEMPERATURE, (uint8 *)&t, 2 );
        success = HalSensorReadReg(LM75A_REG_ADDR_TEMPERATURE, temp, 2 );
        if (success)
        {
        #if defined(MOVE_RIGHT5)
            t = BUILD_UINT16(temp[1], temp[0]);
            t = t >> 5;
            *p = HI_UINT16( t );
            *(p+1) = LO_UINT16( t );
        #else
            *p = temp[1];
            *(p + 1) = temp[0];
        #endif
        }
        p += 2;
        HalSensorWriteReg(LM75A_REG_ADDR_CONFIG, configLM75AOff, 1);
        t = 0;
    }
    return 0;
}

/**************************************************************************************************
 * @fn          HalLM75ATempStatus
 *
 * @brief       Read the state of the sensor
 *
 * @return      none
 **************************************************************************************************/
LM75ATemperature_States_t HalLM75ATempStatus(uint8 id)
{
    return irtSensorState;
}


/**************************************************************************************************
 * @fn          HalIRTempTest
 *
 * @brief       Run a sensor self-test
 *
 * @return      TRUE if passed, FALSE if failed
 **************************************************************************************************/
/*
bool HalLM75ATempTest(void)
{
  uint16 val;

  // Select this sensor on the I2C bus
  HalIRTempSelect();

  // Check manufacturer ID
  ST_ASSERT(HalSensorReadReg(TMP006_REG_MANF_ID, (uint8 *)&val, IRTEMP_REG_LEN));
  val = (LO_UINT16(val) << 8) | HI_UINT16(val);
  ST_ASSERT(val == TMP006_VAL_MANF_ID);

  // Reset sensor
  ST_ASSERT(HalSensorWriteReg(TMP006_REG_ADDR_CONFIG, configSensorReset, IRTEMP_REG_LEN));

  // Check config register (reset)
  ST_ASSERT(HalSensorReadReg(TMP006_REG_ADDR_CONFIG, (uint8 *)&val, IRTEMP_REG_LEN));
  val = ((LO_UINT16(val) << 8) | HI_UINT16(val));
  ST_ASSERT(val == TMP006_VAL_CONFIG_RESET);

  // Turn sensor off
  ST_ASSERT(HalSensorWriteReg(TMP006_REG_ADDR_CONFIG, configSensorOff,IRTEMP_REG_LEN));

  // Check config register (off)
  ST_ASSERT(HalSensorReadReg(TMP006_REG_ADDR_CONFIG, (uint8 *)&val, IRTEMP_REG_LEN));
  val = ((LO_UINT16(val) << 8) | HI_UINT16(val));
  ST_ASSERT(val == TMP006_VAL_CONFIG_OFF);

  // Turn sensor on
  ST_ASSERT(HalSensorWriteReg(TMP006_REG_ADDR_CONFIG, configSensorOn, IRTEMP_REG_LEN));

  // Check config register (on)
  ST_ASSERT(HalSensorReadReg(TMP006_REG_ADDR_CONFIG, (uint8 *)&val, IRTEMP_REG_LEN));
  val = ((LO_UINT16(val) << 8) | HI_UINT16(val));
  ST_ASSERT(val == TMP006_VAL_CONFIG_ON);

  // Turn sensor off
  ST_ASSERT(HalSensorWriteReg(TMP006_REG_ADDR_CONFIG, configSensorOff, IRTEMP_REG_LEN));

  return TRUE;
}
*/

/* ------------------------------------------------------------------------------------------------
*                                           Private functions
* -------------------------------------------------------------------------------------------------
*/

/**************************************************************************************************
 * @fn          HalIRTempSelect
 *
 * @brief       Select the TMP006 slave and set the I2C bus speed
 *
 * @param       0~7
 *
 * @return      none
 **************************************************************************************************/
static void HalLM75ATempSelect(uint8 id)
{
  // Select slave and set clock rate
    switch (id)
    {
        case 0:
            HalI2CInit(LM75A_I2C_ADDRESS0, i2cClock_533KHZ);
            break;

        case 1:
            HalI2CInit(LM75A_I2C_ADDRESS1, i2cClock_533KHZ);
            break;

        case 2:
            HalI2CInit(LM75A_I2C_ADDRESS2, i2cClock_533KHZ);
            break;

        case 3:
            HalI2CInit(LM75A_I2C_ADDRESS3, i2cClock_533KHZ);
            break;

        case 4:
            HalI2CInit(LM75A_I2C_ADDRESS4, i2cClock_533KHZ);
            break;

        case 5:
            HalI2CInit(LM75A_I2C_ADDRESS5, i2cClock_533KHZ);
            break;

        case 6:
            HalI2CInit(LM75A_I2C_ADDRESS6, i2cClock_533KHZ);
            break;

        case 7:
            HalI2CInit(LM75A_I2C_ADDRESS7, i2cClock_533KHZ);
            break;

        default:
            HalI2CInit(LM75A_I2C_ADDRESS0, i2cClock_533KHZ);
            break;
    }
}

/*  Conversion algorithm for die temperature
 *  ================================================
 *
double calcTmpLocal(uint16 rawT)
{
    //-- calculate die temperature [°C] --
    m_tmpAmb = (double)((qint16)rawT)/128.0;

    return m_tmpAmb;
}

*
* Conversion algorithm for target temperature
*
double calcTmpTarget(uint16 rawT)
{
    //-- calculate target temperature [°C] -
    double Vobj2 = (double)(qint16)rawT;
    Vobj2 *= 0.00000015625;

    double Tdie2 = m_tmpAmb + 273.15;
    const double S0 = 6.4E-14;            // Calibration factor

    const double a1 = 1.75E-3;
    const double a2 = -1.678E-5;
    const double b0 = -2.94E-5;
    const double b1 = -5.7E-7;
    const double b2 = 4.63E-9;
    const double c2 = 13.4;
    const double Tref = 298.15;
    double S = S0*(1+a1*(Tdie2 - Tref)+a2*pow((Tdie2 - Tref),2));
    double Vos = b0 + b1*(Tdie2 - Tref) + b2*pow((Tdie2 - Tref),2);
    double fObj = (Vobj2 - Vos) + c2*pow((Vobj2 - Vos),2);
    double tObj = pow(pow(Tdie2,4) + (fObj/S),.25);
    tObj = (tObj - 273.15);

    return tObj;
}

*/

/*********************************************************************
*********************************************************************/