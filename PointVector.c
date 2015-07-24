#include <stdlib.h>
#include "PointVector.h"

void InitPoint(struct gtPoint *Node) {
    Node -> dimension = 0;
    Node -> data = NULL;
    Node -> bitmap = NULL;
    Node -> domainatedCount = 0;
    Node -> next = NULL;
    Node -> previous = NULL;
}

struct gtPoint *StartPoint(struct gtPoint *StartNode, int *TotalSize, struct gtPoint **PointHead, struct gtPoint **PointTail, int dataDimension) {
    StartNode = (struct gtPoint *)palloc(sizeof(struct gtPoint));
    if (StartNode == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a linked list.")));
    else{
        InitPoint(StartNode);
        StartNode->dimension = dataDimension;
        *PointHead = StartNode;
        *PointTail = StartNode;
        *TotalSize = 1;
    }
    return StartNode;
}

void PushPoint(struct gtPoint *NewElement, int *TotalSize, struct gtPoint **PointTail) {
    if (*PointTail == NULL) return;
    (*PointTail) -> next = NewElement;
    NewElement -> previous = *PointTail;
    NewElement -> next = NULL;
    *PointTail = NewElement;
    *TotalSize = *TotalSize + 1;
}

struct gtPoint *GetPoint(int Position, struct gtPoint *PointHead) {
    struct gtPoint *Pointer;
    int i = 0;
    Pointer = PointHead;
    for (i = 0; i < Position; i++) {
        if (Pointer -> next != NULL || i == Position - 1)
            Pointer = Pointer -> next;
        else break;
    }
    if (i != Position)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Illegal GetNode.")));
    return Pointer;
}

void DeletePoint(int Position, struct gtPoint **PointHead, int *TotalSize, struct gtPoint **PointTail) {
    struct gtPoint *Pointer;
    int i = 0;
    Pointer = *PointHead;
    for (i = 0; i < Position; i++) {
        if (Pointer -> next != NULL)
            Pointer = Pointer -> next;
        else
            break;
    }
    if (Pointer -> next == NULL) {      // if delete the last node
        *PointTail = (*PointTail)->previous;
    }
    if (i != Position)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Illegal GetNode.")));
    Pointer -> previous -> next = Pointer -> next;
    if (Pointer -> next != NULL)
        Pointer -> next -> previous = Pointer -> previous;
    Pointer -> next = NULL;
    Pointer -> previous = NULL;
    *TotalSize = *TotalSize - 1;
}

void FreeAllPoints(struct gtPoint *Node, int *TotalSize) {
    struct gtPoint *Pointer;
    struct gtPoint *FreePointer;
    Pointer = Node;
    FreePointer = Node;
    while (Pointer != NULL) {
        Pointer = Pointer -> next;
        pfree(FreePointer);
        FreePointer = Pointer;
    }
    *TotalSize = 0;
}

int SizePoint(struct gtPoint *PointHead) {
    int count = 0;
    while (PointHead != NULL) {
        count++;
        PointHead = PointHead->next;
    }
    return count;
}
