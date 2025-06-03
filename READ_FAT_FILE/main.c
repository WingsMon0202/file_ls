#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hal.h"
#include "fat.h"
#include "app.h"

FILE *fp = NULL;
BootSector bs;


int main() {
    const char *imagePath = "badfloppy2.img";
    fp = hal_Init(imagePath);

    // Đọc Boot Sector
    bs = fat12_INIT(imagePath, fp);

    // Run App
    app_Display_FAT12(bs, fp);

    fclose(fp);
    return 0;
}
