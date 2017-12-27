/**
 * distributed_knn.c
 *
 * Created by Dimitrios Karageorgiou,
 *  for course "Parallel And Distributed Systems".
 *  Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
 *
 * An implementation provided for routines defined in distributed_knn.h
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include "matrix.h"
#include "knn.h"
#include "distributed_knn.h"


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
    MPI_Request *send_req = NULL;  // Handlers for async send operations.
    MPI_Request *recv_req = NULL;  // Handlers for async receive operation.
    int send_req_num = 0;       // Number of send handlers.
    int recv_req_num = 0;       // Number of receive handlers

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
            // Start sending current data to next process.
            out_object = matrix_serialize(cur_labels, &out_size);
            send_req = _async_send_object(
                    out_object, out_size, next_task, &send_req_num);

            // Start receiving next data from previous process.
            recv_req = _async_recv_object(
                   &in_object, &in_size, prev_task, &recv_req_num);
        }

        labeled = knn_labeling(knns, points, k, labeled, index_counter,
                               cur_labels, matrix_get_chunk_offset(cur_labels));

       // On final iterations, no communications exist.
       if (i < tasks_num - 1) {
           // Wait for send/receive operations to complete.
           _wait_async_com(send_req, send_req_num);
           _wait_async_com(recv_req, recv_req_num);

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
    MPI_Request *send_req = NULL;  // Handlers for async send operations.
    MPI_Request *recv_req = NULL;  // Handlers for async receive operation.
    int send_req_num = 0;       // Number of send handlers.
    int recv_req_num = 0;       // Number of receive handlers.
    struct KNN_Pair **knns = NULL; // K Nearest Neighbors for current query chunk.
    struct KNN_Pair **knn_helper = NULL;  // MEMORY LEAK CURRENTLY -- FIX IT LATER!!!

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
            // Start sending current data to next process.
            out_object = matrix_serialize(cur_data_block, &out_size);
            send_req = _async_send_object(
                    out_object, out_size, next_task, &send_req_num);

            // Start receiving next data from previous process.
            recv_req = _async_recv_object(
                   &in_object, &in_size, prev_task, &recv_req_num);
        }

        // On first iteration, search is done using the local data chunk.
        if (i == 0) {
            // First nearest neighbor will be the point itself, so remove it.
            knn_helper = knn_search(cur_data_block, local_data, k+1,
                    matrix_get_chunk_offset(cur_data_block));
            knns = KNN_Pair_create_subtable_ref(
                    knn_helper, matrix_get_rows(local_data), 1);
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
            // Wait for send/receive operations to complete.
            _wait_async_com(send_req, send_req_num);
            _wait_async_com(recv_req, recv_req_num);

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


MPI_Request *_async_send_object(char *object, size_t length, int rank,
                               int *handlerc)
{
    MPI_Request *handlers = (MPI_Request *) malloc(sizeof(MPI_Request));
    *handlerc = 1;

    // Send object.
    MPI_Isend(object, length, MPI_CHAR, rank, MPI_TAG_OBJECT,
              MPI_COMM_WORLD, handlers);

    return handlers;
}


MPI_Request *_async_recv_object(char **object, size_t *length, int rank, int *handlerc)
{
    MPI_Request *handlers = (MPI_Request *) malloc(sizeof(MPI_Request));
    *handlerc = 1;

    int size;
    MPI_Status status;

    // Probe for length of incoming object.
    MPI_Probe(rank, MPI_TAG_OBJECT, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_CHAR, &size);
    *length = (size_t) size;

    // Start receiving the object.
    *object = (char *) malloc(sizeof(char) * size);
    MPI_Irecv(*object, size, MPI_CHAR, rank, MPI_TAG_OBJECT,
              MPI_COMM_WORLD, handlers);

    return handlers;
}


void _wait_async_com(MPI_Request *handlers, int handlerc) {
    // if (handlers == NULL) printf("handlers is null\n");
    // printf("handlerc: %d\n", handlerc);
    // printf("_wait_async_com() : handlers addr: 0x%lx\n", (long unsigned int) handlers);

    MPI_Waitall(handlerc, handlers, MPI_STATUSES_IGNORE);
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
