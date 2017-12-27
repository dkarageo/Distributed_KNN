/**
 * knn.c
 *
 * Created by Dimitrios Karageorgiou,
 *  for course "Parallel And Distributed Systems".
 *  Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
 *
 * knn.c provides an implementation for routines defined in knn.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "knn.h"


struct KNN_Pair **knn_search(matrix_t *data, matrix_t *points,
                             int k, int i_offset)
{
    if (!data || !points || k < 1) {
        printf("ERROR: knn_search() : Invalid Arguments.\n");
        return NULL;
    }
    if (matrix_get_cols(data) != matrix_get_cols(points)) {
        printf("ERROR: knn_search() : k on data=%d, k on points=%d\n",
               matrix_get_cols(data), matrix_get_cols(points));
        return NULL;
    }

    // Get the number of points needed to query their k nearest neighbors.
    int pointc = matrix_get_rows(points);

    // Allocate a new struct KNN_Data, with its fields able to hold
    // pointc * k objects.
    struct KNN_Pair **results = KNN_Pair_create_empty_table(pointc, k);
    if (!results) {
        printf("ERROR: knn_search() : Failed to create results table.\n");
        return NULL;
    }

    // For every point in points matrix, find its kNNs in data matrix and
    // store them into results.
    #pragma omp parallel for
    for (int p = 0; p < pointc; p++) {

        // Calculate the k nearest neighbors for the current point, by
        // searching on all the available data.
        for (int d = 0; d < matrix_get_rows(data); d++) {

            // Calculate the euclidian distance between a queried point and
            // a data point.
            double dist = 0.0;
            for (int i = 0; i < matrix_get_cols(points); i++) {
                dist += pow(matrix_get_cell(points, p, i) -
                            matrix_get_cell(data, d, i), 2.0);
            }
            dist = pow(dist, 0.5);

            // On first k data, just fill the k positions in results array.
            if (d < k) {
                results[p][d].distance = dist;
                results[p][d].index = i_offset + d;

                // When last position in results array gets filled, sort data.
                if (d == k-1) {
                    qsort(results[p], k, sizeof(struct KNN_Pair),
                          KNN_Pair_asc_comp);
                }
            }
            // Every row in results is initialized and sorted. So if
            // current distance is lesser than the distance of the last nearest
            // neighbor, previous value is replaced by current one.
            else if (dist < results[p][k-1].distance) {
                results[p][k-1].distance = dist;
                // Keep track on the index. i_offset is used as the base for
                // all indexes.
                results[p][k-1].index = i_offset + d;
                qsort(results[p], k, sizeof(struct KNN_Pair), KNN_Pair_asc_comp);
            }
        }
    }

    return results;
}

matrix_t *knn_labeling(struct KNN_Pair **knns, int points, int k,
                       matrix_t *previous, int *cur_indexes,
                       matrix_t *labels, int i_offset)
{
    // If no indexes tracker is given, then initialize one to zero.
    int no_index_tracker = 0;
    if (cur_indexes == NULL) {
        cur_indexes = (int *) malloc(points * sizeof(int));
        if (cur_indexes == NULL) {
            printf("ERROR: knn_labeling: Failed to allocate memory.\n");
            return NULL;
        }
        for (int i = 0; i < points; i++) cur_indexes[i] = -1;
        no_index_tracker = 1;
    }

    // If there is a previous incomplete labels matrix, continue filling
    // this one.
    matrix_t *labeled_dists = previous;
    if (labeled_dists == NULL) labeled_dists = matrix_create(points, k);
    if (labeled_dists == NULL) {
        printf("ERROR: knn_labeling: Failed to create matrix.\n");
        return NULL;
    }

    // Get the labels for all nearest neighbors in knns.
    for (int p = 0; p < points; p++) {
        if (cur_indexes[p] == -1) cur_indexes[p] = k-1;

        // For each point, start from the neighbor pointed by cur_indexes.
        int i = cur_indexes[p];
        for (; i >= 0; i--) {
            // When cur_indexes is used, each row in knns is assumed to be sorted
            // by index field. So if one index gets ouf of range, no next pair
            // can be classified using the provided labels. This edge value is
            // the provided offset plus the number of labels in labels matrix.
            // Above this, nothing is valid.
            if (knns[p][i].index < i_offset) break;

            // Subtract the i_offset value for every index, so it matches
            // the actual row index in labels matrix.
            int index = knns[p][i].index - i_offset;
            if (index >= 0 && index < matrix_get_rows(labels)) {
                matrix_set_cell(labeled_dists, p, i,
                                matrix_get_cell(labels, index, 0));
            }
        }

        cur_indexes[p] = i;
    }

    if (no_index_tracker) free(cur_indexes);

    return labeled_dists;
}

matrix_t *knn_classify(matrix_t *labeled_knns)
{

    matrix_t *labeled_points = matrix_create(matrix_get_rows(labeled_knns), 1);

    // Find the greatest label value, so to hash using the value of every label
    // itself.
    // --- ASSUME ALL NON-NEGATIVE AND INTEGERS (JUST CODED INTO DOUBLES) ---
    // --- MAY IMPLEMENT IT BETTER LATER, IF NEEDED ---
    // --- A PROPER HASHTABLE IMPLEMENTATION IS NEEDED ---
    double max = matrix_get_cell(labeled_knns, 0, 0);

    for (int i = 0; i < matrix_get_rows(labeled_knns); i++) {
        for (int j = 0; j < matrix_get_cols(labeled_knns); j++) {
            double cur = matrix_get_cell(labeled_knns, i, j);
            if (cur > max) max = cur;
        }
    }

    // Find the most frequent label for each point's neigbors.
    int int_max = (int) max;

    int *label_counter = (int *) malloc(sizeof(int) * int_max);
    for (int p = 0; p < matrix_get_rows(labeled_knns); p++) {
        // Initialize the labels counter for each point.
        for (int i = 0; i < (int) max; i++) label_counter[i] = 0;

        // Count the occurences of each label in a point's neigbors.
        for (int i = 0; i < matrix_get_cols(labeled_knns); i++) {
            double label = matrix_get_cell(labeled_knns, p, i);
            label_counter[(int) label - 1]++;
        }

        // Find the most frequent label.
        int max_ocur = 0;
        int frequency = label_counter[0];
        for (int i = 0; i < (int) max; i++) {
            if (label_counter[i] > frequency) {
                max_ocur = i;
                frequency = label_counter[i];
            }
        }

        matrix_set_cell(labeled_points, p, 0, (double) max_ocur + 1);
    }

    return labeled_points;
}

struct KNN_Pair **KNN_Pair_create_empty_table(int points, int k)
{
    struct KNN_Pair **obj = (struct KNN_Pair **) malloc(
            sizeof(struct KNN_Pair *) * points);
    if (!obj) return NULL;

    for (int i = 0; i < points; i++) {
        obj[i] = (struct KNN_Pair *) malloc(sizeof(struct KNN_Pair) * k);
        if (!obj[i]) return NULL;

        // Initialize the value of pairs.
        for (int j = 0; j < k; j++) {
            obj[i][j].distance = 0;
            obj[i][j].index = -1;
        }
    }

    return obj;
}

struct KNN_Pair **KNN_Pair_create_subtable_ref(struct KNN_Pair **knns,
                                               int points, int col_reduce)
{
    struct KNN_Pair **subtable = (struct KNN_Pair **) malloc(
                sizeof(struct KNN_Pair *) * points);

    for (int i = 0; i < points; i++) {
        subtable[i] = knns[i] + col_reduce;
    }

    return subtable;
}

void KNN_Pair_destroy_subtable_ref(struct KNN_Pair **knns_ref)
{
    free(knns_ref);
}

int KNN_Pair_asc_comp(const void * a, const void *b)
{
    double da = ((struct KNN_Pair *) a)->distance;
    double db = ((struct KNN_Pair *) b)->distance;
    if (da > db) return 1;
    else if (da < db) return -1;
    else return 0;
}

int KNN_Pair_asc_comp_by_index(const void * a, const void *b)
{
    return ((struct KNN_Pair *) a)->index - ((struct KNN_Pair *) b)->index;
}
