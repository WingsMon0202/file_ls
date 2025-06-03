#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal.h"
#include "fat.h"

extern BootSector bs;

/*
*********************************************************************************************************
*                                            API
********************************************************************************************************* 
*/

BootSector fat12_INIT(const char *imagePath, FILE *fp) {
    BootSector bs;
    if (fp == NULL) {
        perror("Failed to open image file");
        exit(1);
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        perror("Failed to seek boot sector");
        exit(1);
    }

    if (fread(&bs, sizeof(BootSector), 1, fp) != 1) {
        perror("Failed to read boot sector");
        exit(1);
    }

    return bs;
    
}

// Hàm tính sector đầu tiên của Data Region
unsigned int fat_GetFirstDataSector(const BootSector *bs) {
    return bs->reservedSectors + (bs->numberOfFATs * bs->sectorsPerFAT) + 
           ((bs->rootEntryCount * DIR_ENTRY_SIZE) + (bs->bytesPerSector - 1)) / bs->bytesPerSector;
}

/*
* @brief Liệt kê các mục trong thư mục
* @param[in] fp - File pointer
* @param[in] bs - BootSector structure
* @param[in] currentCluster - Cluster của thư mục hiện tại
* @param[out] listing - Danh sách các mục
* @param[in] fatOffset - Offset của bảng FAT
* @param[in] firstDataSector - Sector đầu tiên của Data Region
*/
void fat_listDirectory(FILE *fp, const BootSector *bs, unsigned int cluster, DirectoryListing *listing, unsigned int fatOffset, unsigned int firstDataSector)
{
    DirectoryEntry entry;
    LongFileNameEntry lfnEntries[20];
    int lfnCount = 0;
    listing->count = 0;

    if (cluster == 0) {
        // Root Directory
        fseek(fp, bs->reservedSectors * bs->bytesPerSector + 
            (bs->numberOfFATs * bs->sectorsPerFAT * bs->bytesPerSector), SEEK_SET);
    } else {
        // Subdirectory
        unsigned int sector = fat_GetSectorOfCluster(cluster, bs, firstDataSector);
        fseek(fp, sector * bs->bytesPerSector, SEEK_SET);
    }

    while (fread(&entry, sizeof(DirectoryEntry), 1, fp) == 1) {
        if (entry.filename[0] == 0x00) {
            break; // No more entries
        }
        if ((unsigned char)entry.filename[0] == 0xE5) {
            continue; // Skip deleted entry
        }
        if (entry.attributes == 0x0F) {
            // Long File Name entry
            LongFileNameEntry *lfn = (LongFileNameEntry *)&entry;
            lfnEntries[lfnCount++] = *lfn;
            continue;
        }

        char fileName[MAX_PATH] = {0};
        if (lfnCount > 0) {
            char longFileName[MAX_PATH] = {0};
            getLongFileName(lfnEntries, lfnCount, longFileName);
            strncpy(fileName, longFileName, MAX_PATH);
            lfnCount = 0; // Reset LFN count
        } else {
            // Short File Name
            int i = 0, j = 0;
            // Extract filename
            while (i < 8 && entry.filename[i] != ' ') {
                fileName[j++] = entry.filename[i++];
            }
            // Extract extension
            if (entry.extension[0] != ' ') {
                fileName[j++] = '.';
                for (i = 0; i < 3 && entry.extension[i] != ' '; i++) {
                    fileName[j++] = entry.extension[i];
                }
            }
            fileName[j] = '\0';
        }

        // Loại bỏ khoảng trắng ở cuối tên
        for (int i = strlen(fileName) - 1; i >=0; i--) {
            if (fileName[i] == ' ') fileName[i] = '\0';
            else break;
        }

        // Loại bỏ các ký tự đặc biệt (đặc biệt đối với LFN)
        if (strlen(fileName) == 0) continue;

        // Loại bỏ các thư mục ẩn và hệ thống
        if (entry.attributes & 0x08 || entry.attributes & 0x02 || entry.attributes & 0x04) {
            continue;
        }

        // Kiểm tra xem mục là thư mục hay tệp
        char type[10];
        strcpy(type, (entry.attributes & 0x10) ? "Directory" : "File");

        // Chuyển đổi ngày và giờ
        char createdDate[20];
        char modifiedDate[20];
        convertDateTime(entry.creationDate, entry.creationTime, createdDate, sizeof(createdDate));
        convertDateTime(entry.writeDate, entry.writeTime, modifiedDate, sizeof(modifiedDate));

        // Lưu thông tin vào DirectoryListing
        strncpy(listing->fileNames[listing->count], fileName, MAX_PATH);
        strncpy(listing->fileTypes[listing->count], type, 10);
        strncpy(listing->createdDates[listing->count], createdDate, 20);
        strncpy(listing->modifiedDates[listing->count], modifiedDate, 20);
        listing->fileSizes[listing->count] = entry.fileSize;
        listing->firstClusterLows[listing->count] = entry.firstClusterLow;
        listing->count++;
    }
}

/*
* @brief Doc ten file dai
* @param[in] lfnEntries - Mang cac entry cua LongFileName
* @param[in] lfnCount - So luong entry
* @param[out] longFileName - Ten file dai
*/
void getLongFileName(LongFileNameEntry *lfnEntries, int lfnCount, char *longFileName) {
    int index = 0;
    for (int i = lfnCount - 1; i >= 0; i--) {
        LongFileNameEntry *entry = &lfnEntries[i];
        for (int j = 0; j < 5; j++) {
            if (entry->name1[j] == 0xFFFF || entry->name1[j] == 0x0000) break;
            longFileName[index++] = (char)entry->name1[j];
        }
        for (int j = 0; j < 6; j++) {
            if (entry->name2[j] == 0xFFFF || entry->name2[j] == 0x0000) break;
            longFileName[index++] = (char)entry->name2[j];
        }
        for (int j = 0; j < 2; j++) {
            if (entry->name3[j] == 0xFFFF || entry->name3[j] == 0x0000) break;
            longFileName[index++] = (char)entry->name3[j];
        }
    }
    longFileName[index] = '\0';
}

/*
* @brief Chuyen doi ngay va gio
* @param[in] date - Ngay
* @param[in] time - Gio
* @param[out] buffer - Buffer de luu ket qua
* @param[in] size - Kich thuoc cua buffer
*/
void convertDateTime(unsigned short date, unsigned short time, char *buffer, size_t size) {
    int year = ((date >> 9) & 0x7F) + 1980;
    int month = (date >> 5) & 0x0F;
    int day = date & 0x1F;

    int hours = (time >> 11) & 0x1F;
    int minutes = (time >> 5) & 0x3F;
    int seconds = (time & 0x1F) * 2;

    snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hours, minutes, seconds);
}


/*
* @brief Lay cluster tiep theo
* @param[in] currentCluster - Cluster hien tai
* @param[in] fp - File pointer
* @param[in] fatOffset - Offset cua FAT table 
* @return Cluster tiep theo
*/
unsigned int getNextCluster(unsigned int currentCluster, FILE *fp, unsigned int fatOffset) {
    unsigned int nextCluster = 0;
    fseek(fp, fatOffset + (currentCluster * 3) / 2, SEEK_SET);
    unsigned char fatEntry[2];
    fread(fatEntry, 1, 2, fp);

    if (currentCluster % 2 == 0) {
        nextCluster = fatEntry[0] | ((fatEntry[1] & 0x0F) << 8);
    } else {
        nextCluster = ((fatEntry[0] >> 4) & 0x0F) | (fatEntry[1] << 4);
    }

    return nextCluster;
}

/*
* @brief Lay sector dau cua mot cluster cu the
* @param[in] cluster - Cluster can tinh
*/

unsigned int getFirstDataSector(const BootSector *bs) {
    return bs->reservedSectors + (bs->numberOfFATs * bs->sectorsPerFAT) + 
           ((bs->rootEntryCount * DIR_ENTRY_SIZE) + (bs->bytesPerSector - 1)) / bs->bytesPerSector;
}

/*
* @brief Lay sector cua mot cluster cu the
* @param[in] cluster - Cluster can tinh
* @param[in] bs - BootSector structure
* @param[in] firstDataSector - Sector dau tien cua Data Region
*/
unsigned int fat_GetSectorOfCluster(unsigned int cluster, const BootSector *bs, unsigned int firstDataSector) {
    return firstDataSector + ((cluster - 2) * bs->sectorsPerCluster);
}

/*
* @brief Doc noi dung cua file
* @param[in] fp - File pointer
* @param[in] bs - BootSector structure
* @param[in] firstCluster - Cluster dau tien cua file
* @param[in] fileSize - Kich thuoc file
* @param[in] fatOffset - Offset cua FAT table
* @param[in] firstDataSector - Sector dau tien cua Data Region
*/
void readFileContent(FILE *fp, const BootSector *bs, unsigned int firstCluster, unsigned int fileSize, unsigned int fatOffset, unsigned int firstDataSector) {
    unsigned int cluster = firstCluster;
    unsigned int bytesRead = 0;
    uint8_t buffer[BYTES_PER_SECTOR];
    
    while (bytesRead < fileSize && cluster < END_OF_FILE_CLUSTER) {
        unsigned int sector = fat_GetSectorOfCluster(cluster, bs, firstDataSector);
        fseek(fp, sector * bs->bytesPerSector, SEEK_SET);
        size_t toRead = (fileSize - bytesRead > bs->bytesPerSector) ? bs->bytesPerSector : (fileSize - bytesRead);
        size_t read = fread(buffer, 1, toRead, fp);
        fwrite(buffer, 1, read, stdout);
        bytesRead += read;
        cluster = getNextCluster(cluster, fp, fatOffset);
    }
    printf("\n");
}


