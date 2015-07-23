//
//  main.cpp
//  KeySkyBandQuery
//
//  Created by Armour on 7/10/15.
//  Copyright (c) 2015 ZJU. All rights reserved.
//

#include "postgres.h"
#include "fmgr.h"
#include "stdlib.h"
#include "funcapi.h"
#include "executor/spi.h"
#include "utils/builtins.h"
#include "PointVector.h"
#include "BucketVector.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(skyline_in);

Datum skyline_in(PG_FUNCTION_ARGS);

bool isPoint1DominatePoint2(struct gtPoint *p1, struct gtPoint *p2);
int gtSortAlgo(const struct gtPoint *v1, const struct gtPoint *v2);
void thicknessWarehouse(int dataDimension, int kValue);

int two[29];
int kValue;
int dataDimension;
int dataCount;
int *tmpInt;
int32 **tmpIntStar;
int *dominateBucket;

int tmpSize = 0;
int SSize = 0; //Total data size.
int StwhSize = 0, SesSize = 0, SgSize = 0;
int bucketSize = 0;
struct gtPoint *retPoint;
struct gtPoint *tmpInput, *tmpHead, *tmpTail;
struct gtPoint *S, *SHead, *STail;
struct gtPoint *Stwh, *StwhHead, *StwhTail;
struct gtPoint *Ses, *SesHead, *SesTail;
struct gtPoint *Sg, *SgHead, *SgTail;
struct gtBucket *bucket;
struct gtBucket *bucketHead, *bucketTail;

bool isPoint1DominatePoint2(struct gtPoint *p1, struct gtPoint *p2) {
    int32 x1, x2;
    int i;
    int dimension = p1->dimension;
    int smallCount = 0;
    int atLeastOneSmall = 0;
    bool x1IsNull, x2IsNull;
    if (!p1 || !p2) return 0;
    for (i = 0; i < dimension; i++) {
        x1 = *(*(p1->data)+i);
        x2 = *(*(p2->data)+i);
        x1IsNull = !(p1->bitmap & two[dimension - i]);
        x2IsNull = !(p2->bitmap & two[dimension - i]);
        if (x1IsNull || x2IsNull) {
            smallCount++;
        } else {
            if (x1 <= x2) smallCount++;
            if (x1 < x2) atLeastOneSmall = 1;
        }
    }
    if ((smallCount == dimension) && atLeastOneSmall) 
	return 1;
    else 
	return 0;
}

int gtSortAlgo(const struct gtPoint *v1, const struct gtPoint *v2) {
    return v1->domainatedCount > v2->domainatedCount;
}

void thicknessWarehouse(int dataDimension, int kValue) {
    int i, j, k;
    int bucketCount = 1;
    int iterCount = 0, iterCountB;

    struct gtPoint *iterA;
    struct gtPoint *iterB;
    struct gtPoint *tmpPoint = NULL;
    struct gtPoint *tmpPoint2 = NULL;
    struct gtPoint *tmpPointNext;
    struct gtPoint **tmpPointArray;
    struct gtBucket *tmpBucket = NULL;    

    Stwh = StartPoint(Stwh, &StwhSize, &StwhHead, &StwhTail, dataDimension);
    Ses = StartPoint(Ses, &SesSize, &SesHead, &SesTail, dataDimension);
    Sg = StartPoint(Sg, &SgSize, &SgHead, &SgTail, dataDimension);

    // [STEP 1] Push all points in S to every bucket according to bitmap
    for (i = 0; i < dataDimension; i++)
        bucketCount *= 2;   // !!!!!! Dimension should not larger then 30 !!!!!!!!

    ////////////////////////////////////////////////////
    // Origin: bucket = new gtBucket[bucketCount];
    bucket = StartBucket(bucket, &bucketSize, &bucketHead, &bucketTail, dataDimension); 

    for (i = 0; i < bucketCount; i++) {
        tmpBucket = (struct gtBucket *)palloc(sizeof(struct gtBucket));  // 1 bucket + 3 point => 232 bytes
    	if (tmpBucket == NULL)
    	    ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Step1: Cannot palloc bucket")));
        InitBucket(tmpBucket, dataDimension);
        PushBucket(tmpBucket, &bucketSize, &bucketTail);
    }
    ////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // Origin: bucket[S[i]->bitmap].data.push_back(S[i]);
    tmpPoint = S;
    tmpPointNext = tmpPoint->next;
    while (tmpPointNext != NULL) {
        tmpPoint = tmpPointNext;
        tmpPointNext = tmpPoint->next;
        tmpBucket = GetBucket(tmpPoint->bitmap, bucketHead); // !! how to save time, there should be a trade off between time and space
        PushPoint(tmpPoint, &tmpBucket->dataSize, &tmpBucket->dataTail);
    }
    ////////////////////////////////////////////////////

    elog(INFO, "Step1 over!!");

    // [STEP 2] Divide points in every bucket into Sl and Sln
    tmpBucket = bucket;
    tmpPointArray = (struct gtPoint **)palloc(sizeof(struct gtPoint*) * tmpBucket->dataSize);
    if (tmpPointArray == NULL)
        ereport(ERROR, (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Step2: Cannot palloc PointArray for insert point into Sln")));
    for (i = 0; i < bucketCount; i++) {
        tmpBucket->bitmap = i;
        tmpPoint = tmpBucket->data;
        tmpPointArray[0] = tmpPoint;
        for (j = 1; j < tmpBucket->dataSize; j++) {
            tmpPoint = tmpPoint->next;
            tmpPointArray[j] = tmpPoint;
        }
        for (j = 1; j < tmpBucket->dataSize; j++) {
            tmpPoint = tmpPointArray[j];
            for (k = 1; k < tmpBucket->dataSize; k++) {
                tmpPoint2 = tmpPointArray[k];
                if (j != k ) {
                    if (isPoint1DominatePoint2(tmpPoint2, tmpPoint)) {
                        tmpPoint->domainatedCount++;
                        if (tmpPoint->domainatedCount >= kValue) {
                            PushPoint(tmpPoint, &tmpBucket->SlnSize, &tmpBucket->SlnTail);
                            break;
                        }
                    }
                }
			}
            if (k == tmpBucket->dataSize) // which means data[j] is not dominted more than k times, then put it into Sl.
                PushPoint(tmpPoint, &StwhSize, &StwhTail);
	}
        tmpBucket = tmpBucket->next;
    }

    pfree(tmpPointArray);
    
    elog(INFO, "Step2,3 over!!");

    // [STEP 4] Push Swth -> Ses
    // std::sort(Stwh.begin(), Stwh.end(), gtSortAlgo);


    /////////////////////////////////////////////////////////////////////////////////////
    // Origin:
    // vector<gtPoint *>::iterator itHead, itTail;
    // for (itHead = Stwh.begin(); itHead != Stwh.end(); itHead++) {
    //    if(!*itHead) continue;
    //    for (itTail = Stwh.end(); itTail != Stwh.begin(); itTail--) {
    //        if(!*itTail) continue;
    //        if (isPoint1DominatePoint2(*itTail, *itHead)) (*itHead)->domainatedCount ++;
    //        if ((*itHead)->domainatedCount > kValue) {
    //            Ses.push_back(*itHead);
    //            Stwh.erase(itHead);
    //            break;
    //        }
    //        if (isPoint1DominatePoint2(*itHead, *itTail)) (*itTail)->domainatedCount ++;
    //        if ((*itTail)->domainatedCount > kValue) {
    //            Ses.push_back(*itTail);
    //            Stwh.erase(itTail);
    //        }
    //    }
    // }

    iterCount = 0;
    iterA = Stwh->next;
    while (iterA != NULL) {
        iterCount++;
        tmpPointNext = iterA->next;
        iterB = StwhTail;
        iterCountB = 0;
        while (iterB != StwhHead) {
            iterCountB++;
            tmpPoint = iterB->previous;
            if (iterA->bitmap == iterB->bitmap ) {
                iterB = tmpPoint;
                continue;
            }
            if (isPoint1DominatePoint2(iterB, iterA)) {
                iterA->domainatedCount++;
                if (iterA->domainatedCount >= kValue) {
                    DeletePoint(iterCount, &StwhHead, &StwhSize, &StwhTail);
                    PushPoint(iterA, &SesSize, &SesTail);
                    iterCount--;
                    break;
                }
            }
            if (isPoint1DominatePoint2(iterA, iterB)) {
                iterB->domainatedCount++;
                if (iterB->domainatedCount >= kValue) {
                    if (tmpPointNext == iterB) // if two nearby nodes, we delete the second, then update first node's next.
                        tmpPointNext = iterB->next;
                    DeletePoint(StwhSize - iterCountB, &StwhHead, &StwhSize, &StwhTail);
                    PushPoint(iterB, &SesSize, &SesTail);
                    iterCountB--;
                }
            }
            iterB = tmpPoint;
        }
        iterA = tmpPointNext;
    }
    /////////////////////////////////////////////////////////////////////////////////////

    elog(INFO, "Step4 over!!");
    
    //[STEP 5] (Stwh, Ses) -> Sg

    // Stwh VS Ses
    iterCount = 0;
    iterA = Stwh->next;
    while (iterA != NULL) {
        iterCount++;
        tmpPointNext = iterA->next;
        iterB = Ses->next;
        while (iterB != NULL) {
            if (isPoint1DominatePoint2(iterB, iterA)) {
                iterA->domainatedCount++;
                if (iterA->domainatedCount >= kValue) {
                    DeletePoint(iterCount, &StwhHead, &StwhSize, &StwhTail);
                    iterCount--;
                    break;
                }
            }
            iterB = iterB->next;
        }
        iterA = tmpPointNext;
    }

    //////////////////////////////////////////////////////////////////////////////////////

    // Stwh VS Sln
    iterCount = 0;
    iterA = Stwh->next;
    while (iterA != NULL) {
        iterCount++;
        tmpPointNext = iterA->next;
        tmpBucket = bucket;
        for (i = 0; i < bucketCount; i++) {
            iterB = tmpBucket->Sln->next;
            while (iterB != NULL) {
                if (isPoint1DominatePoint2(iterB, iterA)) {
                    iterA->domainatedCount++;
                    if (iterA->domainatedCount >= kValue) {
                        DeletePoint(iterCount, &StwhHead, &StwhSize, &StwhTail);
                        iterCount--;
                        break;
                    }
                }
                iterB = iterB->next;
            }
            if (iterB != NULL) break;
            tmpBucket = tmpBucket->next;
		}
		iterA = tmpPointNext;
	}

    SgSize = StwhSize;
    SgHead = StwhHead;
    SgTail = StwhTail;
    Sg = Stwh;
}

Datum skyline_in(PG_FUNCTION_ARGS) {
    char *command;
    int ret;
    int i, j;   
    int bitValid;

    FuncCallContext     *funcctx;
    int                  call_cntr;
    int                  max_calls;
    TupleDesc            tupdesc;
    AttInMetadata       *attinmeta;

    /* stuff done only on the first call of the function */
    if (SRF_IS_FIRSTCALL()) {
        MemoryContext   oldcontext;

        /* create a function context for cross-call persistence */
        funcctx = SRF_FIRSTCALL_INIT();

        /* switch to memory context appropriate for multiple function calls */
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

	/////////////////////////////////////////////////////////////////////////////
	// user code here
	/////////////////////////////////////////////////////////////////////////////
 
        two[0] = 0;
        two[1] = 1;
        for (i = 2; i < 29; i++) {
            two[i] = two[i-1] * 2;
	    elog(INFO, "%d ", two[i]);
        }
	elog(INFO, "\n");

        /* get arguments, convert given text object to a C string */
        command = text_to_cstring(PG_GETARG_TEXT_P(0));
        kValue = PG_GETARG_INT32(1);

        /* open internal connection */
        SPI_connect();

        /* run the SQL scommand, 0 for no limit of returned row number */
        ret = SPI_exec(command, 0);

        /* save the number of rows */
        dataCount = SPI_processed;

        /* If some rows were fetched, print them via elog(INFO). */
        if (ret > 0 && SPI_tuptable != NULL) {
	    TupleDesc tupdesc;
	    SPITupleTable *tuptable;
	    HeapTuple tuple;
            char buf[8192];

            tupdesc = SPI_tuptable->tupdesc;
            tuptable = SPI_tuptable;

	    dataDimension = tupdesc->natts; // attribute number
    
    	    S = StartPoint(S, &SSize, &SHead, &STail, dataDimension);
    	    tmpIntStar = (int32 **)palloc(sizeof(int32*) * dataCount);

            for (i = 0; i < dataCount; i++) {
    	        tmpInput = StartPoint(tmpInput, &tmpSize, &tmpHead, &tmpTail, dataDimension);
    	        tmpIntStar[i] = (int32 *)palloc(sizeof(int32) * dataDimension);
    	        tmpInput->data = &(tmpIntStar[i]);
                tuple = tuptable->vals[i];

                for (j = 1, buf[0] = 0; j <= tupdesc->natts; j++) {
		    if (SPI_getvalue(tuple, tupdesc, j) == NULL) 
		        *(*(tmpInput->data) + j - 1) = 0;
		    else 
		        *(*(tmpInput->data) + j - 1) = atoi(SPI_getvalue(tuple, tupdesc, j)); 
                        snprintf(buf + strlen (buf), sizeof(buf) - strlen(buf), " %s%s",
                            SPI_getvalue(tuple, tupdesc, j), (j == tupdesc->natts) ? " " : " |");
	        }
                elog(INFO, "Data Info: %s", buf);

                for (j = 1, buf[0] = 0; j <= tupdesc->natts; j++) {
		    bitValid = atoi((SPI_getvalue(tuple, tupdesc, j) == NULL) ? "0" : "1");
    	            if (bitValid != 1)  {
    	                tmpInput->bitmap <<= 1;
    	            } else {
    	                tmpInput->bitmap <<= 1;
    	                tmpInput->bitmap |= 1;
    	            }
                    snprintf(buf + strlen (buf), sizeof(buf) - strlen(buf), " %s%s",
                            (SPI_getvalue(tuple, tupdesc, j) == NULL) ? "0" : "1", (j == tupdesc->natts) ? " " : " |");
    	        }
                elog(INFO, "Bitmap   : %s", buf);
	
    	    	PushPoint(tmpInput, &SSize, &STail);        // Push point to S       
	    }

    	}

        elog(INFO, "Input over!!");

    	SPI_finish();

        elog(INFO, "SPI over!!");

    	pfree(command);
        elog(INFO, "Cmd Free over!!");

    	kValue = 1;
    	thicknessWarehouse(dataDimension, kValue);

        elog(INFO, "TWH over!!");

	/////////////////////////////////////////////////////////////////////////////
	// end of user code 
	/////////////////////////////////////////////////////////////////////////////

        /* total number of tuples to be returned */
        funcctx->max_calls = SgSize - 1;
	retPoint = Sg->next;

        /* Build a tuple descriptor for our result type */
        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
            ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED), errmsg("function returning record called in context "
                            "that cannot accept type record")));

        /* Generate attribute metadata needed later to produce tuples from raw C strings */
        attinmeta = TupleDescGetAttInMetadata(tupdesc);
        funcctx->attinmeta = attinmeta;

        MemoryContextSwitchTo(oldcontext);

        elog(INFO, "First SRF over!!");
    }

    /* stuff done on every call of the function */
    funcctx = SRF_PERCALL_SETUP();

    call_cntr = funcctx->call_cntr;
    max_calls = funcctx->max_calls;
    attinmeta = funcctx->attinmeta;
    
    elog(INFO, "SRF begin!!");

    if (call_cntr < max_calls) {   /* do when there is more left to send */
        char       **values;
        HeapTuple    tuple;
        Datum        result;

        elog(INFO, "SRF ~~~~!!");
        /*
         * Prepare a values array for building the returned tuple.
         * This should be an array of C strings which will
         * be processed later by the type input functions.
         */
        values = (char **) palloc(dataDimension * sizeof(char *));

	elog(INFO, "SRF palloc values over ~~~~!!");

	for (i = 0; i < dataDimension; i++) {
	    values[i] = (char *) palloc(16 * sizeof(char));	
	}
        //values[0] = (char *) palloc(16 * sizeof(char));
        //values[1] = (char *) palloc(16 * sizeof(char));
        //values[2] = (char *) palloc(16 * sizeof(char));
        
	elog(INFO, "SRF palloc each values over ~~~~!!");
	
	for (i = 0; i < dataDimension; i++) {
	    if (!(retPoint->bitmap & two[dataDimension - i])) 
	        values[i] = NULL;
	    else 
                snprintf(values[i], 16, "%d", *(*(retPoint->data) + i));
	}
        //snprintf(values[0], 16, "%d", *(*(retPoint->data) + 0));
        //snprintf(values[1], 16, "%d", *(*(retPoint->data) + 1));
        //snprintf(values[2], 16, "%d", *(*(retPoint->data) + 2));
	retPoint = retPoint->next;

	elog(INFO, "SRF set value over ~~~~!!");

        /* build a tuple */
        tuple = BuildTupleFromCStrings(attinmeta, values);

	elog(INFO, "SRF buil tuple over ~~~~!!");

        /* make the tuple into a datum */
        result = HeapTupleGetDatum(tuple);

	elog(INFO, "SRF to datum over ~~~~!!");

        SRF_RETURN_NEXT(funcctx, result);
    } else {   /* do when there is no more left */
        SRF_RETURN_DONE(funcctx);
    }
}
