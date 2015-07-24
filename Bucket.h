//
//  Bucket.h
//  KeySkyBandQuery
//
//  Created by Armour on 7/10/15.
//  Copyright (c) 2015 ZJU. All rights reserved.
//

#ifndef KeySkyBandQuery_Bucket_h
#define KeySkyBandQuery_Bucket_h

#include "Point.h"

struct gtBucket {
    int dataSize;
    int SlSize;
    int SlnSize;
    struct gtPoint *data;
    struct gtPoint *dataTail;
    struct gtPoint *Sl;
    struct gtPoint *SlTail;
    struct gtPoint *Sln;
    struct gtPoint *SlnTail;
    struct gtBucket *next;
    struct gtBucket *previous;
};

#endif
