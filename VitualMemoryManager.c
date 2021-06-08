#include <stdio.h>
#include <stdlib.h>
#include "VirtualMemoryManager.h"

//
int decodeAddress(int address, int *pageNumber, int *pageOffset){
    if (address < 0 || address > 65535){
        return 1;
    }
    else{
        *pageNumber = address / 256;
        *pageOffset = address % 256;
        return 0;
    }
}

int readFromBackingStore(FILE *fp, char *buffer, int pageNumber){
    int rsuccess = 0;
    fseek(fp, pageNumber*PAGE_SIZE, SEEK_SET);
    rsuccess = fread(buffer, PAGE_SIZE, 1, fp);
    if (rsuccess!=1)
        return 1;
    return 0;
}

int getIndexLeastRecent(PageTableStruct *pageTableInfo){
    int indexLeast = 0;
    int lowest = 100000000;

    for(int i = 0; i < NUM_FRAMES; i++){
        if(pageTableInfo->accessTime[i] < lowest){
            lowest = pageTableInfo->accessTime[i];
            indexLeast = i;
        }
    }
    return indexLeast;
}

int getFrameNumber(PageTableStruct *pageTableInfo, int logicalPageNumber, int accessTime, int *pageFault){
    int leastRecentFrame;

    // check if page is in memory already
    // case for if page has not been read in yet
    if(pageTableInfo->pageTable[logicalPageNumber] == -1){
        // find next available frame
        for(int i = 0; i < NUM_FRAMES; i++){
            if(pageTableInfo->freeFrame[i] == 1){
                // free frame found, update fields
                *pageFault = 1;
                pageTableInfo->accessTime[i] = accessTime;
                pageTableInfo->pageTable[logicalPageNumber] = i;
                pageTableInfo->freeFrame[i] = 0;

                // return the frame number
                return pageTableInfo->pageTable[logicalPageNumber];
            }
        }

        // case for if page is not in memory and all frames are occupied
        leastRecentFrame = getIndexLeastRecent(pageTableInfo);
        *pageFault = 1;
        int index;
        for(int i = 0; i < NUM_PAGES; i++){
            if(pageTableInfo->pageTable[i] == leastRecentFrame){
                pageTableInfo->pageTable[i] = -1;
                printf("EVICT! oldest frame is %d (age = %d)\n", leastRecentFrame,  pageTableInfo->accessTime[leastRecentFrame]);
                printf("The page mapped to frame %d is %d: page %d is now unmapped (not in memory)\n", leastRecentFrame, i, i);
            }
        }
        pageTableInfo->pageTable[logicalPageNumber] = leastRecentFrame;
        pageTableInfo->accessTime[leastRecentFrame] = accessTime;
        return leastRecentFrame;

    }
    // case for where the page is already in memory
    else {
        *pageFault = 0;
        // change access time of frame
        pageTableInfo->accessTime[pageTableInfo->pageTable[logicalPageNumber]] = accessTime;
        return pageTableInfo->pageTable[logicalPageNumber];
    }

}


int main() {
    char *infile = "addresses.txt";
    char *infileBS = "BACKING_STORE.dat";
    char bufferASCII[BUFLEN];
    FILE *fp;
    char *chp;
    int address;
    int accessTime;
    int numReads = 0, pageNumber = 0, pageOffset = 0, rc = 0;
    int pageFault;
    int frame;
    char byteValue;
    fp = fopen(infile, "r");

    // open files, check return codes
    FILE *BSfp;
    BSfp = fopen("BACKING_STORE.dat", "rb");

    if (fp == NULL) {
        fprintf(stderr, "cannot read file\n");
        return(8);
    }

    if (BSfp == NULL) {
        fprintf(stderr, "cannot read file\n");
        return(8);
    }

    // initialize default values for the page table
    PageTableStruct *pageTable = (PageTableStruct *) malloc(sizeof(PageTableStruct));
    for(int i = 0; i < NUM_FRAMES; i++){
        pageTable->accessTime[i] = -1;
        pageTable->freeFrame[i] = 1;
    }

    // set all pages to -1 to indicate not being in memory yet
    for(int i = 0; i < NUM_PAGES; i++){
        pageTable->pageTable[i] = -1;
    }

    // read in virtual address
    accessTime = 0;
    chp = fgets(bufferASCII, BUFLEN, fp);

    while(chp != NULL){
        // if read was successful, convert text address to integer
        address = atoi(bufferASCII);

        // get page number and offset
        rc = decodeAddress(address, &pageNumber, &pageOffset);

        // get the frame number
        frame = getFrameNumber(pageTable, pageNumber, accessTime, &pageFault);

        if(pageFault ==1){
            readFromBackingStore(BSfp, buffer, pageNumber);
            //save buffer to memory, starting at location frameNumber
            int start = frame * PAGE_SIZE;
            for(int i = 0; i < PAGE_SIZE; i++){
                physicalMemory[start + i] = buffer[i];
            }
            printf("* ");
        }
        else{
            printf("  ");
        }

        // get actual byte from memory
        byteValue = physicalMemory[frame * PAGE_SIZE + pageOffset];

        // display virtual address information to user
        printf("Virtual Address: %d [%d, %d], Physical address: %d [%d, %d] Value: %hhd\n", address, pageNumber, pageOffset, (frame * PAGE_SIZE +pageOffset), frame, pageOffset, byteValue);

        // read next virtual address
        chp = fgets(bufferASCII, BUFLEN, fp);
        numReads++;
        accessTime++;
    }

    fclose(fp);
    fclose(BSfp);
    return(0);
}
