/*-------------------------------------------------
 *
 *  skyband.c
 *  KeySkyBandQuery
 *
 *  Created by Armour on 7/10/15.
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *-------------------------------------------------
 */

#include "postgres.h"

#include <stdlib.h>

#include "fmgr.h"
#include "funcapi.h"
#include "executor/spi.h"
#include "utils/builtins.h"

#include "sky_point_list.h"
#include "sky_bucket_list.h"
#include "sky_domi_list.h"
#include "sky_hashtable.h"


PG_MODULE_MAGIC;

/* Use version 1 */
PG_FUNCTION_INFO_V1(SkybandQuery);

/* Function declaration */
Datum SkybandQuery(PG_FUNCTION_ARGS);
bool IsP1DominateP2(SkyPoint *p1, SkyPoint *p2);
int CmpFunc(const void *p1, const void *p2);
void QsortStwh(int n);
void ThicknessWarehouse(void);
void Init(void);


int sky_k;                  /* The k value that in skyband query */
int sky_dim;                /* The dimension of point in skyband query */
int sky_cnt;                /* The number of the points in skyband query */

double **tmp_pointer;

int s_size;                     /* Point number in S */
int stwh_size;                  /* Point number in Stwh */
int ses_size;                   /* Point number in Ses */
int sg_size;                    /* Point number in Sg */
int tmp_size;

SkyPoint *s_head, *s_tail;              /* Head/tail point for S */
SkyPoint *stwh_head, *stwh_tail;        /* Head/tail point for Stwh */
SkyPoint *ses_head, *ses_tail;          /* Head/tail point for Ses */
SkyPoint *sg_head, *sg_tail;            /* Head/tail point for Sg */
SkyPoint *tmp_head, *tmp_tail;

SkyBucket *first_bucket, *last_bucket;          /* First and last bucket for all bitmap that already knows */
SkyBucket *tmp_bucket;

HashTable *hash_table;                          /* Hashtable for skyband query */
ListNode *tmp_listnode;

DomiTable *domi_table;                          /* Dominate table for storing the index of doninated points */

typedef struct ret_struct {
    SkyPoint **point;
    double **data_pointer;
} ret_struct;

/*
 * Function: IsP1DominateP2
 * -------------------
 *   To judge weather point p2 is dominated by point p1
 *
 *   p1: The point used to compare with p2
 *   p2: The point used to compare with p1
 *
 *   returns:
 *      if p1 dominate p2 return 1
 *                   else return 0
 */

bool IsP1DominateP2(SkyPoint *p1, SkyPoint *p2) {
    double x1, x2;
    int i;
    int dim = p1->dim;
    int is_small = 0;                       /* Used to decide if there exist one dimension that p1 is smaller than p2 */
    int cnt_small_or_equal = 0;             /* Count of dimension number that p1 is smaller than or equal to p2 in such dimension */
    int is_null_x1, is_null_x2;
    if (!p1 || !p2) return 0;               /* If p1 or p2 is not exist */
    for (i = 0; i < dim; i++) {             /* Look at each dimension */
        x1 = *(*(p1->data)+i);
        x2 = *(*(p2->data)+i);
        is_null_x1 = (*(p1->bitmap + i)) == '0';
        is_null_x2 = (*(p2->bitmap + i)) == '0';
        if (is_null_x1 || is_null_x2) {         /* If p1 or p2's bitmap is '0', which means incomplete data */
            cnt_small_or_equal++;
        } else {                                /* Else just do comparation */
            if (x1 <= x2) cnt_small_or_equal++;
            if (x1 < x2) is_small = 1;
        }
    }
    if ((cnt_small_or_equal == dim) && is_small)        /* All dimension not larger than p2 and exist one dimension is smaller than p2 */
        return 1;
    else
        return 0;
}

/*
 * Function: CmpFunc
 * -------------------
 *   Compare function that used in QsortStwh function
 *
 *   p1: The point used to compare with p2
 *   p2: The point used to compare with p1
 *
 *   returns:
 *      if p1's cnt_domi is smaller than p2's, return > 0
 *      if p2's cnt_domi is smaller than p1's, return < 0
 *                                        else return = 0
 */

int CmpFunc(const void *p1, const void *p2) {
    const SkyPoint **t1 = (const SkyPoint **)p1;
    const SkyPoint **t2 = (const SkyPoint **)p2;
    return (**t2).cnt_domi - (**t1).cnt_domi;
}

/*
 * Function: QsortStwh
 * -------------------
 *   Qsort on Stwh list, first we make a copy of list as an array,
 *   after we finished the qsort on array, we'll put it back to list
 *
 *   n: The size of Stwh list (array)
 *
 *   returns: void
 */

void QsortStwh(int n) {
    int i;
    SkyPoint *point_array[n];                       /* Dynamically alloc memory for an array with size n */
    SkyPoint *tmp_point;
    if (sky_k > 1) {                                /* Only need qsort when sky_k is larger than 1 */
        tmp_point = stwh_head;
        for (i = 0; i < n; i++) {                   /* Build the array */
            point_array[i] = tmp_point->next;
            tmp_point = tmp_point->next;
        }
        qsort(point_array, n, sizeof(point_array[0]), CmpFunc);         /* Qsort this array */
        stwh_head->next = point_array[0];           /* Put first point in array back to list */
        point_array[0]->prev = stwh_head;
        point_array[0]->next = NULL;
        stwh_tail = point_array[0];
        for (i = 1; i < n; i++) {                   /* Put rest points in array back to list */
            stwh_tail->next = point_array[i];
            point_array[i]->prev = stwh_tail;
            point_array[i]->next = NULL;
            stwh_tail = point_array[i];
        }
    }
}

/*
 * Function: ThicknessWarehouse
 * -------------------
 *   There are five steps:
 *
 *   step 1: push all points in S to every bucket according to bitmap
 *   step 2: divide points in every bucket into Sl and Sln, then put all points in Sl into Stwh.
 *   step 3: quick sort all points in Stwh according to cnt_domi
 *   step 4: comparing all points in Swth and push expired points to Ses
 *   step 5: comparing points in Swth with Ses and Sln, get the final result Sg.
 *
 *   if you have some thing do not understand, you can take a look at Mr.Gao's paper
 *
 *   returns: void
 */

void ThicknessWarehouse(void) {
    int i, j;
    int cnt_a, cnt_b;

    SkyPoint *iter_a;
    SkyPoint *iter_b;
    SkyPoint *tmp_point = NULL;
    SkyPoint *tmp_point2 = NULL;
    SkyPoint *tmp_next;
    SkyPoint **tmp_array;

    /* Create start point of Stwh, Ses and Sg */
    stwh_head = StartPoint(&stwh_size, &stwh_head, &stwh_tail, sky_dim);
    ses_head = StartPoint(&ses_size, &ses_head, &ses_tail, sky_dim);
    sg_head = StartPoint(&sg_size, &sg_head, &sg_tail, sky_dim);

    //////////////////////////////////////////////////////////////////////////
    //                                                                      //
    // [STEP 1]                                                             //
    //      Push all points in S to every bucket according to bitmap        //
    //                                                                      //
    //////////////////////////////////////////////////////////////////////////

    /* Create hashtable */
    hash_table = InitHashTable(sky_cnt);

    /* Push every point in S to a bucket depends on its bitmap */
    first_bucket = NULL;
    tmp_point = s_head;
    tmp_next = tmp_point->next;
    while (tmp_next != NULL) {
        tmp_point = tmp_next;
        tmp_next = tmp_point->next;
        tmp_listnode = Find(tmp_point->bitmap, hash_table, sky_dim);             /* Find the list of nodes in hashtable according to bimap */
        if (tmp_listnode == NULL) {
            tmp_bucket = (SkyBucket *)palloc(sizeof(SkyBucket));        /* If not exist, then we create a node for this bitmap in hashtable */
            InitBucket(tmp_bucket, sky_dim);
            Insert(tmp_point->bitmap, hash_table, sky_dim, tmp_bucket, &first_bucket, &last_bucket);
        } else {
            tmp_bucket = tmp_listnode->bucket;                          /* Get the bucket of this bitmap */
        }
        PushPoint(tmp_point, &tmp_bucket->data_size, &tmp_bucket->data_tail);       /* Push point into the bucket of this bitmap */
    }


    //////////////////////////////////////////////////////////////////////////
    //                                                                      //
    // [STEP 2]                                                             //
    //      Divide points in every bucket into Sl and Sln, then put all     //
    //  points in Sl into Stwh.                                             //
    //                                                                      //
    //////////////////////////////////////////////////////////////////////////

    tmp_bucket = first_bucket;
    while (tmp_bucket != NULL) {
        tmp_array = (SkyPoint **)palloc(sizeof(SkyPoint*) * tmp_bucket->data_size);
        tmp_point = tmp_bucket->data_head;
        for (i = 0; i < tmp_bucket->data_size; i++) {                           /* Put points in list into array */
            tmp_point = tmp_point->next;
            tmp_array[i] = tmp_point;
        }
        for (i = 0; i < tmp_bucket->data_size; i++) {
            tmp_point = tmp_array[i];
            for (j = 0; j < tmp_bucket->data_size; j++) {                       /* Compare each pair of points in array */
                if (i != j) {
                    tmp_point2 = tmp_array[j];
                    if (IsP1DominateP2(tmp_point2, tmp_point)) {                /* If point A dominate point B */
                        tmp_point->cnt_domi++;                                  /* Add cnt_domi of B */
                        AddDomiPair(domi_table, tmp_point->index, tmp_point2->index);       /* Add pair to dominate table */
                        if (tmp_point->cnt_domi>= sky_k) {                      /* If cnt_domi of B is larger than sky_k, we put B into Sln */
                            PushPoint(tmp_point, &tmp_bucket->sln_size, &tmp_bucket->sln_tail);
                            break;
                        }
                    }
                }
            }
            if (j == tmp_bucket->data_size)                 /* which means data[j] is not dominted more than k times, then put it into Sl */
                PushPoint(tmp_point, &stwh_size, &stwh_tail);
        }
        tmp_bucket = tmp_bucket->next;

        pfree(tmp_array);                                   /* Free the memory of tmp_array */
    }

    //////////////////////////////////////////////////////////////////////////
    //                                                                      //
    // [STEP 3]                                                             //
    //      Quick sort all points in Stwh according to cnt_domi             //
    //                                                                      //
    //////////////////////////////////////////////////////////////////////////

    QsortStwh(stwh_size);

    //////////////////////////////////////////////////////////////////////////
    //                                                                      //
    // [STEP 4]                                                             //
    //      Comparing all points in Swth and push expired points to Ses     //
    //                                                                      //
    //////////////////////////////////////////////////////////////////////////

    cnt_a = 0;
    iter_a = stwh_head->next;
    while (iter_a != NULL) {                                                        /* Iter_a starts from stwh_head to stwh_tail */
        cnt_a++;
        tmp_next = iter_a->next;
        iter_b = stwh_tail;
        cnt_b = 0;
        while (iter_b != stwh_head) {                                               /* Iter_b starts from stwh_tali to stwh_head */
            cnt_b++;
            tmp_point = iter_b->prev;
            if (iter_a->index == iter_b->index) {                                   /* If converge at same index, then break */
                 break;
            }
            if (SameBitmap(iter_a->bitmap, iter_b->bitmap, sky_dim)) {              /* If in the same bitmap, then continue */
                iter_b = tmp_point;
                continue;
            }
            if (IsP1DominateP2(iter_b, iter_a)) {
                iter_a->cnt_domi++;
                AddDomiPair(domi_table, iter_a->index, iter_b->index);              /* Add pair to dominate table */
                if (iter_a->cnt_domi >= sky_k) {
                    DeletePoint(cnt_a, &stwh_size, &stwh_head, &stwh_tail);
                    PushPoint(iter_a, &ses_size, &ses_tail);
                    cnt_a--;
                    break;
                }
            }
            if (IsP1DominateP2(iter_a, iter_b)) {
                iter_b->cnt_domi++;
                AddDomiPair(domi_table, iter_b->index, iter_a->index);              /* Add pair to dominate table */
                if (iter_b->cnt_domi >= sky_k) {
                    if (tmp_next == iter_b)                 /* If two nearby nodes, we delete the second, then update first node's next */
                        tmp_next = iter_b->next;
                    DeletePoint(stwh_size - cnt_b + 1, &stwh_size, &stwh_head, &stwh_tail);
                    PushPoint(iter_b, &ses_size, &ses_tail);
                    cnt_b--;
                }
            }
            iter_b = tmp_point;
        }
        iter_a = tmp_next;
    }

    //////////////////////////////////////////////////////////////////////////
    //                                                                      //
    // [STEP 5]                                                             //
    //      Comparing points in Swth with Ses and Sln, get the final        //
    //  result Sg.                                                          //
    //                                                                      //
    //////////////////////////////////////////////////////////////////////////

    /* Stwh VS Ses */
    cnt_a = 0;
    iter_a = stwh_head->next;
    while (iter_a != NULL) {
        cnt_a++;
        tmp_next = iter_a->next;
        iter_b = ses_head->next;
        while (iter_b != NULL) {
            if (IsP1DominateP2(iter_b, iter_a) && !IsDomiPairExist(domi_table, iter_a->index, iter_b->index)) {
                iter_a->cnt_domi++;
                AddDomiPair(domi_table, iter_a->index, iter_b->index);              /* Add pair to dominate table */
                if (iter_a->cnt_domi >= sky_k) {
                    DeletePoint(cnt_a, &stwh_size, &stwh_head, &stwh_tail);
                    cnt_a--;
                    break;
                }
            }
            iter_b = iter_b->next;
        }
        iter_a = tmp_next;
    }

    /* Stwh VS Sln */
    cnt_a = 0;
    iter_a = stwh_head->next;
    while (iter_a != NULL) {
        cnt_a++;
        tmp_next = iter_a->next;
        tmp_bucket = first_bucket;
        while (tmp_bucket != NULL) {
            iter_b = tmp_bucket->sln_head->next;
            while (iter_b != NULL) {
                if (IsP1DominateP2(iter_b, iter_a)) {
                    iter_a->cnt_domi++;
                    AddDomiPair(domi_table, iter_a->index, iter_b->index);              /* Add pair to dominate table */
                    if (iter_a->cnt_domi >= sky_k) {
                        DeletePoint(cnt_a, &stwh_size, &stwh_head, &stwh_tail);
                        cnt_a--;
                        break;
                    }
                }
                iter_b = iter_b->next;
            }
            if (iter_b != NULL) break;
            tmp_bucket = tmp_bucket->next;
        }
        iter_a = tmp_next;
    }

    /* Sg is equal to Stwh now, and it is the answer of the query */
    sg_size = stwh_size;
    sg_head = stwh_head;
    sg_tail = stwh_tail;
}

/*
 * Function: Init
 * -------------------
 *   Initialize varibles
 *
 *   returns: void
 */

void Init(void) {
    tmp_size = 0;
    s_size = 0;
    stwh_size = 0;
    ses_size = 0;
    sg_size = 0;
    first_bucket = NULL;
    last_bucket = NULL;
}

/*
 * Function: SkybandQuery
 * ------------------------
 *     Main function
 *
 *     returns: Datum
 */

Datum SkybandQuery(PG_FUNCTION_ARGS) {
    char *command;
    int ret;
    int i, j;
    int call_cntr;
    int max_calls;
    SkyPoint *tmp_point;
    ret_struct *ret_points;

    FuncCallContext *funcctx;
    AttInMetadata *attinmeta;
    TupleDesc tupdesc;

    /* Initialize varible */
    Init();

    /* stuff done only on the first call of the function */
    if (SRF_IS_FIRSTCALL()) {
        MemoryContext oldcontext;

        /* create a function context for cross-call persistence */
        funcctx = SRF_FIRSTCALL_INIT();

        /* switch to memory context appropriate for multiple function calls */
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        /* get arguments, convert given text object to a C string */
        command = text_to_cstring(PG_GETARG_TEXT_P(0));
        if (!PG_ARGISNULL(1)) {
            sky_k = PG_GETARG_INT32(1);
        } else {
            sky_k = 1;
        }
        if (sky_k < 1)
            sky_k = 1;

        /* open internal connection */
        SPI_connect();

        /* run the SQL scommand, 0 for no limit of returned row number */
        ret = SPI_exec(command, 0);

        /* save the number of rows */
        sky_cnt = SPI_processed;

        /* Initailize dominate lists */
        domi_table = InitDomiTable(sky_cnt);

        /* If some rows were fetched, print them via elog(INFO). */
        if (ret > 0 && SPI_tuptable != NULL) {
            TupleDesc tupdesc;
            SPITupleTable *tuptable;
            HeapTuple tuple;
            int cnt_dim = 0;
            char *type_name;

            tupdesc = SPI_tuptable->tupdesc;
            tuptable = SPI_tuptable;

            /* get the number of datatype that can be used in skyband query */
            for (i = 1; i <= tupdesc->natts; ++i) {
                type_name = SPI_gettype(tupdesc, i);
                if (strcmp(type_name, "int4") == 0 || strcmp(type_name, "int2") == 0 ||
                    strcmp(type_name, "float4") == 0 || strcmp(type_name, "float8") == 0) {
                    //elog(INFO, "Type: %s", type_name);
                    cnt_dim++;
                }
            }

            sky_dim = cnt_dim;                                                  /* dimension */

            s_head = StartPoint(&s_size, &s_head, &s_tail, sky_dim);            /* Create head point of S */

            tmp_pointer = (double **)palloc(sizeof(double *) * sky_cnt);

            for (i = 0; i < sky_cnt; i++) {
                int cnt_dim = 0;
                tmp_head = StartPoint(&tmp_size, &tmp_head, &tmp_tail, sky_dim);      /* Create temp input point */
                tmp_pointer[i] = (double *)palloc(sizeof(double) * sky_dim);          /* Palloc memory to input point's data */
                tmp_head->data = &(tmp_pointer[i]);
                tmp_head->bitmap = (char *)palloc(sizeof(char) * sky_dim);            /* Palloc memory to input point's bimap */
                tmp_head->index = i;

                tuple = tuptable->vals[i];

                for (j = 1; j <= tupdesc->natts; j++) {                     /* Input point's data from tuple */
                    type_name = SPI_gettype(tupdesc, j);
                    if (strcmp(type_name, "int4") == 0 || strcmp(type_name, "int2") == 0 ||
                        strcmp(type_name, "float4") == 0 || strcmp(type_name, "float8") == 0) {
                        if (SPI_getvalue(tuple, tupdesc, j) == NULL) {                      /* If data is NULL */
                            *(*(tmp_head->data) + cnt_dim) = 0;
                            *(tmp_head->bitmap + cnt_dim) = '0';
                        } else {                                                            /* If data is not NULL */
                            *(*(tmp_head->data) + cnt_dim) = atof(SPI_getvalue(tuple, tupdesc, j));
                            *(tmp_head->bitmap + cnt_dim) = '1';
                        }
                        cnt_dim++;
                    }
                }
                /*elog(INFO, "Data Info: %s", buf);*/

                PushPoint(tmp_head, &s_size, &s_tail);                          /* Push all points to S */
            }
        }

        /*elog(INFO, "Input over!!");*/

        pfree(command);

        /*elog(INFO, "SPI over!!");*/

        ThicknessWarehouse();

        /*elog(INFO, "TWH over!!");*/

        /* total number of tuples to be returned */
        funcctx->max_calls = sg_size;

        /* store all output informatin to funcctx->user_fctx */
        ret_points = (ret_struct *)palloc(sizeof(ret_struct));
        ret_points->point = (SkyPoint **)palloc(sizeof(SkyPoint*) * sg_size);               /* store points */
        ret_points->data_pointer = (double **)palloc(sizeof(double *) * sg_size);           /* store data and then can pass this address to ret_points->point->data */
        tmp_point = sg_head;
        for (i = 0; i < sg_size; ++i) {
            tmp_point = tmp_point->next;
            *((ret_points->point) + i) = (SkyPoint *)palloc(sizeof(SkyPoint));
            (*((ret_points->point) + i))->bitmap = (char *)palloc(sizeof(char) * sky_dim);      /* palloc bitmap memeory */
            ret_points->data_pointer[i] = (double *)palloc(sizeof(double) * sky_dim);
            (*((ret_points->point) + i))->data = &(ret_points->data_pointer[i]);                /* pass data address */
            (*((ret_points->point) + i))->index = tmp_point->index;
            for (j = 0; j < sky_dim; ++j) {
                *(*( (*((ret_points->point) + i))->data ) + j) = *(*(tmp_point->data) + j);     /* store all data information to ret_points */
                *((*((ret_points->point) + i))->bitmap + j) = *(tmp_point->bitmap + j);         /* store all bitmap information to ret_points */
            }
        }
        funcctx->user_fctx = ret_points;                                                /* store all information in ret_points to funcctx->user_fctx */

        /* Build a tuple descriptor for our result type */
        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
            ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED), errmsg("function returning record called in context "
                            "that cannot accept type record")));

        /* Generate attribute metadata needed later to produce tuples from raw C strings */
        attinmeta = TupleDescGetAttInMetadata(tupdesc);
        funcctx->attinmeta = attinmeta;

        /* MemoryContext switch to old context */
        MemoryContextSwitchTo(oldcontext);

        /*elog(INFO, "First SRF over!!");*/
    }

    /* stuff done on every call of the function */
    funcctx = SRF_PERCALL_SETUP();

    call_cntr = funcctx->call_cntr;
    max_calls = funcctx->max_calls;
    attinmeta = funcctx->attinmeta;
    ret_points = funcctx->user_fctx;

    /*elog(INFO, "SRF begin!!");*/

    if (call_cntr < max_calls) {   /* do when there is more left to send */
        char       **values;
        HeapTuple    tuple;
        HeapTuple    ret_tuple;
        Datum        result;
        SkyPoint    *tmp_point;
        TupleDesc   tupdesc;
        SPITupleTable *tuptable;

        tupdesc = SPI_tuptable->tupdesc;
        tuptable = SPI_tuptable;

        /*
         * Prepare a values array for building the returned tuple.
         * This should be an array of C strings which will
         * be processed later by the type input functions.
         */
        values = (char **) palloc(tupdesc->natts * sizeof(char *));

        /*elog(INFO, "SRF palloc values over ~~~~!!");*/

        /* get return tuple value from index */
        tmp_point = (ret_points->point)[call_cntr];
        for (i = 0; i < tupdesc->natts; i++) {
            tuple = tuptable->vals[tmp_point->index];
            values[i] = (SPI_getvalue(tuple, tupdesc, i + 1));
        }

        /*elog(INFO, "SRF get value done ~~~~!!");*/

        /* build a tuple */
        ret_tuple = BuildTupleFromCStrings(attinmeta, values);

        /*elog(INFO, "SRF buil tuple over ~~~~!!");*/

        /* make the tuple into a datum */
        result = HeapTupleGetDatum(ret_tuple);

        /*elog(INFO, "SRF to datum over ~~~~!!");*/

        SRF_RETURN_NEXT(funcctx, result);
    } else {
        SPI_finish();
        SRF_RETURN_DONE(funcctx);    // if all printed
    }
}
