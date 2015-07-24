//
//  HashTable.c
//  KeySkyBandQuery
//
//  Created by Armour on 7/23/15.
//  Copyright (c) 2015 ZJU. All rights reserved.
//

#include "HashTable.h"

int NextPrime(int size) {
    if (size < 53)
        return 53;
    else if (size < 97)
        return 97;
    else if (size < 193)
        return 193;
    else if (size < 389)
        return 389;
    else if (size < 769)
        return 769;
    else if (size < 1543)
        return 1543;
    else if (size < 3079)
        return 3079;
    else if (size < 6151)
        return 6151;
    else if (size < 12289)
        return 12289;
    else if (size < 24593)
        return 24593;
    else if (size < 49157)
        return 49157;
    else if (size < 98317)
        return 98317;
    else if (size < 196613)
        return 196613;
    else if (size < 393241)
        return 393241;
    else if (size < 786433)
        return 786433;
    else if (size < 1572869)
        return 1572869;
    else if (size < 3145739)
        return 3145739;
    else if (size < 6291469)
        return 6291469;
    else
        return 12582917;
}

// BKDR Hash Function
unsigned int Hash(char *str, int M, int dataDimension) {
    unsigned int seed = 131 ; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0 ;
    int i = 0;
    while (i < dataDimension) {
        hash = hash * seed + *(str + i);
        i++;
    }
    return(hash % M);
}

int sameBitmap(char *A, char* B, int dataDimension) {
    int i;
    for (i = 0; i < dataDimension; i++)
        if (*(A + i) != *(B + i)) {
            return 0;
        }
    return  1;
}

struct HashTable* InitializeTable(int TableSize) {
    struct HashTable* H;
    int i;
    H = (struct HashTable *)malloc(sizeof(struct HashTable));  /* Allocate table */
    //if (H == NULL)
    //    FatalError( "Out of space!!!" );
    H->TableSize = NextPrime(TableSize);  /* Better be prime */
    H->TheLists = (struct ListNode **)malloc(sizeof(struct ListNode *) * H->TableSize );  /*Array of lists*/
    //if ( H->TheLists == NULL )
    //    FatalError( "Out of space!!!" );
    for( i = 0; i < H->TableSize; i++ ) {   /* Allocate list headers */
        H->TheLists[i] = (struct ListNode *)malloc(sizeof(struct ListNode)); /* Slow! */
        //if ( H->TheLists[i] == NULL )
        //    FatalError( "Out of space!!!" );
        //else
        H->TheLists[i]->Next = NULL;
    }
    return H;
}

struct ListNode *Find(char *bitmap, struct HashTable *H, int dataDimension) {
    struct ListNode *P;
    struct ListNode *L;
    L = H->TheLists[Hash(bitmap, H->TableSize, dataDimension)];
    P = L->Next;
    while (P != NULL && !sameBitmap(P->bitmap, bitmap, dataDimension))
        P = P->Next;
    return P;
}

void Insert(char *bitmap, struct HashTable *H, int dataDimension, struct gtBucket* bucket, struct gtBucket** firstBucket, struct gtBucket** lastBucket) {
    struct ListNode *Pos, *NewCell;
    struct ListNode *L;
    int i;
    Pos = Find(bitmap, H, dataDimension);
    if (Pos == NULL) {   /* Key is not found, then insert */
        NewCell = (struct ListNode *)malloc(sizeof(struct ListNode));
        NewCell->bitmap = (char *)malloc(sizeof(char) * dataDimension);
        NewCell->bucket = bucket;
        //if (NewCell == NULL)
        //    FatalError( "Out of space!!!" );
        //else {
        L = H->TheLists[Hash(bitmap, H->TableSize, dataDimension)];
        NewCell->Next = L->Next;
        for (i = 0; i < dataDimension; i++)
            *((NewCell->bitmap) + i) = *(bitmap + i);
        L->Next = NewCell;
        //}
        if (*firstBucket == NULL) {
            *firstBucket = NewCell->bucket;
            *lastBucket = NewCell->bucket;
        } else {
            (*lastBucket)->next = NewCell->bucket;
            *lastBucket = NewCell->bucket;
        }
    }
}