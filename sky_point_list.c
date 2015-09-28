/*-------------------------------------------------
 *
 *  sky_point_list.c
 *  KSkyBandQuery
 *
 *  Created by Armour on 7/14/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#include <stdlib.h>
#include "sky_point_list.h"

/*
 * Function: InitPoint
 * -------------------
 *   Initialize point
 *
 *   point: the point that needs to be initialized
 *
 *   returns: void
 */

void InitPoint(SkyPoint *point) {
    point -> dim = 0;
    point -> data = NULL;
    point -> bitmap = NULL;
    point -> cnt_domi = 0;              /* Count of dominated time should be zero at first */
    point -> index = -1;
    point -> next = NULL;
    point -> prev = NULL;
}

/*
 * Function: StartPoint
 * -------------------
 *   Start a new point with d dimesions, and then make it as the head of new list
 *
 *   size: the total size of this list, it equals to zero at first (ignored head point)
 *   head: the head of this point list, need palloc at first
 *   tail: the tail of this point list, equals to head at first
 *   d: the dim of this point list.
 *
 *   returns: the head point of new list, with d dims
 */

struct SkyPoint *StartPoint(int *size, SkyPoint **head, SkyPoint **tail, int d) {
    *head = (SkyPoint *)palloc(sizeof(SkyPoint));
    if (head == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Cannot start a linked list.")));
    else{
        InitPoint(*head);
        (*head)->dim = d;
        *tail = *head;                  /* Because there is only one head point */
        *size = 0;
    }
    return *head;
}

/*
 * Function: PushPoint
 * -------------------
 *   Pushing new point to the end of the list
 *
 *   point: the new point which need to push into list
 *   size: the total size of this list, here we just need to plus 1
 *   tail: the tail of this point list, will be updated to the new point
 *
 *   returns: void
 */

void PushPoint(SkyPoint *point, int *size, SkyPoint **tail) {
    if (*tail == NULL) return;
    (*tail) -> next = point;
    point -> prev = *tail;
    point -> next = NULL;
    *tail = point;
    *size = *size + 1;
}

/*
 * Function: DeletePoint
 * -------------------
 *   Deleting the point in position p in the list
 *
 *   p: the position of the point which we need to delete
 *   size: the total size of this list, here we just need to minus 1
 *   head: the head of this point list
 *   tail: the tail of this point list
 *
 *   returns: void
 */

void DeletePoint(int p, int *size, SkyPoint **head, SkyPoint **tail) {
    SkyPoint *point;                       /* A temp point */
    int i = 0;
    point = *head;
    for (i = 0; i < p; i++)                /* To find the point in the right position */
        if (point -> next != NULL)
            point = point -> next;
        else
            break;
    if (point -> prev != NULL) {           /* If deleted point is not the first point */
        if (point -> next == NULL)         /* If deleted point is the last point, then update tail */
            *tail = (*tail)->prev;
        if (i != p)
            ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Illegal GetNode.")));
        if (point -> prev != NULL)         /* Update prev point of deleted point */
            point -> prev -> next = point -> next;
        if (point -> next != NULL)         /* Update next point of deleted point */
            point -> next -> prev = point -> prev;
        point -> next = NULL;              /* Delete this point */
        point -> prev = NULL;
        *size = *size - 1;
    }
}
