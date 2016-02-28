/**************************************************************************************************
  Filename:       sensorTag.c
  Revised:        $Date: 2015-05-19 11:43:49 -0700 (Tue, 19 May 2015) $
  Revision:       $Revision: 43850 $

  Description:    This file contains the Sensor Tag sample application
                  for use with the TI Bluetooth Low Energy Protocol Stack.

  Copyright 2012-2013  Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED �AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OSAL_Tasks.h"
#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_keys.h"
#include "hal_i2c.h"

#include "gatt.h"
#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"

#if defined ( PLUS_BROADCASTER )
#include "peripheralBroadcaster.h"
#else
#include "peripheral.h"
#endif

#include "gapbondmgr.h"

#if defined FEATURE_OAD
#include "oad.h"
#include "oad_target.h"
#endif

// Services
#include "st_util.h"
#include "devinfoservice-st.h"
#include "irtempservice.h"
#include "accelerometerservice.h"
#include "humidityservice.h"
#include "magnetometerservice.h"
#include "barometerservice.h"
#include "gyroservice.h"
#if defined FEATURE_TEST
#include "testservice.h"
#endif
#include "simplekeys.h"
#include "ccservice.h"

// Sensor drivers
#include "sensorTag.h"
#include "hal_sensor.h"

#include "hal_irtemp.h"
#include "hal_LM75A_nxp.h"
#include "hal_acc.h"
#include "hal_humi.h"
#include "hal_mag.h"
#include "hal_bar.h"
#include "hal_gyro.h"
#include "MPU6050_6Axis_MotionApps20Egg.h"
#include "i2c.h"
#include "OSAL_Clock.h"
#include "md_profile.h"
/*********************************************************************
 * MACROS
 */
#define MPU6050_DATA_LEN          14
/*********************************************************************
 * CONSTANTS
 */

// How often to perform sensor reads (milliseconds)
#define TEMP_DEFAULT_PERIOD                   20000
#define HUM_DEFAULT_PERIOD                    70000
#define MPU6050_DEFAULT_PERIOD                2000
//#define BAR_DEFAULT_PERIOD                    1000
//#define MAG_DEFAULT_PERIOD                    2000
//#define ACC_DEFAULT_PERIOD                    1000
//#define GYRO_DEFAULT_PERIOD                   1000

// Constants for two-stage reading
#define TEMP_MEAS_DELAY                       275   // Conversion time 250 ms
#define BAR_FSM_PERIOD                        80
#define ACC_FSM_PERIOD                        20
#define HUM_FSM_PERIOD                        100
#define GYRO_STARTUP_TIME                     60    // Start-up time max. 50 ms

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          1600

// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     280

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     296

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         3

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          600

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         8

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

#if defined ( PLUS_BROADCASTER )
#define ADV_IN_CONN_WAIT                    500 // delay 500 ms
#endif

// Side key bit
#define SK_KEY_SIDE                           0x04

// Test mode bit
#define TEST_MODE_ENABLE                      0x80

// Common values for turning a sensor on and off + config/status
#define ST_CFG_SENSOR_DISABLE                 0x00
#define ST_CFG_SENSOR_ENABLE                  0x01
#define ST_CFG_CALIBRATE                      0x02
#define ST_CFG_ERROR                          0xFF

// System reset
#define ST_SYS_RESET_DELAY                    3000

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 sensorTag_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
    // complete name
    0x0B,   // length of this data
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    0x56,   // 'V'
    0x75,   // 'u'
    0x6C,   // 'l'
    0x74,   // 't'
    0x75,   // 'u'
    0x72,   // 'r'
    0x65,   // 'e'
    0x45,   // 'E'
    0x67,   // 'g'
    0x67,   // 'g'

    // connection interval range
    0x05,   // length of this data
    GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
    LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
    HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
    LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),
    HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

    // Tx power level
    0x02,   // length of this data
    GAP_ADTYPE_POWER_LEVEL,
    0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
    0x02,   // length of first data structure (2 bytes excluding length byte)
    GAP_ADTYPE_FLAGS,   // AD Type = Flags
    //0x1a,
    GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // service UUID, to notify central devices what services are included
    // in this peripheral
    0x1B,   // length of second data structure (7 bytes excluding length byte)
    0xff, 0x4c, 0x00, 0x02, 0x15,
    0xFD, 0xA5, 0x06, 0x93, 0xA4, 0xE2, 0x4F, 0xB1, 0xAF, 0xCF, 0xC6, 0xEB, 0x07, 0x64, 0x78, 0x25,
    0x27, 0x18,
    0x6A, 0x7F,
    0xc5,
    0xBB,
};
// GAP GATT Attributes
static uint8 attDeviceName[] = "VultureEgg";

// Sensor State Variables
//static bool   irTempEnabled = FALSE;
//static bool   magEnabled = FALSE;
//static uint8  accConfig = ST_CFG_SENSOR_DISABLE;
//static uint8  mpu6050Config = ST_CFG_SENSOR_DISABLE;
//static bool   barEnabled = FALSE;
static bool   humiEnabled = FALSE;
//static bool   gyroEnabled = FALSE;
static bool   lm75Enabled = FALSE;
static bool   mpu6050Enabled = FALSE;

typedef enum
{
    EGG_STATE_MEASURE_IDLE,
    EGG_STATE_MEASURE_HUMIDITY,
    EGG_STATE_MEASURE_LM75A,
    EGG_STATE_MEASURE_MPU6050
} t_enum_EggState;
static t_enum_EggState gEggState = EGG_STATE_MEASURE_IDLE; // 0 : Measure humidity; 1 : Measure DS18B20; 2 : Measure MPU6050
//static bool   barBusy = FALSE;
static uint8  humiState = 0;

static bool   sysResetRequest = FALSE;

//static uint16 sensorMagPeriod = MAG_DEFAULT_PERIOD;
//static uint16 sensorAccPeriod = ACC_DEFAULT_PERIOD;
static uint32 sensorTmpPeriod = TEMP_DEFAULT_PERIOD;
static uint32 sensorHumPeriod = HUM_DEFAULT_PERIOD;
//static uint16 sensorBarPeriod = BAR_DEFAULT_PERIOD;
//static uint16 sensorGyrPeriod = GYRO_DEFAULT_PERIOD;
static uint16 sensorMpu6050Period = MPU6050_DEFAULT_PERIOD;
//static uint8  sensorGyroAxes = 0;
//static bool   sensorGyroUpdateAxes = FALSE;
static uint16 selfTestResult = 0;
static bool   testMode = FALSE;

static uint16 packetSize = 42;    // expected DMP packet size (default is 42 bytes)
static uint16 fifoCount;     // count of all bytes currently in FIFO
static uint8 fifoBuffer[64]; // FIFO storage buffer
static uint8 mpuIntStatus;

static uint8 gsendbuffer[48];
//static uint8 gsendbuffer2[48];
static uint8 gLM75ACounter = 0;
static uint8 gsendbufferI;
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void sensorTag_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );

//static void readIrTempData( void );
static void readHumData( void );
//static void readAccData( void );
//static void readMagData( void );
//static void readBarData( void );
//static void readBarCalibration( void );
//static void readGyroData( void );

//static void barometerChangeCB( uint8 paramID );
//static void irTempChangeCB( uint8 paramID );
//static void accelChangeCB( uint8 paramID );
//static void humidityChangeCB( uint8 paramID);
//static void magnetometerChangeCB( uint8 paramID );
//static void gyroChangeCB( uint8 paramID );
#if defined FEATURE_TEST
static void testChangeCB( uint8 paramID );
#endif
static void ccChangeCB( uint8 paramID );
static void ccUpdate( void );
static void gapRolesParamUpdateCB( uint16 connInterval, uint16 connSlaveLatency,
                                   uint16 connTimeout );

static void md_ProfileChangeCB( uint8 paramID );
static void resetSensorSetup( void );
static void sensorTag_HandleKeys( uint8 shift, uint8 keys );
#if 0
static void resetCharacteristicValue( uint16 servID, uint8 paramID, uint8 value,
                                      uint8 paramLen );
static void resetCharacteristicValues( void );
#endif
static void resolve_command(void);
static void readMPU6050DmpData( uint8 *packet );

//static void eggSerialAppSendNoti(uint8 *pBuffer, uint16 length);
/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t sensorTag_PeripheralCBs =
{
    peripheralStateNotificationCB,  // Profile State Change Callbacks
    NULL                            // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t sensorTag_BondMgrCBs =
{
    NULL,                     // Passcode callback (not used by application)
    NULL                      // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
#if 0
static sensorCBs_t sensorTag_BarometerCBs =
{
    barometerChangeCB,        // Characteristic value change callback
};

static sensorCBs_t sensorTag_IrTempCBs =
{
    irTempChangeCB,           // Characteristic value change callback
};

static sensorCBs_t sensorTag_AccelCBs =
{
    accelChangeCB,            // Characteristic value change callback
};

static sensorCBs_t sensorTag_HumidCBs =
{
    humidityChangeCB,         // Characteristic value change callback
};

static sensorCBs_t sensorTag_MagnetometerCBs =
{
    magnetometerChangeCB,     // Characteristic value change callback
};

static sensorCBs_t sensorTag_GyroCBs =
{
    gyroChangeCB,             // Characteristic value change callback
};

#if defined FEATURE_TEST
static testCBs_t sensorTag_TestCBs =
{
    testChangeCB,             // Charactersitic value change callback
};
#endif
#endif

static ccCBs_t sensorTag_ccCBs =
{
    ccChangeCB,               // Charactersitic value change callback
};

static gapRolesParamUpdateCB_t paramUpdateCB =
{
    gapRolesParamUpdateCB,
};

static const mdProfileCBs_t MDProfileCB =
{
    md_ProfileChangeCB
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SensorTag_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SensorTag_Init( uint8 task_id )
{
    sensorTag_TaskID = task_id;

    // Setup the GAP
    VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );

    // Setup the GAP Peripheral Role Profile
    {
        // Device starts advertising upon initialization
        uint8 initial_advertising_enable = TRUE;

        // By setting this to zero, the device will go into the waiting state after
        // being discoverable for 30.72 second, and will not being advertising again
        // until the enabler is set back to TRUE
        uint16 gapRole_AdvertOffTime = 1;
        uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
        uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
        uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
        uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
        uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

        // Set the GAP Role Parameters
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
        GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );
        GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
        GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

        GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
        GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
        GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
        GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
        GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
    }

    // Set the GAP Characteristics
    GGS_SetParameter( GGS_DEVICE_NAME_ATT, sizeof(attDeviceName), attDeviceName );

    // Set advertising interval
    {
        uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

        GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
        GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
        GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
        GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
    }

    // Setup the GAP Bond Manager
    {
        uint32 passkey = 0; // passkey "000000"
        uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
        uint8 mitm = TRUE;
        uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
        uint8 bonding = TRUE;

        GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
        GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
        GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
        GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
        GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
    }


    // Add services
    GGS_AddService( GATT_ALL_SERVICES );            // GAP
    GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
    DevInfo_AddService();                           // Device Information Service
#if 0
    IRTemp_AddService();                            // IR Temperature Service
    Accel_AddService();                             // Accelerometer Service
    Humidity_AddService();                          // Humidity Service
    Magnetometer_AddService();                      // Magnetometer Service
    Barometer_AddService();                         // Barometer Service
    Gyro_AddService();                              // Gyro Service
    SK_AddService( GATT_ALL_SERVICES );             // Simple Keys Profile
#if defined FEATURE_TEST
    Test_AddService();                              // Test Profile
#endif
#endif
    CcService_AddService();                         // Connection Control Service
    MDProfile_AddService();
#if defined FEATURE_OAD
    VOID OADTarget_AddService();                    // OAD Profile
#endif
#if 0
    // Setup the Seensor Profile Characteristic Values
    resetCharacteristicValues();

    // Register for all key events - This app will handle all key events
    RegisterForKeys( sensorTag_TaskID );
#endif
    // makes sure LEDs are off
    HalLedSet( (HAL_LED_1 | HAL_LED_2), HAL_LED_MODE_OFF );
    HalHumiInit();
#if 0
    // Initialise sensor drivers
    HALIRTempInit();
    HalMagInit();
    HalAccInit();
    HalBarInit();
    HalGyroInit();

    // Register callbacks with profile
    VOID IRTemp_RegisterAppCBs( &sensorTag_IrTempCBs );
    VOID Magnetometer_RegisterAppCBs( &sensorTag_MagnetometerCBs );
    VOID Accel_RegisterAppCBs( &sensorTag_AccelCBs );
    VOID Humidity_RegisterAppCBs( &sensorTag_HumidCBs );
    VOID Barometer_RegisterAppCBs( &sensorTag_BarometerCBs );
    VOID Gyro_RegisterAppCBs( &sensorTag_GyroCBs );
#if defined FEATURE_TEST
    VOID Test_RegisterAppCBs( &sensorTag_TestCBs );
#endif
#endif
    MDProfile_RegisterAppCBs((mdProfileCBs_t *)&MDProfileCB);
    VOID CcService_RegisterAppCBs( &sensorTag_ccCBs );
    VOID GAPRole_RegisterAppCBs( &paramUpdateCB );

    P0SEL = 0x00; // Configure Port 0 as GPIO
    P1SEL = 0x00; // Configure Port 1 as GPIO
    P2SEL = 0;    // Configure Port 2 as GPIO

    // Port Direction,  0: Input,  1: Output
    P0DIR = 0xFF;
    P1DIR = 0xFF;
    P2DIR = 0x1F;

    P0 = 0x00; // All pins on port 0 to low
    P1 = 0x00; // All pins on port 1 to low
    P2 = 0;   // All pins on port 2 to low
    // Enable clock divide on halt
    // This reduces active current while radio is active and CC254x MCU
    // is halted
    //HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

    // Setup a delayed profile startup
    osal_set_event( sensorTag_TaskID, ST_START_DEVICE_EVT );
}

/*********************************************************************
 * @fn      SensorTag_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 SensorTag_ProcessEvent( uint8 task_id, uint16 events )
{
    VOID task_id; // OSAL required parameter that isn't used in this function

    if ( events & SYS_EVENT_MSG )
    {
        uint8 *pMsg;

        if ( (pMsg = osal_msg_receive( sensorTag_TaskID )) != NULL )
        {
            sensorTag_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

            // Release the OSAL message
            VOID osal_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    // Handle system reset (long press on side key)
    if ( events & ST_SYS_RESET_EVT )
    {
        if (sysResetRequest)
        {
            HAL_SYSTEM_RESET();
        }
        return ( events ^ ST_SYS_RESET_EVT );
    }

    if ( events & ST_START_DEVICE_EVT )
    {
        // Start the Device
        VOID GAPRole_StartDevice( &sensorTag_PeripheralCBs );

        // Start Bond Manager
        VOID GAPBondMgr_Register( &sensorTag_BondMgrCBs );

        HALLM75ATempInit();
        EggLM75ATempInit();
        P0_5 = 0;
        return ( events ^ ST_START_DEVICE_EVT );
    }
#if 0
    //////////////////////////
    //    IR TEMPERATURE    //
    //////////////////////////
    if ( events & ST_IRTEMPERATURE_READ_EVT )
    {
        if ( irTempEnabled )
        {
            if (HalIRTempStatus() == TMP006_DATA_READY)
            {
                readIrTempData();
                osal_start_timerEx( sensorTag_TaskID, ST_IRTEMPERATURE_READ_EVT, sensorTmpPeriod - TEMP_MEAS_DELAY );
            }
            else if (HalIRTempStatus() == TMP006_OFF)
            {
                HalIRTempTurnOn();
                osal_start_timerEx( sensorTag_TaskID, ST_IRTEMPERATURE_READ_EVT, TEMP_MEAS_DELAY );
            }
        }
        else
        {
            //Turn off Temperatur sensor
            VOID HalIRTempTurnOff();
            VOID resetCharacteristicValue(IRTEMPERATURE_SERV_UUID, SENSOR_DATA, 0, IRTEMPERATURE_DATA_LEN);
            VOID resetCharacteristicValue(IRTEMPERATURE_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
        }

        return (events ^ ST_IRTEMPERATURE_READ_EVT);
    }

    //////////////////////////
    //    Accelerometer     //
    //////////////////////////
    if ( events & ST_ACCELEROMETER_SENSOR_EVT )
    {
        if(accConfig != ST_CFG_SENSOR_DISABLE)
        {
            readAccData();
            osal_start_timerEx( sensorTag_TaskID, ST_ACCELEROMETER_SENSOR_EVT, sensorAccPeriod );
        }
        else
        {
            VOID resetCharacteristicValue( ACCELEROMETER_SERV_UUID, SENSOR_DATA, 0, ACCELEROMETER_DATA_LEN );
            VOID resetCharacteristicValue( ACCELEROMETER_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
        }

        return (events ^ ST_ACCELEROMETER_SENSOR_EVT);
    }
#endif
    //////////////////////////
    //      Humidity        //
    //////////////////////////
#ifdef HUMIDITY_MEASUREMENT_1
    if ( events & ST_HUMIDITY_SENSOR_EVT )
    {
        if ( gapProfileState != GAPROLE_CONNECTED )
        {
            return (events ^ ST_HUMIDITY_SENSOR_EVT);
        }
        if (humiEnabled)
        {
            if (gEggState == EGG_STATE_MEASURE_MPU6050 ||
                    gEggState == EGG_STATE_MEASURE_LM75A)
            {
                osal_start_timerEx( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT, 1000 );//Try again after 1000ms.
                return (events ^ ST_HUMIDITY_SENSOR_EVT);
            }
            gEggState = EGG_STATE_MEASURE_HUMIDITY;
            bool returnValue = HalHumiExecMeasurementStep(humiState);
            /*if (!returnValue)
            {
              gEggState = EGG_STATE_MEASURE_IDLE;
              humiState = 0;
              osal_start_timerEx( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT, sensorHumPeriod );
              return (events ^ ST_HUMIDITY_SENSOR_EVT);
            }*/
            if (humiState == 2)
            {
                readHumData();
                humiState = 0;
                gEggState = EGG_STATE_MEASURE_IDLE;
                osal_start_timerEx( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT, sensorHumPeriod );
            }
            else
            {
                humiState++;
                osal_start_timerEx( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT, HUM_FSM_PERIOD );
            }
        }
        else
        {
            #if 0
            resetCharacteristicValue( HUMIDITY_SERV_UUID, SENSOR_DATA, 0, HUMIDITY_DATA_LEN);
            resetCharacteristicValue( HUMIDITY_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
            #endif
            HalHumiInit();
        }
        return (events ^ ST_HUMIDITY_SENSOR_EVT);
    }
#else
    if ( events & ST_HUMIDITY_SENSOR_EVT )
    {
        if ( gapProfileState != GAPROLE_CONNECTED )
        {
            return (events ^ ST_HUMIDITY_SENSOR_EVT);
        }
        if (humiEnabled)
        {
            if (gEggState == EGG_STATE_MEASURE_MPU6050 ||
                    gEggState == EGG_STATE_MEASURE_LM75A)
            {
                osal_start_timerEx( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT, 1000 );//Try again after 1000ms.
                return (events ^ ST_HUMIDITY_SENSOR_EVT);
            }
            gEggState = EGG_STATE_MEASURE_HUMIDITY;
            if (humiState == 0)
            {
                HalExecHumidityMeasurement();
            }
            if (humiState == 1)
            {
                readHumData();
                humiState = 0;
                gEggState = EGG_STATE_MEASURE_IDLE;
                osal_start_timerEx( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT, sensorHumPeriod );
            }
            else
            {
                humiState++;
                osal_start_timerEx( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT, HUM_FSM_PERIOD );
            }
        }
        else
        {
            #if 0
            resetCharacteristicValue( HUMIDITY_SERV_UUID, SENSOR_DATA, 0, HUMIDITY_DATA_LEN);
            resetCharacteristicValue( HUMIDITY_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
            #endif
            //HalHumiInit();
        }
        return (events ^ ST_HUMIDITY_SENSOR_EVT);
    }
#endif
    //////////////////////////
    //    LM75A             //
    //////////////////////////
    if ( events & ST_LM75A_SENSOR_EVT )
    {
        if ( gapProfileState != GAPROLE_CONNECTED )
        {
            return (events ^ ST_LM75A_SENSOR_EVT);
        }
        if (!lm75Enabled)
        {
            return (events ^ ST_LM75A_SENSOR_EVT);
        }
        if (gEggState == EGG_STATE_MEASURE_HUMIDITY ||
                gEggState == EGG_STATE_MEASURE_MPU6050)
        {
            //Try again after 1500ms.
            osal_start_timerEx( sensorTag_TaskID, ST_LM75A_SENSOR_EVT, 1500 );
            return (events ^ ST_LM75A_SENSOR_EVT);
        }
        gEggState = EGG_STATE_MEASURE_LM75A;
        uint8 lm75abuffer[2];
        HalLM75ATempRead(gLM75ACounter++, lm75abuffer);
        if (gsendbufferI == 0)
        {
            osal_memset(gsendbuffer, 0xFF, sizeof(gsendbuffer));
            gsendbuffer[0] = 0xAA; // 0
            gsendbuffer[1] = 0xBB;
            gsendbuffer[2] = 0xBB; // 2
            gsendbufferI += 3;
        }
        gsendbuffer[gsendbufferI++] = lm75abuffer[0];
        gsendbuffer[gsendbufferI++] = lm75abuffer[1];
        if (gLM75ACounter >= 8)
        {
            //gsendbuffer[gsendbufferI++] = 0x0D;
            //gsendbuffer[gsendbufferI++] = 0x0A;
            //eggSerialAppSendNoti(gsendbuffer, 11);
            //ST_HAL_DELAY(1000); //Delay 8ms
            //eggSerialAppSendNoti(gsendbuffer+11, 10);
            gLM75ACounter = 0;
            //gsendbufferI = 0;
            //gEggState = EGG_STATE_MEASURE_IDLE;
            osal_start_timerEx( sensorTag_TaskID, ST_LM75A_SENSOR_GPIO_EVT, 200 );
        }
        else
        {
            osal_start_timerEx( sensorTag_TaskID, ST_LM75A_SENSOR_EVT, 101 );
        }
        return (events ^ ST_LM75A_SENSOR_EVT);
    }
    if (events & ST_LM75A_SENSOR_GPIO_EVT)
    {
        if ( gapProfileState != GAPROLE_CONNECTED )
        {
            return (events ^ ST_LM75A_SENSOR_GPIO_EVT);
        }
        if (!lm75Enabled) 
        {
            return (events ^ ST_LM75A_SENSOR_GPIO_EVT);
        }
        if (gEggState == EGG_STATE_MEASURE_HUMIDITY ||
                gEggState == EGG_STATE_MEASURE_MPU6050)
        {
            //Try again after 1500ms.
            osal_start_timerEx( sensorTag_TaskID, ST_LM75A_SENSOR_GPIO_EVT, 1500 );
            return (events ^ ST_LM75A_SENSOR_GPIO_EVT);
        }
        uint8 lm75abuffer[2] = {0};
        EggLM75ATempRead(gLM75ACounter++, lm75abuffer);
#if 0
        if (gsendbufferI == 0)
        {
            osal_memset(gsendbuffer2, 0xFF, sizeof(gsendbuffer2));
            gsendbuffer2[0] = 0xAA; // 0
            gsendbuffer2[1] = 0xBB;
            gsendbuffer2[2] = 0xBC; // 2
            gsendbufferI += 3;
        }
#endif
        gsendbuffer[gsendbufferI++] = lm75abuffer[0];
        gsendbuffer[gsendbufferI++] = lm75abuffer[1];
        if (gLM75ACounter >= 8)
        {
            gsendbuffer[gsendbufferI++] = 0x0D;
            gsendbuffer[gsendbufferI++] = 0x0A;
            MDSerialAppSendNoti(gsendbuffer, 19);
            ST_HAL_DELAY(1000); //Delay 8ms
            MDSerialAppSendNoti(gsendbuffer + 19, 18);
            gLM75ACounter = 0;
            gsendbufferI = 0;
            gEggState = EGG_STATE_MEASURE_IDLE;
            osal_start_timerEx( sensorTag_TaskID, ST_LM75A_SENSOR_EVT, sensorTmpPeriod );
        }
        else
        {
            osal_start_timerEx( sensorTag_TaskID, ST_LM75A_SENSOR_GPIO_EVT, 101 );
        }
        return (events ^ ST_LM75A_SENSOR_GPIO_EVT);
    }
    //////////////////////////
    //    MPU6050           //
    //////////////////////////
    if ( events & ST_MPU6050_SENSOR_EVT )
    {
        if ( gapProfileState != GAPROLE_CONNECTED )
        {
            return (events ^ ST_MPU6050_SENSOR_EVT);
        }
        if (mpu6050Enabled)
        {
            if (gEggState == EGG_STATE_MEASURE_HUMIDITY ||
                    gEggState == EGG_STATE_MEASURE_LM75A)
            {
                osal_start_timerEx( sensorTag_TaskID, ST_MPU6050_SENSOR_EVT, 1000 );
                return (events ^ ST_MPU6050_SENSOR_EVT);
            }
            gEggState = EGG_STATE_MEASURE_MPU6050;
            mpuIntStatus = HalMPU6050getIntStatus();
            fifoCount = HalMPU6050getFIFOCount();
            if ((mpuIntStatus & 0x10) || fifoCount == 1024)
            {
                //P0_5 = !P0_5;
                //if (fifoCount == 1024) {
                // reset so we can continue cleanly
                HalMPU6050resetFIFO();
                osal_start_timerEx( sensorTag_TaskID, ST_MPU6050_SENSOR_EVT, 100 );
                return (events ^ ST_MPU6050_SENSOR_EVT);
            }
            if (mpuIntStatus & 0x02)
            {
                //P0_5 = 1;
                while (fifoCount < packetSize)
                {
                    fifoCount = HalMPU6050getFIFOCount();
                }
                HalMPU6050getFIFOBytes(fifoBuffer, packetSize);
                fifoCount -= packetSize;
                readMPU6050DmpData(fifoBuffer);
            }
            else
            {
              //P0_5 = 0;
            }
            HalMPU6050setDMPEnabled(false);
            HalMPU6050setSleepEnabled(true);
            HalMPU6050resetFIFO();
            gEggState = EGG_STATE_MEASURE_IDLE;
            osal_start_timerEx( sensorTag_TaskID, ST_MPU6050_DMP_temp_EVT, sensorMpu6050Period );
            
        }
        else
        {
            HalMPU6050setDMPEnabled(false);
            HalMPU6050setSleepEnabled(true);
        }
        return (events ^ ST_MPU6050_SENSOR_EVT);
    }
    if (events & ST_MPU6050_DMP_INIT_EVT)
    {
        HalMPU6050dmpInitialize();
        HalMPU6050setDMPEnabled(true);
        osal_start_timerEx( sensorTag_TaskID, ST_MPU6050_DMP_temp_EVT, 1000 );
        return (events ^ ST_MPU6050_DMP_INIT_EVT);
    }
    if (events & ST_MPU6050_DMP_temp_EVT)
    {
        if ( gapProfileState != GAPROLE_CONNECTED )
        {
            return (events ^ ST_MPU6050_SENSOR_EVT);
        }
        if (mpu6050Enabled)
        {
            HalMPU6050setSleepEnabled(false);
            HalMPU6050setDMPEnabled(true);
            osal_start_timerEx( sensorTag_TaskID, ST_MPU6050_SENSOR_EVT, 500 );
        }
        return (events ^ ST_MPU6050_DMP_temp_EVT);
    }
    if (events & ST_TEST_EVT)
    {
        char bu[64];
        strcpy(bu, "not not work");
        MDSerialAppSendNoti((uint8*)bu, strlen(bu));
        osal_start_timerEx( sensorTag_TaskID, ST_TEST_EVT, 4000 );
        return (events ^ ST_TEST_EVT);
    }
    
#if 0
    //////////////////////////
    //      Magnetometer    //
    //////////////////////////
    if ( events & ST_MAGNETOMETER_SENSOR_EVT )
    {
        if(magEnabled)
        {
            if (HalMagStatus() == MAG3110_DATA_READY)
            {
                readMagData();
            }
            else if (HalMagStatus() == MAG3110_OFF)
            {
                HalMagTurnOn();
            }

            osal_start_timerEx( sensorTag_TaskID, ST_MAGNETOMETER_SENSOR_EVT, sensorMagPeriod );
        }
        else
        {
            HalMagTurnOff();
            resetCharacteristicValue( MAGNETOMETER_SERV_UUID, SENSOR_DATA, 0, MAGNETOMETER_DATA_LEN);
            resetCharacteristicValue( MAGNETOMETER_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
        }

        return (events ^ ST_MAGNETOMETER_SENSOR_EVT);
    }

    //////////////////////////
    //        Barometer     //
    //////////////////////////
    if ( events & ST_BAROMETER_SENSOR_EVT )
    {
        if (barEnabled)
        {
            if (barBusy)
            {
                barBusy = FALSE;
                readBarData();
                osal_start_timerEx( sensorTag_TaskID, ST_BAROMETER_SENSOR_EVT, sensorBarPeriod );
            }
            else
            {
                barBusy = TRUE;
                HalBarStartMeasurement();
                osal_start_timerEx( sensorTag_TaskID, ST_BAROMETER_SENSOR_EVT, BAR_FSM_PERIOD );
            }
        }
        else
        {
            resetCharacteristicValue( BAROMETER_SERV_UUID, SENSOR_DATA, 0, BAROMETER_DATA_LEN);
            resetCharacteristicValue( BAROMETER_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
            resetCharacteristicValue( BAROMETER_SERV_UUID, SENSOR_CALB, 0, BAROMETER_CALI_LEN);
        }

        return (events ^ ST_BAROMETER_SENSOR_EVT);
    }

    //////////////////////////
    //      Gyroscope       //
    //////////////////////////
    if ( events & ST_GYROSCOPE_SENSOR_EVT )
    {
        uint8 status;

        status = HalGyroStatus();

        if(gyroEnabled)
        {
            if (status == HAL_GYRO_STOPPED)
            {
                HalGyroSelectAxes(sensorGyroAxes);
                HalGyroTurnOn();
                osal_start_timerEx( sensorTag_TaskID, ST_GYROSCOPE_SENSOR_EVT, GYRO_STARTUP_TIME);
            }
            else
            {
                if(sensorGyroUpdateAxes)
                {
                    HalGyroSelectAxes(sensorGyroAxes);
                    sensorGyroUpdateAxes = FALSE;
                }

                if (status == HAL_GYRO_DATA_READY)
                {
                    readGyroData();
                    osal_start_timerEx( sensorTag_TaskID, ST_GYROSCOPE_SENSOR_EVT, sensorGyrPeriod - GYRO_STARTUP_TIME);
                }
                else
                {
                    // Gyro needs to be activated;
                    HalGyroWakeUp();
                    osal_start_timerEx( sensorTag_TaskID, ST_GYROSCOPE_SENSOR_EVT, GYRO_STARTUP_TIME);
                }
            }
        }
        else
        {
            HalGyroTurnOff();
            resetCharacteristicValue( GYROSCOPE_SERV_UUID, SENSOR_DATA, 0, GYROSCOPE_DATA_LEN);
            resetCharacteristicValue( GYROSCOPE_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof( uint8 ));
        }

        return (events ^ ST_GYROSCOPE_SENSOR_EVT);
    }
#endif
#if defined ( PLUS_BROADCASTER )
    if ( events & ST_ADV_IN_CONNECTION_EVT )
    {
        uint8 turnOnAdv = TRUE;
        // Turn on advertising while in a connection
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );

        return (events ^ ST_ADV_IN_CONNECTION_EVT);
    }
#endif // PLUS_BROADCASTER

    // Discard unknown events
    return 0;
}

/*********************************************************************
 * @fn      sensorTag_test
 *
 * @brief   Run a self-test of the sensor TAG
 *
 * @param   none
 *
 * @return  bitmask of error flags
 */
uint16 sensorTag_test(void)
{
    selfTestResult = HalSensorTest();
    HalLedSet(HAL_LED_2, HAL_LED_MODE_OFF);
#if defined FEATURE_TEST
    // Write the self-test result to the test service
    Test_SetParameter( TEST_DATA_ATTR, TEST_DATA_LEN, &selfTestResult);
#endif
    return selfTestResult;
}

/*********************************************************************
* Private functions
*/


/*********************************************************************
 * @fn      sensorTag_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void sensorTag_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
    switch ( pMsg->event )
    {
    case KEY_CHANGE:
        sensorTag_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
        break;

    default:
        // do nothing
        break;
    }
}

/*********************************************************************
 * @fn      sensorTag_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void sensorTag_HandleKeys( uint8 shift, uint8 keys )
{
    uint8 SK_Keys = 0;
    VOID shift;  // Intentionally unreferenced parameter

    if (keys & HAL_KEY_SW_1)
    {
        // Reset the system if side key is pressed for more than 3 seconds
        sysResetRequest = TRUE;
        osal_start_timerEx( sensorTag_TaskID, ST_SYS_RESET_EVT, ST_SYS_RESET_DELAY );

        if (!testMode ) // Side key
        {
            // If device is not in a connection, pressing the side key should toggle
            //  advertising on and off
            if ( gapProfileState != GAPROLE_CONNECTED )
            {
                uint8 current_adv_enabled_status;
                uint8 new_adv_enabled_status;

                // Find the current GAP advertising status
                GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );

                if( current_adv_enabled_status == FALSE )
                {
                    new_adv_enabled_status = TRUE;
                }
                else
                {
                    new_adv_enabled_status = FALSE;
                }

                // Change the GAP advertisement status to opposite of current status
                GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
            }

            if ( gapProfileState == GAPROLE_CONNECTED )
            {
                uint8 adv_enabled = TRUE;

                // Disconnect
                GAPRole_TerminateConnection();
                // Start advertising
                GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &adv_enabled );
            }
        }
        else
        {
            // Test mode
            if ( keys & HAL_KEY_SW_1 ) // Side key
            {
                SK_Keys |= SK_KEY_SIDE;
            }
        }
    }

    if ( keys & HAL_KEY_SW_2 )   // Carbon S2
    {
        SK_Keys |= SK_KEY_LEFT;
    }

    if ( keys & HAL_KEY_SW_3 )   // Carbon S3
    {
        SK_Keys |= SK_KEY_RIGHT;
    }

    if (!(keys & HAL_KEY_SW_1))
    {
        // Cancel system reset request
        sysResetRequest = FALSE;
    }

    // Set the value of the keys state to the Simple Keys Profile;
    // This will send out a notification of the keys state if enabled
    SK_SetParameter( SK_KEY_ATTR, sizeof ( uint8 ), &SK_Keys );
}


/*********************************************************************
 * @fn      resetSensorSetup
 *
 * @brief   Turn off all sensors that are on
 *
 * @param   none
 *
 * @return  none
 */
static void resetSensorSetup (void)
{
    #if 0
    if (HalIRTempStatus() != TMP006_OFF || irTempEnabled)
    {
        HalIRTempTurnOff();
        irTempEnabled = FALSE;
    }

    if (accConfig != ST_CFG_SENSOR_DISABLE)
    {
        accConfig = ST_CFG_SENSOR_DISABLE;
    }
    #endif
    if (mpu6050Enabled)
    {
        mpu6050Enabled = FALSE;
        HalMPU6050setDMPEnabled(false);
        HalMPU6050setSleepEnabled(true);
    }
    #if 0
    if (HalMagStatus() != MAG3110_OFF || magEnabled)
    {
        HalMagTurnOff();
        magEnabled = FALSE;
    }

    if (gyroEnabled)
    {
        HalGyroTurnOff();
        gyroEnabled = FALSE;
    }

    if (barEnabled)
    {
        HalBarInit();
        barEnabled = FALSE;
    }
    #endif
    if (lm75Enabled)
    {
        lm75Enabled = FALSE;
    }
    if (humiEnabled)
    {
        HalHumiInit();
        humiEnabled = FALSE;
    }

    // Reset internal states
    //sensorGyroAxes = 0;
    //sensorGyroUpdateAxes = FALSE;
    testMode = FALSE;

    // Reset all characteristics values
    //resetCharacteristicValues();
}


/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
    switch ( newState )
    {
    case GAPROLE_STARTED:
    {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

        // Set the serial number from the bd addr.
        uint8 serialNumber[DEVINFO_SERIAL_NUMBER_LEN + 2] = "\0";
        uint8 aNumber;
        uint8 j = 0;
        for (int8 i = B_ADDR_LEN - 1; i >= 0; i--)
        {
            aNumber = ownAddress[i];
            if (aNumber < 16)
            {
                strcat((char *)serialNumber + j * 2, (const char *)"0");
                _itoa(aNumber, serialNumber + j * 2 + 1, 16);
            }
            else
            {
                _itoa(aNumber, serialNumber + j * 2, 16);
            }

            /*if (osal_memcmp(&aNumber, &Zero, 1) == TRUE)
            {
                strcat((char*)serialNumber+j*2+1, (const char*)"0");
            }*/
            j++;
        }
        DevInfo_SetParameter(DEVINFO_SERIAL_NUMBER, DEVINFO_SERIAL_NUMBER_LEN, serialNumber);
    }
    break;

    case GAPROLE_ADVERTISING:
        HalLedSet(HAL_LED_1, HAL_LED_MODE_ON );
        break;

    case GAPROLE_CONNECTED:
        HalLedSet(HAL_LED_1, HAL_LED_MODE_OFF );
        gEggState = EGG_STATE_MEASURE_IDLE;
        ccUpdate();
        //osal_start_timerEx( sensorTag_TaskID, ST_TEST_EVT, 4000 );
        break;

    case GAPROLE_WAITING:
        // Link terminated intentionally: reset all sensors
        resetSensorSetup();
        break;

    default:
        break;
    }

    gapProfileState = newState;
}

#if 0
/*********************************************************************
 * @fn      readAccData
 *
 * @brief   Read accelerometer data
 *
 * @param   none
 *
 * @return  none
 */
static void readAccData(void)
{
    uint8 aData[ACCELEROMETER_DATA_LEN];

    if (HalAccRead(aData))
    {
        Accel_SetParameter( SENSOR_DATA, ACCELEROMETER_DATA_LEN, aData);
    }
}

/*********************************************************************
 * @fn      readMagData
 *
 * @brief   Read magnetometer data
 *
 * @param   none
 *
 * @return  none
 */
static void readMagData( void )
{
    uint8 mData[MAGNETOMETER_DATA_LEN];

    if (HalMagRead(mData))
    {
        Magnetometer_SetParameter(SENSOR_DATA, MAGNETOMETER_DATA_LEN, mData);
    }
}
#endif

/*********************************************************************
 * @fn      readHumData
 *
 * @brief   Read humidity data
 *
 * @param   none
 *
 * @return  none
 */
#ifdef HUMIDITY_MEASUREMENT_1
static void readHumData(void)
{
    uint8 hData[HUMIDITY_DATA_LEN];
    uint8 buffers[7];
    if (HalHumiReadMeasurement(hData))
    {
        //Humidity_SetParameter( SENSOR_DATA, HUMIDITY_DATA_LEN, hData);

        buffers[0] = 0xAA;
        buffers[1] = 0xBB;
        buffers[2] = 0xCC;
        buffers[3] = hData[2];
        buffers[4] = hData[3];
        buffers[5] = 0x0D;
        buffers[6] = 0x0A;
        //eggSerialAppSendNoti(buffers, 7);
        MDSerialAppSendNoti(buffers, 7);
    }
    else
    {
        buffers[0] = 0xAA;
        buffers[1] = 0xBB;
        buffers[2] = 0xCC;
        buffers[3] = 0x76;
        buffers[4] = 0x76;
        buffers[5] = 0x0D;
        buffers[6] = 0x0A;
        //eggSerialAppSendNoti(buffers, 7);
        MDSerialAppSendNoti(buffers, 7);
    }
}
#else
static void readHumData(void)
{
    uint8 hData[HUMIDITY_DATA_LEN];
    uint8 buffers[7];
    HalHumiRead(hData);
    buffers[0] = 0xAA;
    buffers[1] = 0xBB;
    buffers[2] = 0xCC;
    buffers[3] = hData[0];
    buffers[4] = hData[1];
    buffers[5] = 0x0D;
    buffers[6] = 0x0A;
    MDSerialAppSendNoti(buffers, 7);
}
#endif

#if 0
/*********************************************************************
 * @fn      readBarData
 *
 * @brief   Read barometer data
 *
 * @param   none
 *
 * @return  none
 */
static void readBarData( void )
{
    uint8 bData[BAROMETER_DATA_LEN];

    if (HalBarReadMeasurement(bData))
    {
        Barometer_SetParameter( SENSOR_DATA, BAROMETER_DATA_LEN, bData);
    }
}

/*********************************************************************
 * @fn      readBarCalibration
 *
 * @brief   Read barometer calibration
 *
 * @param   none
 *
 * @return  none
 */
static void readBarCalibration( void )
{
    uint8 *cData = osal_mem_alloc(BAROMETER_CALI_LEN);

    if (cData != NULL )
    {
        HalBarReadCalibration(cData);
        Barometer_SetParameter( SENSOR_CALB, BAROMETER_CALI_LEN, cData);
        osal_mem_free(cData);
    }
}

/*********************************************************************
 * @fn      readIrTempData
 *
 * @brief   Read IR temperature data
 *
 * @param   none
 *
 * @return  none
 */
static void readIrTempData( void )
{
    uint8 tData[IRTEMPERATURE_DATA_LEN];

    if (HalIRTempRead(tData))
    {
        IRTemp_SetParameter( SENSOR_DATA, IRTEMPERATURE_DATA_LEN, tData);
    }
}

/*********************************************************************
 * @fn      readGyroData
 *
 * @brief   Read gyroscope data
 *
 * @param   none
 *
 * @return  none
 */
static void readGyroData( void )
{
    uint8 gData[GYROSCOPE_DATA_LEN];

    if (HalGyroRead(gData))
    {
        Gyro_SetParameter( SENSOR_DATA, GYROSCOPE_DATA_LEN, gData);
    }
}

/*********************************************************************
 * @fn      barometerChangeCB
 *
 * @brief   Callback from Barometer Service indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void barometerChangeCB( uint8 paramID )
{
    uint8 newValue;

    switch( paramID )
    {
    case SENSOR_CONF:
        Barometer_GetParameter( SENSOR_CONF, &newValue );

        switch ( newValue)
        {
        case ST_CFG_SENSOR_DISABLE:
            if (barEnabled)
            {
                barEnabled = FALSE;
                osal_set_event( sensorTag_TaskID, ST_BAROMETER_SENSOR_EVT);
            }
            break;

        case ST_CFG_SENSOR_ENABLE:
            if(!barEnabled)
            {
                barEnabled = TRUE;
                osal_set_event( sensorTag_TaskID, ST_BAROMETER_SENSOR_EVT);
            }
            break;

        case ST_CFG_CALIBRATE:
            readBarCalibration();
            break;

        default:
            break;
        }
        break;

    case SENSOR_PERI:
        Barometer_GetParameter( SENSOR_PERI, &newValue );
        sensorBarPeriod = newValue * SENSOR_PERIOD_RESOLUTION;
        break;

    default:
        // should not get here!
        break;
    }
}

/*********************************************************************
 * @fn      irTempChangeCB
 *
 * @brief   Callback from IR Temperature Service indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void irTempChangeCB( uint8 paramID )
{
    uint8 newValue;

    switch (paramID)
    {
    case SENSOR_CONF:
        IRTemp_GetParameter( SENSOR_CONF, &newValue );

        if ( newValue == ST_CFG_SENSOR_DISABLE)
        {
            // Put sensor to sleep
            if (irTempEnabled)
            {
                irTempEnabled = FALSE;
                osal_set_event( sensorTag_TaskID, ST_IRTEMPERATURE_READ_EVT);
            }
        }
        else if (newValue == ST_CFG_SENSOR_ENABLE)
        {
            if (!irTempEnabled)
            {
                irTempEnabled = TRUE;
                osal_set_event( sensorTag_TaskID, ST_IRTEMPERATURE_READ_EVT);
            }
        }
        break;

    case SENSOR_PERI:
        IRTemp_GetParameter( SENSOR_PERI, &newValue );
        sensorTmpPeriod = newValue * SENSOR_PERIOD_RESOLUTION;
        break;

    default:
        // Should not get here
        break;
    }
}

/*********************************************************************
 * @fn      accelChangeCB
 *
 * @brief   Callback from Acceleromter Service indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void accelChangeCB( uint8 paramID )
{
    uint8 newValue;

    switch (paramID)
    {
    case SENSOR_CONF:
        Accel_GetParameter( SENSOR_CONF, &newValue );
        if ( newValue == ST_CFG_SENSOR_DISABLE)
        {
            // Put sensor to sleep
            if (accConfig != ST_CFG_SENSOR_DISABLE)
            {
                accConfig = ST_CFG_SENSOR_DISABLE;
                osal_set_event( sensorTag_TaskID, ST_ACCELEROMETER_SENSOR_EVT);
            }
        }
        else
        {
            if (accConfig == ST_CFG_SENSOR_DISABLE)
            {
                // Start scheduling only on change disabled -> enabled
                osal_set_event( sensorTag_TaskID, ST_ACCELEROMETER_SENSOR_EVT);
            }
            // Scheduled already, so just change range
            accConfig = newValue;
            HalAccSetRange(accConfig);
        }
        break;

    case SENSOR_PERI:
        Accel_GetParameter( SENSOR_PERI, &newValue );
        sensorAccPeriod = newValue * SENSOR_PERIOD_RESOLUTION;
        break;

    default:
        // Should not get here
        break;
    }
}

/*********************************************************************
 * @fn      magnetometerChangeCB
 *
 * @brief   Callback from Magnetometer Service indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void magnetometerChangeCB( uint8 paramID )
{
    uint8 newValue;

    switch (paramID)
    {
    case SENSOR_CONF:
        Magnetometer_GetParameter( SENSOR_CONF, &newValue );

        if ( newValue == ST_CFG_SENSOR_DISABLE )
        {
            if(magEnabled)
            {
                magEnabled = FALSE;
                osal_set_event( sensorTag_TaskID, ST_MAGNETOMETER_SENSOR_EVT);
            }
        }
        else if ( newValue == ST_CFG_SENSOR_ENABLE )
        {
            if(!magEnabled)
            {
                magEnabled = TRUE;
                osal_set_event( sensorTag_TaskID, ST_MAGNETOMETER_SENSOR_EVT);
            }
        }
        break;

    case SENSOR_PERI:
        Magnetometer_GetParameter( SENSOR_PERI, &newValue );
        sensorMagPeriod = newValue * SENSOR_PERIOD_RESOLUTION;
        break;

    default:
        // Should not get here
        break;
    }
}

/*********************************************************************
 * @fn      humidityChangeCB
 *
 * @brief   Callback from Humidity Service indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void humidityChangeCB( uint8 paramID )
{
    uint8 newValue;

    switch ( paramID)
    {
    case  SENSOR_CONF:
        Humidity_GetParameter( SENSOR_CONF, &newValue );

        if ( newValue == ST_CFG_SENSOR_DISABLE)
        {
            if (humiEnabled)
            {
                humiEnabled = FALSE;
                osal_set_event( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT);
            }
        }

        if ( newValue == ST_CFG_SENSOR_ENABLE )
        {
            if (!humiEnabled)
            {
                humiEnabled = TRUE;
                humiState = 0;
                osal_set_event( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT);
            }
        }
        break;

    case SENSOR_PERI:
        Humidity_GetParameter( SENSOR_PERI, &newValue );
        sensorHumPeriod = newValue * SENSOR_PERIOD_RESOLUTION;
        break;

    default:
        // Should not get here
        break;
    }
}

/*********************************************************************
 * @fn      gyroChangeCB
 *
 * @brief   Callback from GyroProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void gyroChangeCB( uint8 paramID )
{
    uint8 newValue;

    switch (paramID)
    {
    case SENSOR_CONF:
        Gyro_GetParameter( SENSOR_CONF, &newValue );

        if (newValue == 0)
        {
            // All three axes off, put sensor to sleep
            if (gyroEnabled)
            {
                gyroEnabled = FALSE;
                osal_set_event( sensorTag_TaskID, ST_GYROSCOPE_SENSOR_EVT);
            }
        }
        else
        {
            // Bitmap tells which axis to enable (bit 0: X, but 1: Y, but 2: Z)
            gyroEnabled = TRUE;
            sensorGyroAxes = newValue & 0x07;
            sensorGyroUpdateAxes = TRUE;
            osal_set_event( sensorTag_TaskID,  ST_GYROSCOPE_SENSOR_EVT);
        }
        break;

    case SENSOR_PERI:
        Gyro_GetParameter( SENSOR_PERI, &newValue );
        sensorGyrPeriod = newValue * SENSOR_PERIOD_RESOLUTION;
        break;

    default:
        // Should not get here
        break;
    }
}

#if defined FEATURE_TEST
/*********************************************************************
 * @fn      testChangeCB
 *
 * @brief   Callback from Test indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void testChangeCB( uint8 paramID )
{
    if( paramID == TEST_CONF_ATTR )
    {
        uint8 newValue;

        Test_GetParameter( TEST_CONF_ATTR, &newValue );

        if (newValue & TEST_MODE_ENABLE)
        {
            testMode = TRUE;
        }
        else
        {
            testMode = FALSE;
        }

        if (testMode)
        {
            // Test mode: possible to operate LEDs. Key hits will cause notifications,
            // side key does not influence connection state
            if (newValue & 0x01)
            {
                HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);
            }
            else
            {
                HalLedSet(HAL_LED_1, HAL_LED_MODE_OFF);
            }

            if (newValue & 0x02)
            {
                HalLedSet(HAL_LED_2, HAL_LED_MODE_ON);
            }
            else
            {
                HalLedSet(HAL_LED_2, HAL_LED_MODE_OFF);
            }
        }
        else
        {
            // Normal mode; make sure LEDs are reset and attribute cleared
            HalLedSet(HAL_LED_1, HAL_LED_MODE_OFF);
            HalLedSet(HAL_LED_2, HAL_LED_MODE_OFF);
            newValue = 0x00;
            Test_SetParameter( TEST_CONF_ATTR, 1, &newValue );
        }
    }
}
#endif
#endif

/**********************************************************************
 * @fn      ccUpdate
 *
 * @brief   Update the Connection Control service with the current connection
 *          control settings
 *
 */
static void ccUpdate( void )
{
    uint8 buf[CCSERVICE_CHAR1_LEN];
    uint16 connInterval;
    uint16 connSlaveLatency;
    uint16 connTimeout;

    // Get the connection control data
    GAPRole_GetParameter(GAPROLE_CONN_INTERVAL, &connInterval);
    GAPRole_GetParameter(GAPROLE_SLAVE_LATENCY, &connSlaveLatency);
    GAPRole_GetParameter(GAPROLE_CONN_TIMEOUT, &connTimeout);

    buf[0] = LO_UINT16(connInterval);
    buf[1] = HI_UINT16(connInterval);
    buf[2] = LO_UINT16(connSlaveLatency);
    buf[3] = HI_UINT16(connSlaveLatency);
    buf[4] = LO_UINT16(connTimeout);
    buf[5] = HI_UINT16(connTimeout);

    CcService_SetParameter(CCSERVICE_CHAR1, sizeof(buf), buf);
}


/*********************************************************************
 * @fn      ccChangeCB
 *
 * @brief   Callback from Connection Control indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void ccChangeCB( uint8 paramID )
{
    // CCSERVICE_CHAR1: read & notify only

    // CCSERVICE_CHAR: requested connection parameters
    if( paramID == CCSERVICE_CHAR2 )
    {
        //P0_5 = 1;
        uint8 buf[CCSERVICE_CHAR2_LEN];
        uint16 minConnInterval;
        uint16 maxConnInterval;
        uint16 slaveLatency;
        uint16 timeoutMultiplier;

        CcService_GetParameter( CCSERVICE_CHAR2, buf );

        minConnInterval = BUILD_UINT16(buf[0], buf[1]);
        maxConnInterval = BUILD_UINT16(buf[2], buf[3]);
        slaveLatency = BUILD_UINT16(buf[4], buf[5]);
        timeoutMultiplier = BUILD_UINT16(buf[6], buf[7]);

        // Update connection parameters
        GAPRole_SendUpdateParam( minConnInterval, maxConnInterval, slaveLatency,
                                 timeoutMultiplier, GAPROLE_NO_ACTION);
    }

    // CCSERVICE_CHAR3: Disconnect request
    if( paramID == CCSERVICE_CHAR3 )
    {
        // Any change in the value will terminate the connection
        GAPRole_TerminateConnection();
    }
}


/*********************************************************************
 * @fn      gapRolesParamUpdateCB
 *
 * @brief   Called when connection parameters are updates
 *
 * @param   connInterval - new connection interval
 *
 * @param   connSlaveLatency - new slave latency
 *
 * @param   connTimeout - new connection timeout
 *
 * @return  none
*/
static void gapRolesParamUpdateCB( uint16 connInterval, uint16 connSlaveLatency,
                                   uint16 connTimeout )
{
    uint8 buf[CCSERVICE_CHAR1_LEN];

    buf[0] = LO_UINT16(connInterval);
    buf[1] = HI_UINT16(connInterval);
    buf[2] = LO_UINT16(connSlaveLatency);
    buf[3] = HI_UINT16(connSlaveLatency);
    buf[4] = LO_UINT16(connTimeout);
    buf[5] = HI_UINT16(connTimeout);
    CcService_SetParameter(CCSERVICE_CHAR1, sizeof(buf), buf);
}

static void md_ProfileChangeCB( uint8 paramID )
{
    //uint8 newValueBuf[MD_TRANS_LEN] = {0};
    switch( paramID )
    {
#if 0
    case MDPROFILE_ERASE:
        //osal_start_timerEx( keyfobapp_TaskID, KFD_RESET_NOW, 5000 );
        gWriteDone = MDPROFILE_ERASE;
        osal_start_timerEx( keyfobapp_TaskID, KFD_TERMINATE_EVT, 3000 );
        break;
    case MDPROFILE_UPDATE_AVR:
        gWriteDone = MDPROFILE_UPDATE_AVR;
        osal_start_timerEx( keyfobapp_TaskID, KFD_TERMINATE_EVT, 300 );
        break;
    case MDPROFILE_RESET_MANUAL:
        gWriteDone = MDPROFILE_RESET_MANUAL;
        osal_start_timerEx( keyfobapp_TaskID, KFD_TERMINATE_EVT, 3000 );
        break;
    case MDPROFILE_PARTID:
        //        MDProfile_GetParameter
        break;
#endif
    case MDPROFILE_COMMAND:
        //MDProfile_GetParameter( MDPROFILE_COMMAND, newValueBuf );
        //sbpSerialAppWrite(newValueBuf, strlen(newValueBuf));
        //osal_start_timerEx( sensorTag_TaskID, EGG_RESOLVE_COMMAND, 100 );
        resolve_command();
        break;
    default:
        // should not reach here!
        break;
    }
}

static void resolve_command(void)
{
    BLUETOOTH_COMMUNICATE_COMMAND command_id;
    uint8 data[MD_TRANS_LEN] = {0};
    MDProfile_GetParameter( MDPROFILE_COMMAND, data );
    if(data[0] != HEADER_MAGIC)
    {
        return;
    }
    command_id = (BLUETOOTH_COMMUNICATE_COMMAND)data[1];
    uint8 startORstop = data[2];
    uint8 interval = 70; // second
    switch (command_id)
    {
    // AB010114 -- start, interval is 20s; AB0100 -- stop
    case REQUEST_TEMPERATURE_CMD_ID:
        #if 0
        strcpy(data, "not work");
        MDSerialAppSendNoti(data, strlen(data));
        break;
        #endif
        if (startORstop)
        {
            if (!lm75Enabled)
            {
                if (data[3] != 0) // Do not use default period.
                {
                    sensorTmpPeriod = data[3];
                    if (sensorTmpPeriod < 5)
                    {
                        sensorTmpPeriod = 5000;
                    }
                    else if (sensorTmpPeriod > 70)
                    {
                        sensorTmpPeriod = 70000;
                    }
                    else
                    {
                        sensorTmpPeriod = sensorTmpPeriod * 1000;
                    }
                }
                lm75Enabled = TRUE;
                gsendbufferI = 0;
                gLM75ACounter = 0;
                osal_start_timerEx( sensorTag_TaskID, ST_LM75A_SENSOR_EVT, 100 );
            }
        }
        else
        {
            if (lm75Enabled)
            {
                lm75Enabled = FALSE;
                if (gEggState == EGG_STATE_MEASURE_LM75A)
                {
                    gEggState = EGG_STATE_MEASURE_IDLE;
                }
                //osal_start_timerEx( sensorTag_TaskID, ST_LM75A_SENSOR_EVT, 100 );
            }
        }
        break;

    // AB020146 -- start, interval is 70s; AB0200 -- stop
    case REQUEST_HUMIDITY_CMD_ID:
        if (startORstop)
        {
            if (!humiEnabled)
            {
                if (data[3] != 0) // Do not use default period.
                {
                    sensorHumPeriod = data[3];
                    if (sensorHumPeriod < 5)
                    {
                        sensorHumPeriod = 5000;
                    }
                    else if (sensorHumPeriod > 70)
                    {
                        sensorHumPeriod = 70000;
                    }
                    else
                    {
                        sensorHumPeriod = sensorHumPeriod * 1000;
                    }
                }
                humiEnabled = TRUE;
                humiState = 0;
                //osal_set_event( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT);
                osal_start_timerEx( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT, 101 );
            }
        }
        else
        {
            if (humiEnabled)
            {
                humiEnabled = FALSE;
                if (gEggState == EGG_STATE_MEASURE_HUMIDITY)
                {
                    gEggState = EGG_STATE_MEASURE_IDLE;
                }
                //osal_set_event( sensorTag_TaskID, ST_HUMIDITY_SENSOR_EVT);
            }
        }
        break;

    // AB030102 -- start, interval is 2s; AB0300 -- stop
    case REQUEST_MPU6050_CMD_ID:
        if (startORstop)
        {
            //P0_5 = !P0_5;
            if (!mpu6050Enabled)
            {
                if (data[3] != 0) // Do not use default period.
                {
                    sensorMpu6050Period = data[3];
                    if (sensorMpu6050Period < 2)
                    {
                        sensorMpu6050Period = 2000;
                    }
                    else if (sensorMpu6050Period > 10)
                    {
                        sensorMpu6050Period = 10000;
                    }
                    else
                    {
                        sensorMpu6050Period = sensorMpu6050Period * 1000;
                    }
                }
                mpu6050Enabled = TRUE;
                //HalMPU6050initialize();
                osal_start_timerEx( sensorTag_TaskID, ST_MPU6050_DMP_INIT_EVT, 102 );
            }
        }
        else
        {
            if (mpu6050Enabled)
            {
                mpu6050Enabled = FALSE;
                if (gEggState == EGG_STATE_MEASURE_MPU6050)
                {
                    gEggState = EGG_STATE_MEASURE_IDLE;
                }
                osal_set_event( sensorTag_TaskID, ST_MPU6050_SENSOR_EVT);
            }
        }
        break;
    }
}

static void readMPU6050DmpData( uint8 *packet )
{
    int16 ax, ay, az;
    ax = 0;
    ay = 0;
    az = 0;
    uint8 buffers[MPU6050_DATA_LEN];
    int16_t qI[4];
    HalMPU6050getAcceleration(&ax, &ay, &az);
    HalMPU6050dmpGetQuaternion(qI, packet);
    //    float w = (float)qI[0] / 16384.0f;

    buffers[1] = ax >> 8;
    buffers[0] = ax & 0xFF;
    buffers[3] = ay >> 8;
    buffers[2] = ay & 0xFF;
    buffers[5] = az >> 8;
    buffers[4] = az & 0xFF;
#if 1
    buffers[7] = qI[0] >> 8;
    buffers[6] = qI[0] & 0xFF;
    buffers[9] = qI[1] >> 8;
    buffers[8] = qI[1] & 0xFF;
    buffers[11] = qI[2] >> 8;
    buffers[10] = qI[2] & 0xFF;
    buffers[13] = qI[3] >> 8;
    buffers[12] = qI[3] & 0xFF;
#else
    buffers[6] = qI[0] >> 8;
    buffers[7] = qI[0] & 0xFF;
    buffers[8] = qI[1] >> 8;
    buffers[9] = qI[1] & 0xFF;
    buffers[10] = qI[2] >> 8;
    buffers[11] = qI[2] & 0xFF;
    buffers[12] = qI[3] >> 8;
    buffers[13] = qI[3] & 0xFF;
#endif
    uint8 sendbuffer[MPU6050_DATA_LEN + 5];
    sendbuffer[0] = 0xAA;
    sendbuffer[1] = 0xBB;
    sendbuffer[2] = 0xAA;
    VOID osal_memcpy( sendbuffer + 3, buffers, MPU6050_DATA_LEN );
    sendbuffer[17] = 0x0D;
    sendbuffer[18] = 0x0A;
    //Mpu6050_SetParameter(SENSOR_DATA, MPU6050_DATA_LEN, buffers);
    MDSerialAppSendNoti(sendbuffer, MPU6050_DATA_LEN + 5);
}

#if 0
/*********************************************************************
 * @fn      resetCharacteristicValue
 *
 * @brief   Initialize a characteristic value to zero
 *
 * @param   servID - service ID (UUID)
 *
 * @param   paramID - parameter ID of the value is to be cleared
 *
 * @param   vakue - value to initialise with
 *
 * @param   paramLen - length of the parameter
 *
 * @return  none
 */
static void resetCharacteristicValue(uint16 servUuid, uint8 paramID,
                                     uint8 value, uint8 paramLen)
{
    uint8 *pData = osal_mem_alloc(paramLen);

    if (pData == NULL)
    {
        return;
    }

    osal_memset(pData, value, paramLen);

    switch(servUuid)
    {
    case IRTEMPERATURE_SERV_UUID:
        IRTemp_SetParameter( paramID, paramLen, pData);
        break;

    case ACCELEROMETER_SERV_UUID:
        Accel_SetParameter( paramID, paramLen, pData);
        break;

    case MAGNETOMETER_SERV_UUID:
        Magnetometer_SetParameter( paramID, paramLen, pData);
        break;

    case HUMIDITY_SERV_UUID:
        Humidity_SetParameter( paramID, paramLen, pData);
        break;

    case BAROMETER_SERV_UUID:
        Barometer_SetParameter( paramID, paramLen, pData);
        break;

    case GYROSCOPE_SERV_UUID:
        Gyro_SetParameter( paramID, paramLen, pData);
        break;

    default:
        // Should not get here
        break;
    }

    osal_mem_free(pData);
}

/*********************************************************************
 * @fn      resetCharacteristicValues
 *
 * @brief   Initialize all the characteristic values
 *
 * @return  none
 */
static void resetCharacteristicValues( void )
{
    resetCharacteristicValue( IRTEMPERATURE_SERV_UUID, SENSOR_DATA, 0, IRTEMPERATURE_DATA_LEN);
    resetCharacteristicValue( IRTEMPERATURE_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
    resetCharacteristicValue( IRTEMPERATURE_SERV_UUID, SENSOR_PERI, TEMP_DEFAULT_PERIOD / SENSOR_PERIOD_RESOLUTION, sizeof ( uint8 ));

    resetCharacteristicValue( ACCELEROMETER_SERV_UUID, SENSOR_DATA, 0, ACCELEROMETER_DATA_LEN );
    resetCharacteristicValue( ACCELEROMETER_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
    resetCharacteristicValue( ACCELEROMETER_SERV_UUID, SENSOR_PERI, ACC_DEFAULT_PERIOD / SENSOR_PERIOD_RESOLUTION, sizeof ( uint8 ));

    resetCharacteristicValue( HUMIDITY_SERV_UUID, SENSOR_DATA, 0, HUMIDITY_DATA_LEN);
    resetCharacteristicValue( HUMIDITY_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
    resetCharacteristicValue( HUMIDITY_SERV_UUID, SENSOR_PERI, HUM_DEFAULT_PERIOD / SENSOR_PERIOD_RESOLUTION, sizeof ( uint8 ));

    resetCharacteristicValue( MAGNETOMETER_SERV_UUID, SENSOR_DATA, 0, MAGNETOMETER_DATA_LEN);
    resetCharacteristicValue( MAGNETOMETER_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
    resetCharacteristicValue( MAGNETOMETER_SERV_UUID, SENSOR_PERI, MAG_DEFAULT_PERIOD / SENSOR_PERIOD_RESOLUTION, sizeof ( uint8 ));

    resetCharacteristicValue( BAROMETER_SERV_UUID, SENSOR_DATA, 0, BAROMETER_DATA_LEN);
    resetCharacteristicValue( BAROMETER_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof ( uint8 ));
    resetCharacteristicValue( BAROMETER_SERV_UUID, SENSOR_PERI, BAR_DEFAULT_PERIOD / SENSOR_PERIOD_RESOLUTION, sizeof ( uint8 ));

    resetCharacteristicValue( GYROSCOPE_SERV_UUID, SENSOR_DATA, 0, GYROSCOPE_DATA_LEN);
    resetCharacteristicValue( GYROSCOPE_SERV_UUID, SENSOR_CONF, ST_CFG_SENSOR_DISABLE, sizeof( uint8 ));
    resetCharacteristicValue( GYROSCOPE_SERV_UUID, SENSOR_PERI, GYRO_DEFAULT_PERIOD / SENSOR_PERIOD_RESOLUTION, sizeof ( uint8 ));
}
#endif
/*********************************************************************
*********************************************************************/

