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
 *  -matrix_t *knn_classify(struct KNN_Pair **, int, int, matrix_t *, int)
 *  -struct KNN_Pair **KNN_Pair_create_empty_table(int, int)
 *  -struct KNN_Pair **KNN_Pair_create_subtable_ref(struct KNN_Pair **, int, int)
 *  -void KNN_Pair_destroy_subtable_ref(struct KNN_Pair **)
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

matrix_t *knn_labeling(struct KNN_Pair **knns, int points, int k,
                       matrix_t *previous, int *cur_indexes,
                       matrix_t *labels, int i_offset);

/**
 *
 */
struct KNN_Pair **KNN_Pair_create_empty_table(int points, int k);

/**
 *
 */
struct KNN_Pair **KNN_Pair_create_subtable_ref(struct KNN_Pair **knns,
                                               int points, int col_reduce);

/**
 *
 */
void KNN_Pair_destroy_subtable_ref(struct KNN_Pair **knns_ref);

/**
 * An ascending comparator for struct KNN_Pair object, based on distance
 * field of each one.
 *
 * It is intended for usage in functions like qsort().
 */
int KNN_Pair_asc_comp(const void * a, const void *b);

int KNN_Pair_asc_comp_by_index(const void * a, const void *b);

#endif
