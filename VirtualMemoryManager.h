
#ifndef VITUALMEMORYMANAGER_VIRTUALMEMORYMANAGER_H
#define VITUALMEMORYMANAGER_VIRTUALMEMORYMANAGER_H

#include <stdio.h>
#define NUM_PAGES 256
#define PAGE_SIZE 256
#define NUM_FRAMES 128
#define BUFLEN 256

char physicalMemory[NUM_FRAMES * PAGE_SIZE];
char buffer[10000];

// struct to represent the page table
typedef struct {
    int pageTable[NUM_PAGES]; // the actual page table
    int accessTime[NUM_FRAMES]; // the most recent time that a frame was accessed
    unsigned char freeFrame[NUM_FRAMES]; // 0 if a frame is occupied; otherwise 1
} PageTableStruct;


// function to get the page number and offset for a specific address.
// return 0 if there was no error, otherwise return 1
int decodeAddress(int address, int *pageNumber, int *pageOffset);


// read bytes n to n+PAGE_SIZE-1, where n = PAGE_SIZE * pageNumber;
// put the data into the location pointed to by buffer
// return 0 if there was no error during the read; otherwise return 1
int readFromBackingStore(FILE *fp, char *buffer, int pageNumber);


// function to manage the page table. Handles page faults/evictions
// and updates the page table accordingly
int getFrameNumber(PageTableStruct *pageTableInfo,
                   int logicalPageNumber,
                   int accessTime,
                   int *pageFault);

#endif //VITUALMEMORYMANAGER_VIRTUALMEMORYMANAGER_H
