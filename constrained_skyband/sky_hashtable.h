/*-------------------------------------------------
 *
 *  sky_hashtable.h
 *  KSkyBandQuery
 *
 *  Created by Armour on 7/23/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#ifndef KSKYBANDQUERY_SKYHASHTABLE_H
#define KSKYBANDQUERY_SKYHASHTABLE_H

#include <stdlib.h>
#include "sky_bucket_list.h"

typedef struct ListNode {
    char *bitmap;                       /* Bitmap of this listnode */
    SkyBucket *bucket;                  /* Stored bucket of this listnode */
    struct ListNode *next;              /* Next node in the list */
} ListNode;

typedef struct HashTable {
    int size;                           /* The size of hashtable */
    ListNode **list;                    /* The list of nodes where each one stores a bucket list */
} HashTable;

/* Initialize hashtable with size n */
HashTable *InitHashTable(int n);

/* Find the list of nodes which has the same bitmap with b (from hashtable h) */
ListNode *Find(char *b, HashTable *h, int d);

/* Insert into hashtable h a bucket with bitmap b, make it the first node of a list */
void Insert(char *b, HashTable *h, int d, SkyBucket* bucket, SkyBucket** first_bucket, SkyBucket** last_bucket);

/* Used to get the next prime number of n */
int NextPrime(int n);

/* Used to compare a to b to see if they have the same bitmap */
int SameBitmap(char *a, char* b, int d);

/* BKDR hash function */
unsigned int BKDRHash(char *str, int m, int d);

#endif
