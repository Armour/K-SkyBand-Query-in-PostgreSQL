#include <stdlib.h>
#include "PointVector.h"
#include "BucketVector.h"

void InitBucket(struct gtBucket *Bucket, int Dimension) {
    Bucket -> dataSize = 0;
    Bucket -> dataTail = NULL;
    Bucket -> data = StartPoint(Bucket->data, &Bucket->dataSize, &Bucket->data, &Bucket->dataTail, Dimension);
    Bucket -> Sl = StartPoint(Bucket->Sl, &Bucket->SlSize, &Bucket->Sl, &Bucket->SlTail, Dimension);
    Bucket -> Sln = StartPoint(Bucket->Sln, &Bucket->SlnSize, &Bucket->Sln, &Bucket->SlnTail, Dimension);;
    Bucket -> next = NULL;
    Bucket -> previous = NULL;
}

struct gtBucket *StartBucket(struct gtBucket *StartBucket, int *TotalSize, struct gtBucket **BucketHead, struct gtBucket **BucketTail, int Dimension) {
    StartBucket = (struct gtBucket *)palloc(sizeof(struct gtBucket));
    if (StartBucket == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a linked list.")));
    else{
        InitBucket(StartBucket, Dimension);
        *BucketHead = StartBucket;
        *BucketTail = StartBucket;
        *TotalSize = 1;
    }
    return StartBucket;
}

void PushBucket(struct gtBucket *NewBucket, int *TotalSize, struct gtBucket **BucketTail) {
    (*BucketTail) -> next = NewBucket;
    NewBucket -> previous = *BucketTail;
    NewBucket -> next = NULL;
    *BucketTail = NewBucket;
    *TotalSize = *TotalSize + 1;
}

struct gtBucket *GetBucket(int Position, struct gtBucket *BucketHead) {
    struct gtBucket *Pointer;
    int i = 0;
    Pointer = BucketHead;
    for (i = 0; i < Position; i++) {
        if (Pointer != NULL)
            Pointer = Pointer -> next;
        else break;
    }
    if (i != Position)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Illegal GetNode.")));
    return Pointer;
}

void DeleteBucket(int Position, struct gtBucket **BucketHead, int *TotalSize) {
    struct gtBucket *Pointer;
    int i = 0;
    Pointer = *BucketHead;
    for (i = 0; i < Position; i++) {
        if (Pointer->next != NULL){
            Pointer = Pointer -> next;
        }
        else break;
    }
    if (i != Position)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Illegal GetNode.")));
    Pointer -> previous -> next = Pointer -> next;
    Pointer -> next = NULL;
    Pointer -> previous = NULL;
    *TotalSize = *TotalSize - 1;
}

void FreeAllBuckets(struct gtBucket *Bucket, int *TotalSize) {
    struct gtBucket *Pointer;
    struct gtBucket *FreePointer;
    Pointer = Bucket;
    FreePointer = Bucket;
    while (Pointer != NULL) {
        Pointer = Pointer -> next;
        pfree(FreePointer);
        FreePointer = Pointer;
    }
    *TotalSize = 0;
}

int SizeBucket(struct gtBucket *BucketHead) {
    int count=0;
    while (BucketHead != NULL) {
        count++;
        BucketHead = BucketHead->next;
    }
    return count;
}
