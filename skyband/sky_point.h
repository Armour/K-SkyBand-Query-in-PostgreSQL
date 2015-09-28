/*-------------------------------------------------
 *
 *  sky_point.h
 *  KSkyBandQuery
 *
 *  Created by Armour on 7/10/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#ifndef KSKYBANDQUERY_SKYPOINT_H
#define KSKYBANDQUERY_SKYPOINT_H

#include "postgres.h"
#include "fmgr.h"

typedef struct SkyPoint {
    int dim;                    /* Point dimension */
    double **data;               /* Stored data of this point */
    char *bitmap;               /* Bitmap of this point */
    int cnt_domi;               /* The dominated count of this point */
    int index;                  /* The index of this point */
    struct SkyPoint *next;
    struct SkyPoint *prev;      /* Stored as a list */
} SkyPoint;

#endif
