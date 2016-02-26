/**************************************************************************************************
  Filename:       hal_LM75A_nxp.h
  Revised:        $Date: 2014-10-29 14:06:46 +0800 (Wed, 29 Oct 2014) $
  Revision:       $Revision: 1 $

  Description:    Interface to the LM75A temperature sensor driver
**************************************************************************************************/

#ifndef HAL_LM75A_NXP_H
#define HAL_LM75A_NXP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "comdef.h"

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
  LM75A_OFF,               // LM75A Temperature shutdown
  LM75A_NORMAL,			   // LM75A Temperature On normal
} LM75ATemperature_States_t;


/*********************************************************************
 * FUNCTIONS
 */
void HALLM75ATempInit(void);
void HalLM75ATempTurnOn(uint8 id);
void HalLM75ATempTurnOff(uint8 id);
bool HalLM75ATempRead(uint8 id, uint8 *pBuf);
//bool HalLM75ATempTest(void);
LM75ATemperature_States_t HalLM75ATempStatus(uint8 id);
int8 HalLM75ATempReadAll(uint8 *pBuf);

#ifdef __cplusplus
}
#endif

#endif /* HAL_IRTEMP_H */