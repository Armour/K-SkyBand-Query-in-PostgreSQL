//
//  PointVector.h
//  KeySkyBandQuery
//
//  Created by Armour on 7/14/15.
//  Copyright (c) 2015 ZJU. All rights reserved.
//

#ifndef KeySkyBandQuery_PointVector_h
#define KeySkyBandQuery_PointVector_h

#include "Point.h"

void InitPoint(struct gtPoint *Node);
struct gtPoint *StartPoint(struct gtPoint *StartNode, int *TotalSize, struct gtPoint **PointHead, struct gtPoint **PointTail, int dataDimension);
void PushPoint(struct gtPoint *NewElement, int *TotalSize, struct gtPoint **PointTail);
struct gtPoint *GetPoint(int Position, struct gtPoint *PointHead);
void DeletePoint(int Position, struct gtPoint **PointHead, int *TotalSize, struct gtPoint **PointTail);
void FreeAllPoints(struct gtPoint *Node, int *TotalSize);
int SizePoint(struct gtPoint *PointHead);

#endif
