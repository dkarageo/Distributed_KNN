/**
 * knn.h
 *
 * Created by Dimitrios Karageorgiou,
 *  for course "Parallel And Distributed Systems".
 *  Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
 *
 * knn.h defines routines that may be used for k-Nearest-Neighbors searching.
 *
 * Types defined in knn.h:
 *  -struct KNN_Pair
 *
 * Functions defined in knn.h:
 *  -struct KNN_Pair **knn_search(matrix_t *, matrix_t *, int, int)
 *  -matrix_t *knn_classify(matrix_t *labeled_knns)
 *  -matrix_t *knn_labeling(struct KNN_Pair **, int, int, matrix_t *, int *,
 *                          matrix_t *, int)
 *  -struct KNN_Pair **KNN_Pair_create_empty_table(int, int)
 *  -void KNN_Pair_destroy_table(struct KNN_Pair **table, int rows)
 *  -struct KNN_Pair **KNN_Pair_create_subtable(struct KNN_Pair **knns,
 *                                              int row_start, int row_end,
 *                                              int col_start, int col_end)
 *  -int KNN_Pair_asc_comp(const void *, const void *)
 */

#ifndef __knn_h__
#define __knn_h__

#include "matrix.h"


// A struct to keep data resulted by knn_search.
struct KNN_Pair {
    double distance;
    int index;
};

/**
 * Does a k-Nearest-Neighbors search for given points, on provided data.
 *
 * data and points are expected to be matrixes of the same width, i.e. to
 * contain the same cords for each point. Otherwise, it leads to undefined
 * behaviour. Also, k should be greater than the number of rows in data matrix.
 *
 * i_offset argument allows doing knn searching in chunks. The returned index
 * for each of k nearest neighbors, will be the index it has in data matrix(
 * the row counter) plus the i_offset, which will be the base for all returned
 * indexes of this call to knn_search(). This way, there is no need to know
 * the complete data set. The only required thing to know, is the offset of
 * current data chunk from the beggining of the complete data set.
 *
 * Parameters:
 *  -data : A matrix containing all the points, that will be searched for
 *          nearest points for every point in points argument.
 *  -points : The query of points, that is needed to be matched to their kNNs.
 *  -k : The number of nearest neighbors to be returned for each point in
 *          points.
 *  -i_offset : The offset of the beggining of this data chunk, from the
 *          beggining of the complete data set.
 *
 * Returns:
 *  A table of KNN_Pair objects, each one containing the distance of k-th
 *  nearest neighbor from equivalent query point and their original index
 *  in data matrix plus the value of i_offset argument.
 */
struct KNN_Pair **knn_search(matrix_t *data, matrix_t *points,
                             int k, int i_offset);

/**
 * Classifies points based on the given table of k-Nearest-Neigbors and
 * a provided matrix with their labels.
 *
 * The 2D table of KNN_Pair objects, should normally be the result of
 * a call to knn_search() function.
 *
 * The use of i_offset argument allows to classify chunks of points returned
 * by knn_search() function. It should be the same offset provided to
 * knn_search(). When chunk classifying is not needed, it should always be 0.
 *
 * Parameters:
 *  -knns : A 2D table of KNN_Pair objects, as resulted by knn_search(). Each
 *          row is a point to be classified. On each row, k KNN_Pair objects
 *          should be contained, each one containing the distance and label's
 *          index of the k-th nearest neighbor to the point represented by this
 *          row.
 *  -points : The number of points contained in knns, i.e. the number of rows
 *          in knns.
 *  -k: The number of nearest neighbors to be used in classifying. This number
 *          should be equal or lesser than the nearest neighbors contained in
 *          knns for each point.
 *  -labels: A column matrix to look up the labels of kNNs contained in knns.
 *          Labels in this matrix should reside in the same order as defined
 *          by neighbors' indexes. i_offset can be used to control sliding up
 *          or down.
 *  -i_offset : An offset to be subtracted from all the neighbors' indexes in
 *          knns table, that will later be used for lookup in labels.
 *
 * Returns:
 *  A (points x 1) matrix containing the classification of each point in knns.
 *  Each row of returned matrix, correspond to the equivalent point row in knns.
 */
matrix_t *knn_classify(matrix_t *labeled_knns);

/**
 * Labels points in the given table of k-Nearest-Neigbors.
 *
 * The 2D table of KNN_Pair objects, should normally be the result of
 * a call to knn_search() function.
 *
 * The use of i_offset argument allows to label chunks of points returned
 * by knn_search() function. It should be the same offset provided to
 * knn_search(). When chunk classifying is not needed, it should always be 0.
 *
 * Parameters:
 *  -knns : A 2D table of KNN_Pair objects, as resulted by knn_search().
 *          On each row, k KNN_Pair objects
 *          should be contained, each one containing the distance and label's
 *          index of the k-th nearest neighbor to the point represented by this
 *          row.
 *  -points : The number of points contained in knns, i.e. the number of rows
 *          in knns.
 *  -k: The number of nearest neighbors to be labeled. This number
 *          should be equal or lesser than the nearest neighbors contained in
 *          knns for each point.
 *  -previous: A matrix contained all the neighbors that has been labeled so far.
 *          It allows labeling to be done in parts and should normally be
 *          a matrix returned by a previous call to knn_labeling, though with a
 *          different labels matrix. If labels argument provide labels for all
 *          neighbors contained in knns argument, this argument should be NULL.
 *          This argument can also be NULL the first time knn_labeling is called
 *          on subsequent calls to it. Though, the returned matrix should be
 *          used in all sebsequent calls.
 *  -cur_indexes: An helper bookeeping object needed when doing labeling in
 *          parts. If labeling is not to be done in parts, it can be safely
 *          set to NULL. In any other case, it should be an allocated array of
 *          size sizeof(int) * points.
 *  -labels: A column matrix to look up the labels of kNNs contained in knns.
 *          Labels in this matrix should reside in the same order as defined
 *          by neighbors' indexes. i_offset can be used to control sliding up
 *          or down.
 *  -i_offset : An offset to be subtracted from all the neighbors' indexes in
 *          knns table, that will later be used for lookup in labels.
 *
 * Returns:
 *  A (points x k) matrix containing the labels for neighbors contained in
 *  knns 2D array.
 */
matrix_t *knn_labeling(struct KNN_Pair **knns, int points, int k,
                       matrix_t *previous, int *cur_indexes,
                       matrix_t *labels, int i_offset);

/**
 * Creates a new 2D array with KNN_Pair objects.
 *
 * Parameters:
 *  -points: The number of points that should fit in array.
 *  -k: The number of nearest neighbors each point is expected to have.
 *
 * Returns:
 *  The newly created array.
 */
struct KNN_Pair **KNN_Pair_create_empty_table(int points, int k);

/**
 * Destroy the given table of struct KNN_Pair objects.
 *
 * Parameters:
 *  -table: A reference to the table to destroy.
 *  -rows: Number of rows that table has.
 */
void KNN_Pair_destroy_table(struct KNN_Pair **table, int rows);

/**
 * Creates a new table, with the content of a subtable of the given table.
 *
 * If row_start and col_start are set to 0 and row_end and col_end are set
 * to the number of rows given table has minus 1 (since it is 0-indexed and
 * ending values are inclusive), the new table is simply a copy of the given
 * one.
 *
 * Parameters:
 *  -knns: The table from which content will be copied to the new one.
 *  -row_start: The index of row to start copying from (inclusive).
 *  -row_end: The index of row to stop copying at (inclusive).
 *  -col_start: The index of column to start copying from (inclusive).
 *  -col_end: The index of column to stop copying at (inclusive).
 *
 * Returns:
 *  A reference to the newly created table.
 */
struct KNN_Pair **KNN_Pair_create_subtable(struct KNN_Pair **knns,
                                           int row_start, int row_end,
                                           int col_start, int col_end);

/**
 * An ascending comparator for struct KNN_Pair object, based on distance
 * field of each one.
 *
 * It is intended for usage in functions like qsort().
 */
int KNN_Pair_asc_comp(const void * a, const void *b);

/**
 * An ascending comparator for struct KNN_Pair object, based on index
 * field of each one.
 *
 * It is intended for usage in functions like qsort().
 */
int KNN_Pair_asc_comp_by_index(const void * a, const void *b);


// ================ Legacy Code =================

// /**
//  * ----BY ITS NATURE THIS ROUTINE WILL PROBABLY LEAD TO MEMORY LEAKS----
//  * ----SHOULD BE REPLACED IN FUTURE----
//  *
//  * Returns a reference to the subtable of given knns 2D array, that has
//  * col_reduce columns removed from its beggining.
//  *
//  * No new object is allocated. The reference is valid as long as the original
//  * object is valid. Though, it should be destroyed seperately.
//  *
//  * Parameters:
//  *  -knns: A reference to a 2D struct KNN_Pair array.
//  *  -points: The number of points contained in knns.
//  *  -col_reduce: The number of columns to cut from the beggining.
//  *
//  * Returns:
//  *  A reference to a 2D KNN_Pair array, with col_reduce less columns.
//  */
// struct KNN_Pair **KNN_Pair_create_subtable_ref(struct KNN_Pair **knns,
//                                                int points, int col_reduce);
//
// /**
//  * ----DANGEROUS, DANGEROUS, DANGEROUS!!!----
//  * ----SHOULD BE REPLACED IN FUTURE----
//  *
//  * Destroys a given reference inside a struct KNN_Pair 2D array.
//  *
//  * knns_ref: A reference inside a real struct KNN_Pair 2D array. In any case
//  *          it should not be the real table.
//  */
// void KNN_Pair_destroy_subtable_ref(struct KNN_Pair **knns_ref);

#endif
