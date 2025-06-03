#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hal.h"
#include "fat.h"
#include "app.h"

// Hàm thêm một thư mục vào ngăn xếp
void pushStack(DirectoryStack **top, unsigned int cluster, const char *path) {
    DirectoryStack *newNode = (DirectoryStack *)malloc(sizeof(DirectoryStack));
    newNode->cluster = cluster;
    strncpy(newNode->path, path, MAX_PATH);
    newNode->next = *top;
    *top = newNode;
}

// Hàm loại bỏ một thư mục khỏi ngăn xếp
unsigned int popStack(DirectoryStack **top, char *path) {
    if (*top == NULL) return 0;
    DirectoryStack *temp = *top;
    unsigned int cluster = temp->cluster;
    strncpy(path, temp->path, MAX_PATH);
    *top = temp->next;
    free(temp);
    return cluster;
}


int app_Display_FAT12(BootSector bs, FILE *fp)
{
     // Tính toán các giá trị cần thiết
    unsigned int fatOffset = bs.reservedSectors * bs.bytesPerSector;
    unsigned int firstDataSector = fat_GetFirstDataSector(&bs);
    unsigned int rootDirSector = bs.reservedSectors + (bs.numberOfFATs * bs.sectorsPerFAT);
    unsigned int rootDirSize = (bs.rootEntryCount * DIR_ENTRY_SIZE) / bs.bytesPerSector;

    // Khởi tạo ngăn xếp thư mục với thư mục gốc (cluster 0)
    DirectoryStack *stack = NULL;
    char currentPath[MAX_PATH] = "Root";
    pushStack(&stack, 0, currentPath);

    while (stack != NULL) {
        unsigned int currentCluster = stack->cluster;
        DirectoryListing listing;
        fat_listDirectory(fp, &bs, currentCluster, &listing, fatOffset, firstDataSector);
        

        // Hiển thị đường dẫn hiện tại
        printf("\n--- Directory: %s ---\n", currentPath);

        // Hiển thị danh sách các mục
        printf("%-5s %-30s %-10s %-20s %-20s %-10s\n", "No.", "Name", "Type", "Created", "Last Modified", "Size");
        printf("------------------------------------------------------------------------------------------\n");
        for (int i = 0; i < listing.count; i++) {
            printf("[%d] %-30s %-10s %-20s %-20s %-10u\n", i + 1, listing.fileNames[i], listing.fileTypes[i],
                   listing.createdDates[i], listing.modifiedDates[i], listing.fileSizes[i]);
        }
        printf("[D] Enter Directory | [F] View File | [B] Back | [Q] Quit\n");

        // Nhập lựa chọn từ người dùng
        char choice[10];
        printf("\nEnter choice (e.g., D1 to enter directory 1, F2 to view file 2, B to go back, Q to quit): ");
        scanf("%s", choice);

        if (strlen(choice) == 0) {
            printf("Invalid input.\n");
            continue;
        }

        if (choice[0] == 'Q' || choice[0] == 'q') {
            break;
        } else if (choice[0] == 'B' || choice[0] == 'b') {
            char parentPath[MAX_PATH];
            unsigned int parentCluster = popStack(&stack, parentPath);
            if (stack != NULL) {
                strncpy(currentPath, parentPath, MAX_PATH);
            } else {
                // Nếu ngăn xếp trống, quay lại thư mục gốc
                strncpy(currentPath, "Root", MAX_PATH);
            }
        } else if (choice[0] == 'D' || choice[0] == 'd') {
            int dirIndex = atoi(&choice[1]) - 1;
            if (dirIndex >= 0 && dirIndex < listing.count && strcmp(listing.fileTypes[dirIndex], "Directory") == 0) {
                unsigned int newCluster = listing.firstClusterLows[dirIndex];
                if (newCluster == 0) {
                    printf("Invalid cluster for directory.\n");
                    continue;
                }
                // Cập nhật đường dẫn
                char newPath[MAX_PATH];
                snprintf(newPath, sizeof(newPath), "%s/%s", currentPath, listing.fileNames[dirIndex]);

                pushStack(&stack, newCluster, newPath);
                strncpy(currentPath, newPath, MAX_PATH);
            } else {
                printf("Invalid directory selection.\n");
            }
        } else if (choice[0] == 'F' || choice[0] == 'f') {
            int fileIndex = atoi(&choice[1]) - 1;
            if (fileIndex >= 0 && fileIndex < listing.count && strcmp(listing.fileTypes[fileIndex], "File") == 0) {
                printf("\nDetails of file %d:\n", fileIndex + 1);
                printf("Name: %s\n", listing.fileNames[fileIndex]);
                printf("Type: %s\n", listing.fileTypes[fileIndex]);
                printf("Created: %s\n", listing.createdDates[fileIndex]);
                printf("Last Modified: %s\n", listing.modifiedDates[fileIndex]);
                printf("Size: %u bytes\n", listing.fileSizes[fileIndex]);

                // Đọc và hiển thị nội dung tệp
                if (listing.fileSizes[fileIndex] > 0) {
                    printf("\nContent:\n");
                    readFileContent(fp, &bs, listing.firstClusterLows[fileIndex], listing.fileSizes[fileIndex], fatOffset, firstDataSector);
                }
            } else {
                printf("Invalid file selection.\n");
            }
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    // Giải phóng ngăn xếp
    while (stack != NULL) {
        char tempPath[MAX_PATH];
        popStack(&stack, tempPath);
    }

}









