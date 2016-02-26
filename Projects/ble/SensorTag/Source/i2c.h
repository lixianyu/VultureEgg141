/**************************************************************************************************
  Filename:       i2c.h
  Revised:        $Date: 2015-06-10 10:04:58 +0800 (Wed, 10 Jun 2015) $
  Revision:       $Revision: 0 $


**************************************************************************************************/

#ifndef I2C_H
#define I2C_H

#include "hal_types.h"

// Function prototypes
extern int8 EggReadAllLM75ATemp(uint8 *pBuf);
extern void EggLM75ATempInit(void);
extern bool EggLM75ATempRead(uint8 id, uint8 *pBuf);
#endif