/*-------------------------------------------------
 *
 *  sky_bucket_list.c
 *  KeySkyBandQuery
 *
 *  Created by Armour on 7/14/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#include <stdlib.h>
#include "sky_point_list.h"
#include "sky_bucket_list.h"

/*
 * Function: InitBucket
 * -------------------
 *   Initialize bucket
 *
 *   bucket: the bucket that needs to be initialized
 *   d: the dimension of this bucket list.
 *
 *   returns: void
 */

void InitBucket(SkyBucket *bucket, int d) {
    bucket -> data_size = 0;                 /* Init data list in bucket */
    bucket -> data_tail = NULL;
    bucket -> data_head = StartPoint(&bucket->data_size, &bucket->data_head, &bucket->data_tail, d);
    bucket -> sln_size = 0;                  /* Init sln list in bucket */
    bucket -> sln_tail = NULL;
    bucket -> sln_head = StartPoint(&bucket->sln_size, &bucket->sln_head, &bucket->sln_tail, d);;
    bucket -> next = NULL;
    bucket -> prev = NULL;
}

/*
 * Function: StartBucket
 * -------------------
 *   Start a new bucket with d dimesions, and then make it as the head of new list
 *
 *   size: the total size of this list, it equals to zero at first (ignored head point)
 *   head: the head of this bucket list, need palloc at first
 *   tail: the tail of this bucket list, equals to head at first
 *   d: the dimension of this bucket list.
 *
 *   returns: the head bucket of new list, with d dimensions
 */

SkyBucket *StartBucket(int *size, SkyBucket **head, SkyBucket **tail, int d) {
    *head = (SkyBucket *)palloc(sizeof(SkyBucket));
    if (head == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a linked list.")));
    else{
        InitBucket(*head, d);
        *tail = *head;              /* Because there is only one head point */
        *size = 0;
    }
    return *head;
}

/*
 * Function: PushBucket
 * -------------------
 *   Pushing new bucket to the end of the list
 *
 *   bucket: the new bucket which need to push into list
 *   size: the total size of this list, here we just need to plus 1
 *   tail: the tail of this point list, will be updated to the new bucket
 *
 *   returns: void
 */

void PushBucket(SkyBucket *bucket, int *size, SkyBucket **tail) {
    (*tail) -> next = bucket;
    bucket -> prev = *tail;
    bucket -> next = NULL;
    *tail = bucket;
    *size = *size + 1;
}
