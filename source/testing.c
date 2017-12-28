/**
 * testing.c
 *
 * Created by Dimitrios Karageorgiou,
 *  for course "Parallel And Distributed Systems".
 *  Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
 *
 * This file implements the main entry point for a basic testing/presentation
 * of the distributed knn implementation.
 *
 * It allows for self-classificiation of a provided dataset.
 *
 * In order to do a successfull testing, two files required. One with the cords
 * of the points to be classified and the other with the labels of these points
 * that has been previously calculated. In this way, every single provided point
 * is classified by using all the other points. Finally, a result in percentage
 * is presented, reffering to the percentage of correctly classified points
 * by comparing to their actual provided class.
 *
 * It utilizes MPI routines, and in order to successfully compile and run,
 * at least an MPI 1.0 implementation should exist.
 *
 * Common usage for testing on shared memory systems:
 *  mpirun -np <procs_num> ./<binary_name> <path_to_data_file> \
 *          <path_to_labels_file> <k>
 *      where:
 *          -procs_num : Number of processes to be spawned.
 *          -binary_name : The name of the compiled binary.
 *          -path_to_data_file : Path to a .karas file containing the cords
 *              of the points to be used in testing.
 *          -path_to_labels_file : Path to a .karas file containing the actual
 *              labels of the points contained in data file.
 *          -k : The number of k-nearest-neighbors to be used for classification.
 * path_to_data_file, path_to_labels_file and k arguments should be provided in
 * every setup the executable will run upon. Though, in a cluster setup compile
 * the executable and follow cluster's guide to properly submit it.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include "knn.h"
#include "matrix.h"

// If BLOCKING_COMMUNICATIONS is defined, then use header with
// blocking communications. Else use the one with async ones. Also, the
// appropriate implementation file should be linked.
// ====Since there is no explicit shared interface between these two headers,
// compatibility between those two is provided for now as a convenience.
// May implement it different later.====
#ifdef BLOCKING_COMMUNICATIONS
    #include "distributed_knn_blocking.h"
#else
    #include "distributed_knn.h"
#endif


double get_elapsed_time(struct timeval start, struct timeval stop);


int main(int argc, char *argv[])
{
	if (argc < 4) {
		printf("Required args: data_filename, labels_filename, k\n");
		exit(-1);
	}

    char *data_fn = argv[1];  // Filename of data matrix.
    char *labels_fn = argv[2];  // Filename of labels matrix.
    int k = atoi(argv[3]);  // Number of nearest neighbors to be used for search.

    int tasks_num;  // Tasks in MPI_COMM_WORLD.
	int rank;       // Task ID of current task, in MPI_COMM_WORLD.
    int next_task;  // ID of next task in ring topology.
    int prev_task;  // ID of previous task in ring topology.

    struct timeval start, stop;

    // Initialize MPI env.
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &tasks_num);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Setup a ring topology.
    next_task = rank + 1;
    prev_task = rank - 1;
    if (rank == tasks_num - 1) next_task = 0;
    if (rank == 0) prev_task = tasks_num - 1;

    // Load a chunk of the data matrix, based on the rank of current process.
    matrix_t *initial_data = matrix_load_in_chunks(data_fn, tasks_num, rank);
    if (!initial_data) {
        printf("ERROR: Failed to load data matrix in task %d.\n", rank);
        MPI_Finalize();
        exit(-1);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&start, NULL);

    // Find the k nearest neighbors for local data chunk.
    struct KNN_Pair **results = knn_search_distributed(
            initial_data, k, prev_task, next_task, tasks_num);

    MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&stop, NULL);

    if (rank == MPI_MASTER) {
        printf("knn search using %d processes took: %.2f secs.\n",
               tasks_num, get_elapsed_time(start, stop));
    }

    // Load the labels chunk belonging to current process according to its rank.
    matrix_t *labels = matrix_load_in_chunks(labels_fn, tasks_num, rank);
    if (!labels) {
        printf("ERROR: Failed to load labels matrix in task %d.\n", rank);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&start, NULL);

    // Perform a distributed labeling process, in order to get the labels
    // of the nearest neighbors previously found.
    matrix_t *labeled_results = knn_labeling_distributed(
            results, matrix_get_rows(initial_data), k,
            labels, prev_task, next_task, tasks_num
    );

    // Finally, use the labels of nearest neighbors, to classify each point
    // in initial data.
    matrix_t *classified = knn_classify(labeled_results);

    MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&stop, NULL);

    if (rank == MPI_MASTER) {
        printf("knn classification using %d processes took: %.2f secs.\n",
               tasks_num, get_elapsed_time(start, stop));
    }

    // Verify the local classification results.
    matrix_t *local_labels = matrix_load_in_chunks(labels_fn, tasks_num, rank);
    int valid = 0;
    for (int i = 0; i < matrix_get_rows(classified); i++) {
        if (matrix_get_cell(classified, i, 0) ==
            matrix_get_cell(local_labels, i, 0))
        {
            valid++;
        }
    }

    // Transmit local results to master process to join and present them.
    int *success = NULL;
    int *total = NULL;

    if (rank == MPI_MASTER) {
        success = (int *) malloc(sizeof(int) * tasks_num);
        total = (int *) malloc(sizeof(int) * tasks_num);
    }

    int current_points = matrix_get_rows(classified);

    // A simple MPI_Gather operation is used. There is absolutely no need to go
    // for ring topology. That would be a waste.
    MPI_Gather(&valid, 1, MPI_INT, success, 1, MPI_INT,
               MPI_MASTER, MPI_COMM_WORLD);
    MPI_Gather(&current_points, 1, MPI_INT, total, 1, MPI_INT,
               MPI_MASTER, MPI_COMM_WORLD);

    if (rank == MPI_MASTER) {
        int total_valid = 0;
        int total_all = 0;
        for (int i = 0; i < tasks_num; i++) {
            total_valid += success[i];
            total_all += total[i];
        }

        printf("k = %d - classification accuracy: %2.1f %%\n",
               k, ((double) total_valid / (double) total_all) * 100.0);
    }

	matrix_destroy(initial_data);
    matrix_destroy(labels);
    matrix_destroy(local_labels);
    matrix_destroy(classified);

    MPI_Finalize();

    return 0;
}

/*
 * Returns the elapsed time in seconds between the two provided
 * timeval objects.
 *
 * Parameters:
 *  -start : A timeval object representing the beggining moment.
 *  -stop : A timeval obect representing the ending moment.
 *
 * Returns:
 *  The elapsed time in seconds format, with the same precision as
 *  the precision of the provided timeval objects.
 */
double get_elapsed_time(struct timeval start, struct timeval stop)
{
    double elapsed_time = (stop.tv_sec - start.tv_sec) * 1.0;
    elapsed_time += (stop.tv_usec - start.tv_usec) / 1000000.0;
    return elapsed_time;
}
