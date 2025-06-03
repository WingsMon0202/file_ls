/*
FAT12 Library
*/

#ifndef __FAT_H__
#define __FAT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/*
*********************************************************************************************************
*                                            DEFINES & MACROS
*********************************************************************************************************
*/

#define BYTES_PER_SECTOR 512
#define END_OF_FILE_CLUSTER 0xFF8

#define DIR_ENTRY_SIZE 32
#define MAX_PATH 260


#pragma pack(push, 1)
typedef struct {
    uint8_t jump[3];
    char OEMName[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t numberOfFATs;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t media;
    uint16_t sectorsPerFAT;
    uint16_t sectorsPerTrack;
    uint16_t numberOfHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;
    // Extended Boot Record
    uint8_t driveNumber;
    uint8_t reserved1;
    uint8_t bootSignature;
    uint32_t volumeID;
    char volumeLabel[11];
    char fileSystemType[8];
} BootSector;

typedef struct {
    char filename[8];
    char extension[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creationTimeTenths;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessDate;
    uint16_t firstClusterHigh;
    uint16_t writeTime;
    uint16_t writeDate;
    uint16_t firstClusterLow;
    uint32_t fileSize;
} DirectoryEntry;

typedef struct {
    uint8_t order;
    uint16_t name1[5];
    uint8_t attributes;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t firstClusterLow;
    uint16_t name3[2];
} LongFileNameEntry;
#pragma pack(pop)


// Cấu trúc để lưu trữ thông tin thư mục hiện tại
typedef struct {
    char fileNames[100][MAX_PATH];
    char fileTypes[100][10];
    char createdDates[100][20];
    char modifiedDates[100][20];
    unsigned int fileSizes[100];
    unsigned int firstClusterLows[100];
    int count;
} DirectoryListing;

/*
*********************************************************************************************************
*                                            API
*********************************************************************************************************
*/

/*
* @brief Read boot sector from image file
* @param[in] imagePath - Path to image file
* @return BootSector structure
*/
BootSector fat12_INIT(const char *imagePath, FILE *fp);

/*
* @brief Get first sector of Data Region
* @param[in] bs - BootSector structure
* @return First sector of Data Region
*/
unsigned int fat_GetFirstDataSector(const BootSector *bs);

/*
* @brief List directory entries
* @param[in] fp - File pointer
* @param[in] bs - BootSector structure
* @param[in] currentCluster - Cluster of current directory
* @param[in] fatOffset - Offset of FAT table
* @param[in] firstDataSector - First sector of Data Region
* @param[out] listing - DirectoryListing structure
*/
void fat_listDirectory(FILE *fp, const BootSector *bs, unsigned int cluster, DirectoryListing *listing, unsigned int fatOffset, unsigned int firstDataSector);

/*
* @brief Read file content
* @param[in] fp - File pointer
* @param[in] bs - BootSector structure
* @param[in] firstCluster - First cluster of file
* @param[in] fileSize - File size
* @param[in] fatOffset - Offset of FAT table
* @param[in] firstDataSector - First sector of Data Region
*/
void fat_readFileContent(FILE *fp, const BootSector *bs, unsigned int firstCluster, unsigned int fileSize, unsigned int fatOffset, unsigned int firstDataSector);

/*
* @brief Get next cluster of a file
* @param[in] currentCluster - Current cluster
* @param[in] fp - File pointer
* @param[in] fatOffset - Offset of FAT table
* @return Next cluster
*/
unsigned int fat_getNextCluster(unsigned int currentCluster, FILE *fp, unsigned int fatOffset);

/*
* @brief Lay sector cua mot cluster cu the
* @param[in] cluster - Cluster can tinh
* @param[in] bs - BootSector structure
* @param[in] firstDataSector - Sector dau tien cua Data Region
*/
unsigned int fat_GetSectorOfCluster(unsigned int cluster, const BootSector *bs, unsigned int firstDataSector);

/*
* @brief Doc ten file dai
* @param[in] lfnEntries - Mang cac entry cua LongFileName
* @param[in] lfnCount - So luong entry
* @param[out] longFileName - Ten file dai
*/
void getLongFileName(LongFileNameEntry *lfnEntries, int lfnCount, char *longFileName);

/*
* @brief Chuyen doi ngay va gio
* @param[in] date - Ngay
* @param[in] time - Gio
* @param[out] buffer - Buffer de luu ket qua
* @param[in] size - Kich thuoc cua buffer
*/
void convertDateTime(unsigned short date, unsigned short time, char *buffer, size_t size);

/*
* @brief Doc noi dung cua file
* @param[in] fp - File pointer
* @param[in] bs - BootSector structure
* @param[in] firstCluster - Cluster dau tien cua file
* @param[in] fileSize - Kich thuoc file
* @param[in] fatOffset - Offset cua FAT table
* @param[in] firstDataSector - Sector dau tien cua Data Region
*/
void readFileContent(FILE *fp, const BootSector *bs, unsigned int firstCluster, unsigned int fileSize, unsigned int fatOffset, unsigned int firstDataSector);









#endif // __FAT_H__


















