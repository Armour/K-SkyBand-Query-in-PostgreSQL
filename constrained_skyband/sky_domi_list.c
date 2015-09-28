/*-------------------------------------------------
 *
 *  sky_domi_list.c
 *  KSkyBandQuery
 *
 *  Created by Armour on 9/26/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#include "sky_domi_list.h"
#include <stdlib.h>

/*
 * Function: InitDomiTable
 * -------------------
 *   Initialize a dominate table
 *
 *   n: the size of list in this dominate table.
 *
 *   returns: void
 */

DomiTable *InitDomiTable(int n) {
    DomiTable *d;
    int i;
    d = (DomiTable *)palloc(sizeof(DomiTable));
    if (d == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a dominate table.")));
    d->size = n;
    d->list = (DomiNode **)palloc(sizeof(DomiNode *) * d->size);
    if (d->list == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a dominate table.")));
    for (i = 0; i < d->size; i++) {
        d->list[i] = (DomiNode *)palloc(sizeof(DomiNode));
        if (d->list[i] == NULL)
            ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a dominate table.")));
        (d->list[i])->index = -1;
        (d->list[i])->next = NULL;
    }
    return d;
}

/*
 * Function: AddDomiPair
 * -------------------
 *   Insert into domitable d with a pair of nodes, which the node of index x dominate the node of index y
 *
 *   d: dominate table
 *   x: index of the dominate node
 *   y: index of the dominated node
 *
 *   returns: void
 */

void AddDomiPair(DomiTable *d, int x, int y) {
    DomiNode *p;
    DomiNode *new_node;
    new_node = (DomiNode *)palloc(sizeof(DomiNode));
    if (new_node == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot create a new dominate pair.")));
    new_node->index = y;
    new_node->next = NULL;
    p = d->list[x];
    while (p->next != NULL)
        p = p->next;
    p->next = new_node;
}

/*
 * Function: IsDomiPairExist
 * -------------------
 *   Test if the node of index x dominate the node of node y exist in dominate table d
 *
 *   d: dominate table
 *   x: index of the dominate node
 *   y: index of the dominated node
 *
 *   returns: void
 */

int IsDomiPairExist(DomiTable *d, int x, int y) {
    DomiNode *p;
    p = d->list[x];
    while (p != NULL) {
        if (p->index == y) {
            return 1;
        }
        p = p->next;
    }
    return 0;
}

