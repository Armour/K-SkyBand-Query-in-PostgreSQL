//
//  BucketVector.h
//  KeySkyBandQuery
//
//  Created by Armour on 7/14/15.
//  Copyright (c) 2015 ZJU. All rights reserved.
//

#ifndef KeySkyBandQuery_BucketVector_h
#define KeySkyBandQuery_BucketVector_h

#include "Point.h"
#include "Bucket.h"

void InitBucket(struct gtBucket *Bucket, int Dimension);
struct gtBucket *StartBucket(struct gtBucket *StartBucket, int *TotalSize, struct gtBucket **BucketHead, struct gtBucket **BucketTail, int Dimension);
void PushBucket(struct gtBucket *NewBucket, int *TotalSize, struct gtBucket **BucketTail);
struct gtBucket *GetBucket(int Position, struct gtBucket *BucketHead);
void DeleteBucket(int Position, struct gtBucket **BucketHead, int *TotalSize);
void FreeAllBuckets(struct gtBucket *Bucket, int *TotalSize);
int SizeBucket(struct gtBucket *BucketHead);

#endif
