/*-------------------------------------------------
 *
 *  sky_hashtable.c
 *  KSkyBandQuery
 *
 *  Created by Armour on 7/23/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#include "sky_hashtable.h"

/*
 * Function: NextPrime
 * -------------------
 *   Give a prime number as hashtable size that is suitble for n
 *
 *   n: the maximal points that may uesd in hashtable
 *
 *   returns: prime number
 */

int NextPrime(int n) {                  /* Prime numbers were recommended from web */
    if (n < 53) return 53;
    else if (n < 97) return 97;
    else if (n < 193) return 193;
    else if (n < 389) return 389;
    else if (n < 769) return 769;
    else if (n < 1543) return 1543;
    else if (n < 3079) return 3079;
    else if (n < 6151) return 6151;
    else if (n < 12289) return 12289;
    else if (n < 24593) return 24593;
    else if (n < 49157) return 49157;
    else if (n < 98317) return 98317;
    else if (n < 196613) return 196613;
    else if (n < 393241) return 393241;
    else if (n < 786433) return 786433;
    else if (n < 1572869) return 1572869;
    else if (n < 3145739) return 3145739;
    else if (n < 6291469) return 6291469;
    else return 12582917;               /* Here I recommend do not have a sky_count more then 10000000, it wil be realllllly slow! */
}

/*
 * Function: BKDRHash
 * -------------------
 *   Hash function that used to calculate a hashValue for str with dimension d and limitation m
 *
 *   str: the character array that used to hash
 *   m: the limitation of hashValue
 *   d: the dimension of str (in other words, called "length")
 *
 *   returns: hashed value
 */

unsigned int BKDRHash(char *str, int m, int d) {
    unsigned int seed = 131;            /* Seed like 31 131 1313 13131 131313 etc.. */
    unsigned int hash_value = 0;
    int i = 0;
    while (i < d) {
        hash_value = hash_value * seed + *(str + i);
        i++;
        if ((int)hash_value >= m)
            hash_value %= m;
    }
    return(hash_value % m);
}

/*
 * Function: SameBitmap
 * -------------------
 *   Compare to see weather a has the same bitmap with b or not
 *
 *   a: the compared characters array
 *   b: the compared characters array
 *   d: the dimension of a and b (in other words, called "length")
 *
 *   returns: if same returns 1, else 0
 */

int SameBitmap(char *a, char* b, int d) {
    int i;
    for (i = 0; i < d; i++)
        if (*(a + i) != *(b + i))
            return 0;
    return  1;
}

/*
 * Function: InitHashTable
 * -------------------
 *   Initialize a hashtable with size of n
 *
 *   n: size of the new hashtable
 *
 *   returns: the pointer to the new hashtable
 */

HashTable* InitHashTable(int n) {
    HashTable* h;
    int i;
    h = (HashTable *)palloc(sizeof(HashTable));
    if (h == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a hash table.")));
    h->size = NextPrime(n);                                         /* Better be prime */
    h->list = (ListNode **)palloc(sizeof(ListNode *) * h->size);
    if (h->list == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a hash table's lists.")));
    for( i = 0; i < h->size; i++ ) {                                /* Allocate list headers */
        h->list[i] = (ListNode *)palloc(sizeof(ListNode));
        if (h->list == NULL)
            ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a hash table's list node.")));
        h->list[i]->next = NULL;
    }
    return h;
}

/*
 * Function: Find
 * -------------------
 *   Find the list that has the bitmap b and return the first node of this list
 *
 *   b: bitmap that want to find in hashtable
 *   h: hashtable used to find things
 *   d: the dimension of bitmap b (in other words, called "length")
 *
 *   returns: the pointer to the new hashtable
 */

ListNode *Find(char *b, HashTable *h, int d) {
    ListNode *p;
    p = h->list[BKDRHash(b, h->size, d)]->next;
    while (p != NULL && !SameBitmap(p->bitmap, b, d))
        p = p->next;
    return p;
}

/*
 * Function: Insert
 * -------------------
 *   Insert the list that has the bitmap b and make it head node of that list
 *
 *   b: bitmap that want to insert in hashtable
 *   h: hashtable used to insert things
 *   d: the dimension of bitmap b (in other words, called "length")
 *   bucket: the bucket that you want to insert
 *   first_bucket: first bucket in list, if it is NULL, then make the bucket that you insert to be the first bucket
 *   last_bucket: last bucket in list, update to the bucket that you insert
 *
 *   returns: void
 */

void Insert(char *b, HashTable *h, int d, SkyBucket* bucket, SkyBucket** first_bucket, SkyBucket** last_bucket) {
    int i;
    ListNode *p;                                        /* A new listnode uesd to insert into hashtable */
    p = (ListNode *)palloc(sizeof(ListNode));
    if (p == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a insert.")));
    p->bucket = bucket;
    p->next = h->list[BKDRHash(b, h->size, d)]->next;   /* Set nextNode of this new listnode */
    h->list[BKDRHash(b, h->size, d)]->next = p;
    p->bitmap = (char *)palloc(sizeof(char) * d);       /* Set bitmap of this new listnode */
    for (i = 0; i < d; i++)
        *((p->bitmap) + i) = *(b + i);
    if (*first_bucket == NULL) {                        /* Check if it is the first bucket */
        *first_bucket = p->bucket;
        *last_bucket = p->bucket;
    } else {
        (*last_bucket)->next = p->bucket;
        *last_bucket = p->bucket;
    }
}
