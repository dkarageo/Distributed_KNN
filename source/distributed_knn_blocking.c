/**
 * distributed_knn_blocking.c
 *
 * Created by Dimitrios Karageorgiou,
 *  for course "Parallel And Distributed Systems".
 *  Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
 *
 * An implementation provided for routines defined in distributed_knn_blocking.h
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include "matrix.h"
#include "knn.h"
#include "distributed_knn_blocking.h"


matrix_t *knn_labeling_distributed(struct KNN_Pair **knns, int points, int k,
                                   matrix_t *local_labels, int prev_task,
                                   int next_task, int tasks_num)
{
    // Sort the neighbors of each point by index.
    for (int i = 0; i < points; i++) {
        qsort(knns[i], k, sizeof(struct KNN_Pair), KNN_Pair_asc_comp_by_index);
    }

    matrix_t *cur_labels = local_labels;
    matrix_t *next_labels = NULL;

    // Initialize a counter for the index where labeling begins for the
    // nearest neighbors of each point.
    int *index_counter = (int *) malloc(points * sizeof(int));
    for (int i = 0; i < points; i++) {
        index_counter[i] = -1;
    }

    matrix_t *labeled = NULL;

    for (int i = 0; i < tasks_num; i++) {
        size_t out_size = 0;
        size_t in_size = 0;
        char *out_object = NULL;
        char *in_object = NULL;

        if (i < tasks_num - 1) {
            out_object = matrix_serialize(cur_labels, &out_size);

            // On even numbered nodes, first send then receive. On odd numbered
            // nodes, do the opposite. This is necessary in order to prevent
            // deadlock, since operations are blocking.
            switch (next_task % 2) {
            case 0:
                // Receive next data from previous process.
                _recv_object(&in_object, &in_size, prev_task);
                // Send current data to next process.
                _send_object(out_object, out_size, next_task);
                break;

            case 1:
                // Send current data to next process.
                _send_object(out_object, out_size, next_task);
                // Receive next data from previous process.
                _recv_object(&in_object, &in_size, prev_task);
                break;
            }
        }

        labeled = knn_labeling(knns, points, k, labeled, index_counter,
                               cur_labels, matrix_get_chunk_offset(cur_labels));

       // On final iterations, no communications exist.
       if (i < tasks_num - 1) {
           // Inflate an actual matrix object. Serialized objects are no longer
           // needed after that.
           next_labels = matrix_deserialize(in_object, in_size);
           free(out_object);
           free(in_object);
       }

       // If current block is not the local labels, it is no more needed.
       if (i > 0) matrix_destroy(cur_labels);
       // Do labeling for next block.
       cur_labels = next_labels;
    }

    return labeled;
}

struct KNN_Pair **knn_search_distributed(matrix_t *local_data, int k,
                                         int prev_task, int next_task,
                                         int tasks_num)
{
    matrix_t *cur_data_block = NULL;   // Current block of data to be used for knn search.
    matrix_t *next_data_block = NULL;  // Next block of data for knn search.
    struct KNN_Pair **knns = NULL; // K Nearest Neighbors for current query chunk.

    cur_data_block = local_data;  // Start knn search using local data.

    // Repeat the process tasks_num times and update kNNs based on the new
    // blocks.
    for (int i = 0; i < tasks_num; i++) {
        size_t out_size = 0;
        size_t in_size = 0;
        char *out_object = NULL;
        char *in_object = NULL;

        // On final iteration, there is nothing more to send/receive. Last data
        // exchange happened on tasks_num - 1 iteration.
        if (i < tasks_num - 1) {

            out_object = matrix_serialize(cur_data_block, &out_size);

            // Resolve the deadlocks in ring topology using blocking routines,
            // by first receiving data on even nodes and first sending data on
            // odd nodes. That approach works for both even and odd total number
            // of nodes, since operations are two (send and receive). Thus,
            // total number of performed operations is always even.
            switch(next_task % 2) {
            case 0:
                // Receive next data from previous process.
                _recv_object(&in_object, &in_size, prev_task);
                // Send current data to next process.
                _send_object(out_object, out_size, next_task);
                break;
            case 1:
                // Send current data to next process.
                _send_object(out_object, out_size, next_task);
                // Receive next data from previous process.
                _recv_object(&in_object, &in_size, prev_task);
                break;
            }
        }

        // On first iteration, search is done using the local data chunk.
        if (i == 0) {
            // First nearest neighbor will be the point itself, so remove it.
            struct KNN_Pair **knn_helper = knn_search(
                    cur_data_block, local_data, k+1,
                    matrix_get_chunk_offset(cur_data_block));
            knns = KNN_Pair_create_subtable(
                    knn_helper, 0, matrix_get_rows(local_data)-1, 1, k);
            KNN_Pair_destroy_table(knn_helper, matrix_get_rows(local_data));
        }
        // On all remaining iterations, search is done using data chunks
        // received from other tasks.
        else {
            struct KNN_Pair **new_knns = knn_search(
                    cur_data_block, local_data, k,
                    matrix_get_chunk_offset(cur_data_block));
            // Merge new and old results.
            _update_knns(knns, new_knns, matrix_get_rows(local_data), k);
        }

        // On final iterations, no communications exist.
        if (i < tasks_num - 1) {
            // Inflate an actual matrix object. Serialized objects are no longer
            // needed after that.
            next_data_block = matrix_deserialize(in_object, in_size);
            free(out_object);
            free(in_object);
        }

        // If current block is not the local data, it is no more needed.
        if (i > 0) matrix_destroy(cur_data_block);
        // Do the search for next block.
        cur_data_block = next_data_block;
    }

    return knns;
}


void _send_object(char *object, size_t length, int rank)
{
    // Send object.
    MPI_Send(object, length, MPI_CHAR, rank, MPI_TAG_OBJECT, MPI_COMM_WORLD);
}


void _recv_object(char **object, size_t *length, int rank)
{
    int size;
    MPI_Status status;

    // Probe for length of incoming object.
    MPI_Probe(rank, MPI_TAG_OBJECT, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_CHAR, &size);
    *length = (size_t) size;

    // Start receiving the object.
    *object = (char *) malloc(sizeof(char) * size);
    MPI_Recv(*object, size, MPI_CHAR, rank, MPI_TAG_OBJECT,
              MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}


void _update_knns(struct KNN_Pair **original, struct KNN_Pair **new,
                 int points, int k)
{
    // Create an array that fits nearest neighbors from both original and new.
    struct KNN_Pair *both = (struct KNN_Pair *) malloc(
        sizeof(struct KNN_Pair) * k*2
    );

    for (int i = 0; i < points; i++) {
        memcpy(both, original[i], sizeof(struct KNN_Pair) * k);
        memcpy(both+k, new[i], sizeof(struct KNN_Pair) * k);
        qsort(both, k*2, sizeof(struct KNN_Pair), KNN_Pair_asc_comp);
        memcpy(original[i], both, sizeof(struct KNN_Pair) * k);
    }

    free(both);
}
