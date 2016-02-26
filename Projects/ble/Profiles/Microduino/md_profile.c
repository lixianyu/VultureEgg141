/**************************************************************************************************
  Filename:       md_profile.c
  Revised:        $Date: 2015-09-10 15:49:55 +0800 (Thu, 10 Sep 2015) $
  Revision:       $Revision: 0 $

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include "bcomdef.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "hal_aes.h"
#include "hal_crc.h"
#include "hal_flash.h"
#include "hal_dma.h"
#include "hal_types.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "peripheral.h"
//#include "elara_file_common.h"
#include "md_profile.h"
#include "osal_snv.h"
//#define DEBUG_SERIAL
//#include "SerialApp.h"
/*********************************************************************
 * CONSTANTS
 */
static uint8 partID;
static mdProfileCBs_t *mdProfile_AppCBs = NULL;
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
//static uint32 gBinSize;
//static uint32 gBinRealSize;
//static uint32 gi = 0;

// MD Service UUID
static CONST uint8 mdServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MD_SERVICE_UUID), HI_UINT16(MD_SERVICE_UUID),
    //TI_BASE_UUID_128( MD_SERVICE_UUID )
};

static CONST uint8 mdCharUUID[2][ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MD_CMD_UUID), HI_UINT16(MD_CMD_UUID),
    LO_UINT16(MD_TRANS_UUID), HI_UINT16(MD_TRANS_UUID),
};

/*********************************************************************
 * Profile Attributes - variables
 */

// MD Service attribute
static CONST gattAttrType_t mdService = { ATT_BT_UUID_SIZE, mdServUUID };

// Place holders for the GATT Server App to be able to lookup handles.
//static uint8 mdCharVals[3];
static uint8 mdCmdChar[MD_TRANS_LEN] = {0};
static uint8 mdTRANSChar[MD_TRANS_LEN] = {0};

// MD Characteristic Properties
static uint8 mdCharProps = GATT_PROP_NOTIFY;
static uint8 mdCharPropsWrite = GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// MD Client Characteristic Configs
static gattCharCfg_t *mdTransConfig;

// MD Characteristic user descriptions
static CONST uint8 mdCmdDesc[] = "Command ID";
static CONST uint8 mdTransDesc[] = "Trans";
/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t mdAttrTbl[] =
{
    // MD Service
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *) &mdService
    },

    // Command Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &mdCharPropsWrite 
    },

      // Command Value 
      { 
        { ATT_BT_UUID_SIZE, mdCharUUID[0] },
        GATT_PERMIT_WRITE, 
        0, 
        mdCmdChar, 
      },

      // Command User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        (uint8*)mdCmdDesc 
      },

    // MD TRANS Characteristic Declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &mdCharProps
    },

    // MD TRANS Characteristic Value
    {
        { ATT_BT_UUID_SIZE, mdCharUUID[1] },
        GATT_PERMIT_WRITE,
        0,
        mdTRANSChar
    },

    // Characteristic configuration
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *) &mdTransConfig
    },

    // MD TRANS User Description
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *)mdTransDesc
    },
};

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t mdReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                              uint8 *pValue, uint8 *pLen, uint16 offset,
                              uint8 maxLen, uint8 method);

static bStatus_t mdWriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                               uint8 *pValue, uint8 len, uint16 offset,
                               uint8 method);

CONST gattServiceCBs_t mdCBs =
{
    mdReadAttrCB,  // Read callback function pointer.
    mdWriteAttrCB, // Write callback function pointer.
    NULL            // Authorization callback function pointer.
};
#if 0
static void mdImgBlockReq(uint16 connHandle, uint32 wSize);

static void mdImgIdentifyReq(uint16 connHandle, uint32 avSize);

static bStatus_t mdImgIdentifyWrite( uint16 connHandle, uint8 *pValue );

static bStatus_t mdImgBlockWrite( uint16 connHandle, uint8 *pValue );
#endif

/*********************************************************************
 * @fn      MDProfile_AddService
 *
 * @brief   Initializes the Microduino Service by registering GATT attributes
 *          with the GATT server. Only call this function once.
 *
 * @return  The return value of GATTServApp_RegisterForMsg().
 */
bStatus_t MDProfile_AddService(void)
{
#if 0
    // Allocate Client Characteristic Configuration table
    mdImgIdentifyConfig = (gattCharCfg_t *)osal_mem_alloc( sizeof(gattCharCfg_t) *
                          linkDBNumConns);
    if (mdImgIdentifyConfig == NULL)
    {
        return ( bleMemAllocError );
    }

    // Allocate Client Characteristic Configuration table
    mdImgBlockConfig = (gattCharCfg_t *)osal_mem_alloc( sizeof(gattCharCfg_t) *
                       linkDBNumConns);

    if (mdImgBlockConfig == NULL)
    {
        // Free already allocated data
        osal_mem_free( mdImgIdentifyConfig );

        return ( bleMemAllocError );
    }
#endif
    mdTransConfig = (gattCharCfg_t *)osal_mem_alloc( sizeof(gattCharCfg_t) * linkDBNumConns);
    if (mdTransConfig == NULL)
    {
        #if 0
        osal_mem_free( mdImgIdentifyConfig );
        osal_mem_free( mdImgBlockConfig );
        #endif
        return ( bleMemAllocError );
    }
    // Initialize Client Characteristic Configuration attributes
    #if 0
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, mdImgIdentifyConfig );
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, mdImgBlockConfig );
    #endif
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, mdTransConfig );

    return GATTServApp_RegisterService(mdAttrTbl, GATT_NUM_ATTRS(mdAttrTbl),
                                       GATT_MAX_ENCRYPT_KEY_SIZE, &mdCBs);
}

/*********************************************************************
 * @fn      mdReadAttrCB
 *
 * @brief   Read an attribute.
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be read
 * @param   pLen - length of data to be read
 * @param   offset - offset of the first octet to be read
 * @param   maxLen - maximum length of data to be read
 * @param   method - type of read message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t mdReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                              uint8 *pValue, uint8 *pLen, uint16 offset,
                              uint8 maxLen, uint8 method)
{
    bStatus_t status = SUCCESS;

    // TBD: is there any use for supporting reads
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;

    return status;
}

/*********************************************************************
 * @fn      mdWriteAttrCB
 *
 * @brief   Validate and Write attribute data
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
gattAttribute_t *gp_Attr;
uint16 gConnHandle = 0;
static bStatus_t mdWriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                               uint8 *pValue, uint8 len, uint16 offset,
                               uint8 method)
{
    bStatus_t status = SUCCESS;
    
    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
        switch ( uuid )
        {
#if 0
            case MD_BIN_SIZE_UUID:
                //SerialPrintValue("offset = ", offset, 10);
                //SerialPrintValue("len = ", len, 10);
                if ( offset == 0 )
                {
                    if ( len != 5 )
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                    }
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }
                if ( status == SUCCESS )
                {
                    mdImgIdentifyWrite( connHandle, pValue );
                }
                break;
            case MD_BIN_BLOCK_UUID:
                //SerialPrintValue("offset1 = ", offset, 10);
                //SerialPrintValue("len1 = ", len, 10);
                if ( offset == 0 )
                {
                    if ( len > MD_BLOCK_SIZE_BIG)
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                    }
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }
                if ( status == SUCCESS )
                {
                    gp_Attr = pAttr;
                    mdImgBlockWrite( connHandle, pValue );
                }
                break;

            case MD_TRANS_UUID:
                if ( offset == 0 )
                {
                    if ( len > MD_TRANS_LEN)
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                    }
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }
                if ( status == SUCCESS )
                {
                    uint8 *pCurValue = (uint8 *)pAttr->pValue;
		            memset(pCurValue, 0, MD_TRANS_LEN);
                    memcpy(pCurValue, pValue, len );
                    mdProfile_AppCBs->pfnMDProfileChange( MDPROFILE_TRANS );
                }
                break;
#endif
            case MD_CMD_UUID:
                if ( offset == 0 )
                {
                    if ( len != 4)
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                    }
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }
                if ( status == SUCCESS )
                {
                    uint8 *pCurValue = (uint8 *)pAttr->pValue;
		            memset(pCurValue, 0, MD_TRANS_LEN);
                    memcpy(pCurValue, pValue, len );
                    mdProfile_AppCBs->pfnMDProfileChange( MDPROFILE_COMMAND );
                }
                break;

            case GATT_CLIENT_CHAR_CFG_UUID:
                status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                         offset, GATT_CLIENT_CFG_NOTIFY );
                break;
            default:
                // Should never get here!
                status = ATT_ERR_ATTR_NOT_FOUND;
                break;
        }
    }
    else
    {/*
        // 128-bit UUID
        if (osal_memcmp(pAttr->type.uuid, mdCharUUID[MD_CHAR_IMG_IDENTIFY], ATT_UUID_SIZE))
        {
            //Validate the value
            // Make sure it's not a blob oper
            if ( offset == 0 )
            {
                if ( len != 5 )
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }
            if ( status == SUCCESS )
            {
                mdImgIdentifyWrite( connHandle, pValue );
            }
        }
        else if (osal_memcmp(pAttr->type.uuid, mdCharUUID[MD_CHAR_IMG_BLOCK], ATT_UUID_SIZE))
        {
            if ( offset == 0 )
            {
                if ( len != MD_BLOCK_SIZE)
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else
            {
                status = ATT_ERR_ATTR_NOT_LONG;
            }
            if ( status == SUCCESS )
            {
                mdImgBlockWrite( connHandle, pValue );
            }
        }
        else
        {
            status = ATT_ERR_ATTR_NOT_FOUND; // Should never get here!
        }
        */
    }

    return status;
}

#if 0
/*********************************************************************
 * @fn      mdImgIdentifyWrite
 *
 * @brief   Process the Image Identify Write.
 *
 * @param   connHandle - connection message was received on
 * @param   pValue - pointer to data to be written
 *
 * @return  status
 */
static bStatus_t mdImgIdentifyWrite( uint16 connHandle, uint8 *pValue )
{
    gBinSize = BUILD_UINT32(pValue[0], pValue[1], pValue[2], pValue[3]);
    partID = pValue[4];
    //elara_flash_write_init();
    uint32 avSize = elara_flash_if_flash_enough(gBinSize);
    if (avSize != 0)
    {
        if (avSize == 0xFFFFFFFF)
        {
            //uint8 uState = 3;
            //elara_snv_write(BLE_NVID_U_STATE, 1, &uState);
            mdProfile_AppCBs->pfnMDProfileChange( MDPROFILE_ERASE );
        }
        else
        {
            osal_pwrmgr_device( PWRMGR_ALWAYS_ON );
            P1_0 = 0;
            //elara_snv_write(BLE_NVID_PART_ID, 1, &partID);
            //mdProfile_AppCBs->pfnMDProfileChange( MDPROFILE_PARTID );
            gi = 0;
            gBinRealSize = ((gBinSize + HAL_FLASH_WORD_SIZE - 1) / (uint32)HAL_FLASH_WORD_SIZE) * (uint32)HAL_FLASH_WORD_SIZE;//4
            //gBinRealSize += 4;
            //gBinRealSize = gBinSize;
        }
    }
    mdImgIdentifyReq(connHandle, avSize);
    return ( SUCCESS );
}

/*********************************************************************
 * @fn      mdImgBlockWrite
 *
 * @brief   Process the Image Block Write.
 *
 * @param   connHandle - connection message was received on
 * @param   pValue - pointer to data to be written
 *
 * @return  status
 */
static bStatus_t mdImgBlockWrite( uint16 connHandle, uint8 *pValue )
{
    static uint8 leds = 0;
    //if (leds++ % 2 == 0)
    {
        P1_1 = !P1_1; // Blue LED.
    }
    uint16 block_size = MD_BLOCK_SIZE;

    if (gi < gBinRealSize)
    {
        #if 1
        if (gBinRealSize - gi < MD_BLOCK_SIZE)
        {
            block_size = gBinRealSize - gi;
        }
        else
        {
            block_size = MD_BLOCK_SIZE;
        }
        #endif
        elara_flash_write(block_size, pValue);
    }

    gi += block_size;
    if (gi >= gBinRealSize)
    {
        //elara_flash_write_done();
        mdImgBlockReq(connHandle, gi);
        mdProfile_AppCBs->pfnMDProfileChange( MDPROFILE_UPDATE_AVR );
        P1_0 = 1; // Green LED.
    }
    else
    {
        //mdImgBlockReq(connHandle, gi);
    }

    return ( SUCCESS );
}

/*********************************************************************
 * @fn      mdImgBlockReq
 *
 * @brief   Process the Image Identify Request.
 *
 * @param   connHandle - connection message was received on
 * @param   blkNum - size.
 *
 * @return  None
 */
static void mdImgBlockReq(uint16 connHandle, uint32 wSize)
{
    uint16 value = GATTServApp_ReadCharCfg( connHandle, mdImgBlockConfig );

    // If notifications enabled
    if ( value & GATT_CLIENT_CFG_NOTIFY )
    {
        //gattAttribute_t *pAttr = GATTServApp_FindAttr(mdAttrTbl, GATT_NUM_ATTRS(mdAttrTbl),
          //                       mdCharVals + MD_CHAR_IMG_BLOCK);
        //if ( pAttr != NULL )
        {
            attHandleValueNoti_t noti;

            noti.pValue = GATT_bm_alloc(connHandle, ATT_HANDLE_VALUE_NOTI,
                                        8, NULL);
            if ( noti.pValue != NULL )
            {
                uint32 avSize = elara_flash_if_flash_enough(1);

                noti.handle = gp_Attr->handle;
                //noti.handle = connHandle;
                noti.len = 8;
                noti.pValue[0] = (uint8)(wSize & 0xFF);
                noti.pValue[1] = (uint8)(wSize >> 8 & 0xFF);
                noti.pValue[2] = (uint8)(wSize >> 16 & 0xFF);
                noti.pValue[3] = (uint8)(wSize >> 24 & 0xFF);

                noti.pValue[4] = (uint8)(avSize & 0xFF);
                noti.pValue[5] = (uint8)(avSize >> 8 & 0xFF);
                noti.pValue[6] = (uint8)(avSize >> 16 & 0xFF);
                noti.pValue[7] = (uint8)(avSize >> 24 & 0xFF);
                if ( GATT_Notification(connHandle, &noti, FALSE) != SUCCESS )
                {
                    GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
                }
            }
        }
    }
}

/*********************************************************************
 * @fn      mdImgIdentifyReq
 *
 * @brief   Process the Image Identify Request.
 *
 * @param   connHandle - connection message was received on
 * @param   pImgHdr - Pointer to the img_hdr_t data to send.
 *
 * @return  None
 */
static void mdImgIdentifyReq(uint16 connHandle, uint32 avSize)
{
    uint16 value = GATTServApp_ReadCharCfg( connHandle, mdImgIdentifyConfig );

    // If notifications enabled
    if ( value & GATT_CLIENT_CFG_NOTIFY )
    {
        gattAttribute_t *pAttr = GATTServApp_FindAttr(mdAttrTbl, GATT_NUM_ATTRS(mdAttrTbl),
                                 mdCharVals + MD_CHAR_IMG_IDENTIFY);
        if ( pAttr != NULL )
        {
            attHandleValueNoti_t noti;

            noti.pValue = GATT_bm_alloc(connHandle, ATT_HANDLE_VALUE_NOTI,
                                        4, NULL);
            if ( noti.pValue != NULL )
            {
                noti.handle = pAttr->handle;
                noti.len = 4;
                #if 0
                noti.pValue[0] = BREAK_UINT32(avSize, 0);
                noti.pValue[1] = BREAK_UINT32(avSize, 1);
                noti.pValue[2] = BREAK_UINT32(avSize, 2);
                noti.pValue[3] = BREAK_UINT32(avSize, 3);
                #else
                noti.pValue[0] = (uint8)(avSize & 0xFF);
                noti.pValue[1] = (uint8)(avSize >> 8 & 0xFF);
                noti.pValue[2] = (uint8)(avSize >> 16 & 0xFF);
                noti.pValue[3] = (uint8)(avSize >> 24 & 0xFF);
                #endif
                if ( GATT_Notification(connHandle, &noti, FALSE) != SUCCESS )
                {
                    GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
                }
            }
        }
    }
}
#endif

bool MDSerialAppSendNoti(uint8 *pBuffer, uint16 length)
{
    GAPRole_GetParameter(GAPROLE_CONNHANDLE, &gConnHandle);
    if (gConnHandle == INVALID_CONNHANDLE || length == 0)
    {
        return FALSE;
    }
    uint16 value = GATTServApp_ReadCharCfg( gConnHandle, mdTransConfig );

    // If notifications enabled
    if ( value & GATT_CLIENT_CFG_NOTIFY )
    {
        gattAttribute_t *pAttr = GATTServApp_FindAttr(mdAttrTbl, GATT_NUM_ATTRS(mdAttrTbl),
                                 mdTRANSChar);
        if ( pAttr != NULL )
        {
            attHandleValueNoti_t noti;

            noti.pValue = GATT_bm_alloc(gConnHandle, ATT_HANDLE_VALUE_NOTI,
                                        length, NULL);
            if ( noti.pValue != NULL )
            {
                noti.handle = pAttr->handle;
                noti.len = length;
                memcpy(noti.pValue, pBuffer, length);
                if ( GATT_Notification(gConnHandle, &noti, FALSE) != SUCCESS )
                {
                    GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
                }
            }
        }
    }
    return TRUE;
}

#if 0
void MDBeforeReset(void)
{
    uint32 avSize = elara_flash_if_flash_enough(gBinSize);
#if 0 // Can not go into here at all.
    if (avSize == 0)
    {
    }
    else
#endif
    if (avSize == 0xFFFFFFFF)
    {
        uint32 uState = 3;
        #if 1
        elara_snv_write(BLE_NVID_U_STATE, 1, &uState);
        #endif
    }
    else
    {
        elara_flash_lost_connected();
    }
}
#endif

bStatus_t MDProfile_RegisterAppCBs( mdProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    mdProfile_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

bStatus_t MDProfile_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case MDPROFILE_PARTID:
      *((uint32 *)value) = partID;
      break;

    case MDPROFILE_COMMAND:
        memcpy( value, mdTRANSChar, MD_TRANS_LEN );
        break;
        
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}
/*********************************************************************
*********************************************************************/
