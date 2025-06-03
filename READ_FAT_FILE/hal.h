/*
HAL - Hardware Abstraction Layer for Read FAT12 File System
*/
#ifndef __HAL_H__
#define __HAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "fat.h"

/*
*********************************************************************************************************
*                                            DEFINES & MACROS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            API
*********************************************************************************************************
*/

FILE *hal_Init(const char *imagePath);
int hal_ReadSector(FILE *fp, unsigned int sector, void *buffer);


















#endif // __HAL_H__





