/*-------------------------------------------------
 *
 *  sky_bucket.h
 *  KSkyBandQuery
 *
 *  Created by Armour on 7/10/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#ifndef KSKYBANDQUERY_SKYBUCKET_H
#define KSKYBANDQUERY_SKYBUCKET_H

#include "sky_point.h"

typedef struct SkyBucket {
    int data_size;
    SkyPoint *data_head;
    SkyPoint *data_tail;         // Used to store points in this bucket

    int sln_size;
    SkyPoint *sln_head;
    SkyPoint *sln_tail;          // Store Sln points in this bucket

    struct SkyBucket *next;
    struct SkyBucket *prev;      // Stored as a list
} SkyBucket;

#endif
