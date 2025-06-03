/*

*/
#ifndef __APP_H__
#define __APP_H__   

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hal.h"
#include "fat.h"

// Cấu trúc để quản lý ngăn xếp thư mục
typedef struct DirectoryStack {
    unsigned int cluster;
    char path[MAX_PATH];
    struct DirectoryStack *next;
} DirectoryStack;

int app_Display_FAT12(BootSector bs, FILE *fp);

void pushStack(DirectoryStack **top, unsigned int cluster, const char *path);

unsigned int popStack(DirectoryStack **top, char *path);







#endif // __APP_H__



