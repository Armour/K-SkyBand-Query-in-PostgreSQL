/*-------------------------------------------------
 *
 *  sky_bucket_list.h
 *  KSkyBandQuery
 *
 *  Created by Armour on 7/14/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#ifndef KSKYBANDQUERY_SKYBUCKETLIST_H
#define KSKYBANDQUERY_SKYBUCKETLIST_H

#include "sky_point.h"
#include "sky_bucket.h"

/* Initialize bucket with d dimension */
void InitBucket(SkyBucket *bucket, int d);

/* Start a head bucket of new list with d dimesions */
SkyBucket *StartBucket(int *size, SkyBucket **head, SkyBucket **tail, int d);

/* Push a bucket to a list, make it as tail */
void PushBucket(SkyBucket *bucket, int *size, SkyBucket **tail);

#endif
