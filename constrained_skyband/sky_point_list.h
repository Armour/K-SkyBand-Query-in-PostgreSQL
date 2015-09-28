/*-------------------------------------------------
 *
 *  sky_point_list.h
 *  KSkyBandQuery
 *
 *  Created by Armour on 7/14/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#ifndef KSKYBANDQUERY_SKYPOINTLIST_H
#define KSKYBANDQUERY_SKYPOINTLIST_H

#include "sky_point.h"

/* Initialize point */
void InitPoint(SkyPoint *point);

/* Start a head point of new list with d dimesions */
SkyPoint *StartPoint(int *size, SkyPoint **head, SkyPoint **tail, int d);

/* Push a point to a list, make it as tail */
void PushPoint(SkyPoint *point, int *size, SkyPoint **tail);

/* Delete point in any position p of the list */
void DeletePoint(int p, int *size, SkyPoint **head, SkyPoint **tail);

#endif
