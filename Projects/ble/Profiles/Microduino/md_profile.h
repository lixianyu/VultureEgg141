/**************************************************************************************************
  Filename:       md_profile.h
  Revised:        $Date: 2015-09-10 15:41:26 +0800 (Thu, 19 Sep 2015) $
  Revision:       $Revision: 0 $

**************************************************************************************************/
#ifndef MD_PROFILE_H
#define MD_PROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "hal_types.h"

/*********************************************************************
 * CONSTANTS
 */
#if 0
#define MD_SERVICE_UUID      0xF0C0
#define MD_CMD_UUID          0xF0C1
#define MD_TRANS_UUID        0xF0C2
#else
#define MD_SERVICE_UUID      0xFFF0
#define MD_CMD_UUID          0xF0C1
#define MD_TRANS_UUID        0xFFF6
#endif

// OAD Characteristic Indices
#define MD_CHAR_IMG_IDENTIFY 0
#define MD_CHAR_IMG_BLOCK    1

#define MD_BLOCK_SIZE         16
#define MD_BLOCK_SIZE_BIG     20


#define MD_TRANS_LEN                    19

// Profile Parameters
#define MDPROFILE_ERASE                 1
#define MDPROFILE_UPDATE_AVR            2
#define MDPROFILE_RESET_MANUAL          3
#define MDPROFILE_PARTID                4
#define MDPROFILE_TRANS                 5
#define MDPROFILE_COMMAND               6
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*mdProfileChange_t)( uint8 paramID );
typedef struct
{
  mdProfileChange_t        pfnMDProfileChange;  // Called when characteristic value changes
} mdProfileCBs_t;

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * @fn      MDProfile_AddService
 *
 * @brief   Initializes the Microduino Service by registering GATT attributes
 *          with the GATT server. Only call this function once.
 *
 * @return  Success or Failure
 */
bStatus_t MDProfile_AddService( void );
bStatus_t MDProfile_GetParameter( uint8 param, void *value );
extern bStatus_t MDProfile_RegisterAppCBs( mdProfileCBs_t *appCallbacks );
extern void MDBeforeReset(void);
extern bool MDSerialAppSendNoti(uint8 *pBuffer,uint16 length);
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OAD_TARGET_H */
