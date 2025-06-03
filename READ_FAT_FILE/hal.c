#include "hal.h"
#include "fat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
*********************************************************************************************************
*                                            API
*********************************************************************************************************
*/

/*
* @brief Init - Open file 
* @param[in] imagePath - Path to image file
* @param[out] fp - File pointer
* @return 0 if success, otherwise return 1
*/

FILE *hal_Init(const char *imagePath) {
    FILE *fp = fopen(imagePath, "rb");
    if (fp == NULL) {
        perror("Failed to open image file");
        return NULL;
    }
    return fp;
}

/*
* @brief ReadSector - Read sector from image file
* @param[in] fp - File pointer
* @param[in] sector - Sector number
* @param[out] buffer - Buffer to store data
* @return 0 if success, otherwise return 1
*/

int hal_ReadSector(FILE *fp, unsigned int sector, void *buffer) {
    if (fseek(fp, sector * BYTES_PER_SECTOR, SEEK_SET) != 0) {
        perror("Failed to seek sector");
        return 1;
    }
    if (fread(buffer, BYTES_PER_SECTOR, 1, fp) != 1) {
        perror("Failed to read sector");
        return 1;
    }
    return 0;
}

/*
* @brief seek and read n bytes from file
* @param[in] fp - File pointer
* @param[in] n - Number of bytes to read
* @param[in] offset - Offset to seek
* @param[out] buffer - Buffer to store data
* @return 0 if success, otherwise return 1
*/

int hal_Seek_And_Read(FILE *fp, unsigned int n, unsigned int offset, void *buffer) {
    if (fseek(fp, offset, SEEK_SET) != 0) {
        perror("Failed to seek sector");
        return 1;
    }
    if (fread(buffer, n, 1, fp) != 1) {
        perror("Failed to read sector");
        return 1;
    }
    return 0;
}

