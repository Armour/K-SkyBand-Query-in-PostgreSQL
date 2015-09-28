/*-------------------------------------------------
 *
 *  sky_domi_list.h
 *  KSkyBandQuery
 *
 *  Created by Armour on 9/26/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#ifndef KSKYBANDQUERY_SKY_DOMI_LIST_H
#define KSKYBANDQUERY_SKY_DOMI_LIST_H

#include "postgres.h"
#include "fmgr.h"

typedef struct DomiNode {
    int index;                          /* Store the index of point which dominate a certain point */
    struct DomiNode *next;              /* Next dominated index of the certain point */
} DomiNode;

typedef struct DomiTable {
    int size;                           /* The size of dominate table */
    struct DomiNode **list;             /* The list of nodes where each one stores a dominate list */
} DomiTable;

/* Initialize dominate table with size n */
DomiTable *InitDomiTable(int n);

/* Insert into domitable d with a pair of nodes, which the node of index x dominate the node of index y */
void AddDomiPair(DomiTable *d, int x, int y);

/* Test if the node of index x dominate the node of node y exist in dominate table d */
int IsDomiPairExist(DomiTable *d, int x, int y);

#endif

