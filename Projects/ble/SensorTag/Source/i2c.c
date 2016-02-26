/**************************************************************************************************
  Filename:       i2c.c
  Revised:        $Date: 2015-06-10 10:13:31 +0800 (Wed, 10 Jun 2015) $
  Revision:       $Revision: 1 $

**************************************************************************************************/

#include "hal_mcu.h"
#include "i2c.h"

//#define MOVE_RIGHT5_EGG

/* GPIO setting */
#define SDA_GPIO P0_4
#define SCL_GPIO P0_1

/* set GPIO function as general output/input */
#define SET_SDA_FN_0  (P0SEL &= 0xEF)
#define SET_SCL_FN_0  (P0SEL &= 0xFD)

// Port Direction,  0: Input,  1: Output
//P0DIR = 0xFF;
//P1DIR = 0xFF;
//P2DIR = 0x1F;
#define SET_SDA_OUTPUT  (P0DIR |= 0x10)
#define SET_SDA_INPUT   (P0DIR &= 0xEF)
#define SET_SCL_OUTPUT  (P0DIR |= 0x02)

/* read SDA status */
#define GET_SDA_VALUE  SDA_GPIO

/* pull SDA/SCL high or low */
#define SEND_SDA_VALUE(x)
#define PULL_SDA_LOW      (SDA_GPIO = 0)
#define PULL_SDA_HIGH     (SDA_GPIO = 1)
#define PULL_SCL_LOW      (SCL_GPIO = 0)
#define PULL_SCL_HIGH     (SCL_GPIO = 1)

/* delay */
#define I2C_DUMMY_DELAY  \
do{\
   uint16  i2c_dummy_cnt; \
   for (i2c_dummy_cnt = 5; i2c_dummy_cnt!=0; i2c_dummy_cnt--); \
}while(0)

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

/* Ative delay: 125 cycles ~1 msec */
#define ST_HAL_DELAY(n) st( { volatile uint32 i; for (i=0; i<(n); i++) { }; } )

static uint8 configLM75AOff[1] = {0x01};    // LM75A shutdown
static uint8 configLM75AOn[1] =  {0x00};    // LM75A normal
static uint8 gI2CAddress = 0x48;
static uint8 buffer[24];

static void delay_nus(uint16 timeout);
static void EggLM75ATempSelect(uint8 id);
static void I2C_Stop(void);
static bool EggReadReg(uint8 addr, uint8 *pBuf, uint8 nBytes);
static void EggTurnOffLM75A(void);

void EggLM75ATempInit(void)
{
    for (int i = 0; i < LM75A_NUMBER; i++)
    {
        EggLM75ATempSelect(i);
        EggTurnOffLM75A();
    }
}

void EggTurnOnLM75A(void)
{
    int8 i, ack, data;
    SET_SCL_OUTPUT;
    SET_SDA_OUTPUT;

    PULL_SCL_HIGH;
    PULL_SDA_HIGH;
    I2C_DUMMY_DELAY;

    PULL_SDA_LOW;
    I2C_DUMMY_DELAY;

    PULL_SCL_LOW;
    I2C_DUMMY_DELAY;

    data = gI2CAddress << 1;
    // send 8bits data, MSB first
    for (i = 8; --i >= 0; )
    {
        if ((data >> i) & 0x01)
        {
            PULL_SDA_HIGH;
        }
        else
        {
            PULL_SDA_LOW;
        }
        I2C_DUMMY_DELAY;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }

    // receive ack
    SET_SDA_INPUT;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    ack = GET_SDA_VALUE; // read ack when HIGH period of the SCL
    PULL_SCL_LOW;
    SET_SDA_OUTPUT;

    data = LM75A_REG_ADDR_CONFIG;
    // send 8bits data, MSB first
    for (i = 8; --i >= 0; )
    {
        if ((data >> i) & 0x01)
        {
            PULL_SDA_HIGH;
        }
        else
        {
            PULL_SDA_LOW;
        }
        I2C_DUMMY_DELAY;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }
    // receive ack
    SET_SDA_INPUT;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    ack = GET_SDA_VALUE; // read ack when HIGH period of the SCL
    PULL_SCL_LOW;
    SET_SDA_OUTPUT;

    data = configLM75AOn[0];
    // send 8bits data, MSB first
    for (i = 8; --i >= 0; )
    {
        if ((data >> i) & 0x01)
        {
            PULL_SDA_HIGH;
        }
        else
        {
            PULL_SDA_LOW;
        }
        I2C_DUMMY_DELAY;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }
    // receive ack
    SET_SDA_INPUT;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    ack = GET_SDA_VALUE; // read ack when HIGH period of the SCL
    PULL_SCL_LOW;
    SET_SDA_OUTPUT;

    I2C_Stop();
}

static void EggTurnOffLM75A(void)
{
    int8 i, ack, data;
    SET_SCL_OUTPUT;
    SET_SDA_OUTPUT;

    PULL_SCL_HIGH;
    PULL_SDA_HIGH;
    I2C_DUMMY_DELAY;

    PULL_SDA_LOW;
    I2C_DUMMY_DELAY;

    PULL_SCL_LOW;
    I2C_DUMMY_DELAY;

    data = gI2CAddress << 1;
    // send 8bits data, MSB first
    for (i = 8; --i >= 0; )
    {
        if ((data >> i) & 0x01)
        {
            PULL_SDA_HIGH;
        }
        else
        {
            PULL_SDA_LOW;
        }
        I2C_DUMMY_DELAY;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }

    // receive ack
    SET_SDA_INPUT;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    ack = GET_SDA_VALUE; // read ack when HIGH period of the SCL
    PULL_SCL_LOW;
    SET_SDA_OUTPUT;

    data = LM75A_REG_ADDR_CONFIG;
    // send 8bits data, MSB first
    for (i = 8; --i >= 0; )
    {
        if ((data >> i) & 0x01)
        {
            PULL_SDA_HIGH;
        }
        else
        {
            PULL_SDA_LOW;
        }
        I2C_DUMMY_DELAY;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }
    // receive ack
    SET_SDA_INPUT;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    ack = GET_SDA_VALUE; // read ack when HIGH period of the SCL
    PULL_SCL_LOW;
    SET_SDA_OUTPUT;

    data = configLM75AOff[0];
    // send 8bits data, MSB first
    for (i = 8; --i >= 0; )
    {
        if ((data >> i) & 0x01)
        {
            PULL_SDA_HIGH;
        }
        else
        {
            PULL_SDA_LOW;
        }
        I2C_DUMMY_DELAY;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }
    // receive ack
    SET_SDA_INPUT;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    ack = GET_SDA_VALUE; // read ack when HIGH period of the SCL
    PULL_SCL_LOW;
    SET_SDA_OUTPUT;

    I2C_Stop();
}

bool EggLM75ATempRead(uint8 id, uint8 *pBuf)
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
    EggLM75ATempSelect(id);
    //HalSensorWriteReg(LM75A_REG_ADDR_CONFIG, configLM75AOn, 1);
    EggTurnOnLM75A();
    ST_HAL_DELAY(12500);
    // Read the sensor registers

    //success = HalSensorReadReg(LM75A_REG_ADDR_TEMPERATURE, temp, 2 );
    success = EggReadReg(LM75A_REG_ADDR_TEMPERATURE, temp, 2 );
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
    /*
    if (HalSensorWriteReg(LM75A_REG_ADDR_CONFIG, configLM75AOff, 1))
    {
      irtSensorState = LM75A_OFF;
    }
    */
    EggTurnOffLM75A();
    //HalDcDcControl(ST_IRTEMP,false);

    return success;
}
int8 EggReadAllLM75ATemp(uint8 *pBuf)
{
    uint16 t = 0;
    uint8 temp[2] = {0};
    uint8 *p = pBuf;
    bool success;
    for (int id = 0; id < LM75A_NUMBER; id++)
    {
        EggLM75ATempSelect(id);
        EggTurnOnLM75A();
        ST_HAL_DELAY(12500);
        //success = HalSensorReadReg(LM75A_REG_ADDR_TEMPERATURE, (uint8 *)&t, 2 );
        success = EggReadReg(LM75A_REG_ADDR_TEMPERATURE, temp, 2 );
        if (success)
        {
#if defined(MOVE_RIGHT5_EGG)
            t = BUILD_UINT16(temp[1], temp[0]);
            t = t >> 5;
            *p = HI_UINT16( t );
            *(p + 1) = LO_UINT16( t );
#else
            *p = temp[1];
            *(p + 1) = temp[0];
#endif
        }
        p += 2;
        EggTurnOffLM75A();
        t = 0;
    }
    return 0;
}

/**************************************************************************************************
 * @fn          EggSensorReadReg
 *
 * @brief       This function implements the I2C protocol to read from a sensor. The sensor must
 *              be selected before this routine is called.
 *
 * @param       addr - which register to read
 * @param       pBuf - pointer to buffer to place data
 * @param       nBytes - numbver of bytes to read
 *
 * @return      TRUE if the required number of bytes are reveived
 **************************************************************************************************/
bool EggSensorReadReg(uint8 addr, uint8 *pBuf, uint8 nBytes)
{
    uint8 i = 0;

    return i == nBytes;
}

static bool EggReadReg(uint8 addr, uint8 *pBuf, uint8 nBytes)
{
    //uint8 i = 0;
    int8 i, ack, data;
    SET_SCL_OUTPUT;
    SET_SDA_OUTPUT;

    PULL_SCL_HIGH;
    PULL_SDA_HIGH;
    I2C_DUMMY_DELAY;

    PULL_SDA_LOW;
    I2C_DUMMY_DELAY;

    PULL_SCL_LOW;
    I2C_DUMMY_DELAY;

    data = gI2CAddress << 1;
    // send 8bits data, MSB first
    for (i = 8; --i >= 0; )
    {
        if ((data >> i) & 0x01)
        {
            PULL_SDA_HIGH;
        }
        else
        {
            PULL_SDA_LOW;
        }
        I2C_DUMMY_DELAY;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }

    // receive ack
    SET_SDA_INPUT;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    ack = GET_SDA_VALUE; // read ack when HIGH period of the SCL
    PULL_SCL_LOW;
    SET_SDA_OUTPUT;

    data = LM75A_REG_ADDR_TEMPERATURE;
    // send 8bits data, MSB first
    for (i = 8; --i >= 0; )
    {
        if ((data >> i) & 0x01)
        {
            PULL_SDA_HIGH;
        }
        else
        {
            PULL_SDA_LOW;
        }
        I2C_DUMMY_DELAY;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }
    // receive ack
    SET_SDA_INPUT;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    ack = GET_SDA_VALUE; // read ack when HIGH period of the SCL
    PULL_SCL_LOW;
    SET_SDA_OUTPUT;
    I2C_DUMMY_DELAY;
    //To re-start
    PULL_SCL_HIGH;
    PULL_SDA_HIGH;
    I2C_DUMMY_DELAY;
    PULL_SDA_LOW;
    I2C_DUMMY_DELAY;
    PULL_SCL_LOW;
    I2C_DUMMY_DELAY;
    ///////////////////////////////////////////////////////////////////////////////
    data = gI2CAddress << 1;
    data |= 0x01;
    // send 8bits data, MSB first
    for (i = 8; --i >= 0; )
    {
        if ((data >> i) & 0x01)
        {
            PULL_SDA_HIGH;
        }
        else
        {
            PULL_SDA_LOW;
        }
        I2C_DUMMY_DELAY;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }

    // receive ack
    SET_SDA_INPUT;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    ack = GET_SDA_VALUE; // read ack when HIGH period of the SCL
    PULL_SCL_LOW;

    // To read
    uint32 dataCache = 0;
    SET_SDA_INPUT;

    // receive 8bits data
    for(i = 8; --i >= 0; )
    {
        dataCache <<= 1;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        dataCache |= GET_SDA_VALUE;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }

    SET_SDA_OUTPUT;
    // send ack
    PULL_SDA_LOW;

    I2C_DUMMY_DELAY;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    PULL_SCL_LOW;
    pBuf[0] = (uint8)dataCache;

    I2C_DUMMY_DELAY;
    // To read the second byte
    dataCache = 0;
    SET_SDA_INPUT;

    // receive 8bits data
    for(i = 8; --i >= 0; )
    {
        dataCache <<= 1;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        dataCache |= GET_SDA_VALUE;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }

    SET_SDA_OUTPUT;
    // send ack
    PULL_SDA_HIGH;

    I2C_DUMMY_DELAY;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    PULL_SCL_LOW;

    pBuf[1] = (uint8)dataCache;

    I2C_Stop();
    return TRUE;
}

/**************************************************************************************************
* @fn          EggSensorWriteReg
* @brief       This function implements the I2C protocol to write to a sensor. The sensor must
*              be selected before this routine is called.
*
* @param       addr - which register to write
* @param       pBuf - pointer to buffer containing data to be written
* @param       nBytes - number of bytes to write
*
* @return      TRUE if successful write
*/
static bool EggSensorWriteReg(uint8 addr, uint8 *pBuf, uint8 nBytes)
{
    uint8 i;

    return (i == nBytes);
}

static void EggLM75ATempSelect(uint8 id)
{
    // Select slave and set clock rate
    switch (id)
    {
    case 0:
        //HalI2CInit(LM75A_I2C_ADDRESS0, i2cClock_533KHZ);
        gI2CAddress = LM75A_I2C_ADDRESS0;
        break;

    case 1:
        gI2CAddress = LM75A_I2C_ADDRESS1;
        break;

    case 2:
        gI2CAddress = LM75A_I2C_ADDRESS2;
        break;

    case 3:
        gI2CAddress = LM75A_I2C_ADDRESS3;
        break;

    case 4:
        gI2CAddress = LM75A_I2C_ADDRESS4;
        break;

    case 5:
        gI2CAddress = LM75A_I2C_ADDRESS5;
        break;

    case 6:
        gI2CAddress = LM75A_I2C_ADDRESS6;
        break;

    case 7:
        gI2CAddress = LM75A_I2C_ADDRESS7;
        break;

    default:
        gI2CAddress = LM75A_I2C_ADDRESS0;
        break;
    }
}
/*************************************************************************
* FUNCTION
* I2C_Init
*
* DESCRIPTION
* Prepare to control I2C bus, set SCL, SDA normal GPIO mode.
*
* PARAMETERS
* None.
*
* RETURNS
* None
*
*************************************************************************/
void I2C_Init(void)
{
    /* set SCL, SDA normal GPIO mode */
    SET_SDA_FN_0;
    SET_SCL_FN_0;
}

/*************************************************************************
* FUNCTION
*     I2C_Release
*
* DESCRIPTION
*     Release I2C bus.
*
* PARAMETERS
*     None.
*
* RETURNS
*     None
*
*************************************************************************/
void I2C_Release(void)
{
    /* set SCL, SDA high */
    PULL_SCL_HIGH;
    PULL_SDA_HIGH;
}

/*************************************************************************
* FUNCTION
*     I2C_Start
*
* DESCRIPTION
*     Initiate a START condition.
*     A High to Low transition on the SDA line while SCL is High defines a START condition.
*
* SCL --- ---
*            |___
* SDA ---
*        |___ ___
*
* PARAMETERS
*     None.
*
* RETURNS
*     None
*
*************************************************************************/
void I2C_Start(void)
{
    SET_SCL_OUTPUT;
    SET_SDA_OUTPUT;

    PULL_SCL_HIGH;
    PULL_SDA_HIGH;
    I2C_DUMMY_DELAY;

    PULL_SDA_LOW;
    I2C_DUMMY_DELAY;

    PULL_SCL_LOW;
    I2C_DUMMY_DELAY;
}

/*************************************************************************
* FUNCTION
*     I2C_Stop
*
* DESCRIPTION
*     Generate a Stop condition.
*     A Low to High transition on the SDA line while SCL is High defines a START condition.
*
* SCL     --- ---
*     ___|
* SDA         ---
*     ___ ___|
*
* PARAMETERS
*     None.
*
* RETURNS
*     None
*
*************************************************************************/
static void I2C_Stop(void)
{
    SET_SCL_OUTPUT;
    SET_SDA_OUTPUT;

    PULL_SCL_LOW;
    PULL_SDA_LOW;
    I2C_DUMMY_DELAY;

    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;

    PULL_SDA_HIGH;
    I2C_DUMMY_DELAY;
}

/*************************************************************************
* FUNCTION
*     I2C_TxByte
*
* DESCRIPTION
*     Send a byte to I2C bus.
*
* PARAMETERS
*     @uint8 data  data to be send.
*
* RETURNS
*     ack received from I2C slave device.
*
*************************************************************************/
uint8 I2C_TxByte(uint8 data)
{
    int8 i, ack;
    SET_SCL_OUTPUT;
    SET_SDA_OUTPUT;

    // send 8bits data, MSB first
    for(i = 8; --i >= 0; )
    {
        if((data >> i) & 0x01)
            PULL_SDA_HIGH;
        else
            PULL_SDA_LOW;
        I2C_DUMMY_DELAY;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }

    // receive ack
    SET_SDA_INPUT;

    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    ack = GET_SDA_VALUE; // read ack when HIGH period of the SCL
    PULL_SCL_LOW;

    SET_SDA_OUTPUT;
    return ack;
}

/*************************************************************************
* FUNCTION
*     I2C_RxByte
*
* DESCRIPTION
*     Receive a byte from I2C bus.
*
* PARAMETERS
*     @uint8 *data  store received data.
*     @uint8 ack  ack send to I2C slave device after receive a byte.
*                        ACK -- 0
*                        NACK -- 1
* RETURNS
*     None.
*
*************************************************************************/
void I2C_RxByte(uint8 *data, uint8 ack)
{
    int16 i;
    uint32 dataCache;
    dataCache = 0;

    SET_SDA_INPUT;

    // receive 8bits data
    for(i = 8; --i >= 0; )
    {
        dataCache <<= 1;
        PULL_SCL_HIGH;
        I2C_DUMMY_DELAY;
        dataCache |= GET_SDA_VALUE;
        PULL_SCL_LOW;
        I2C_DUMMY_DELAY;
    }

    SET_SDA_OUTPUT;
    // send ack
    if(ack)
        PULL_SDA_HIGH;
    else
        PULL_SDA_LOW;
    I2C_DUMMY_DELAY;
    PULL_SCL_HIGH;
    I2C_DUMMY_DELAY;
    PULL_SCL_LOW;

    *data = (uint8)dataCache;
}

/*
 *    ÑÓÊ±º¯Êý
 *    ÊäÈëÎ¢Ãî
 */
static void delay_nus(uint16 timeout)
{
    while (timeout--)
    {
        asm("NOP");
        asm("NOP");
        asm("NOP");
    }
}