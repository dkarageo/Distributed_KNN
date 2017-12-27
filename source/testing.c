/**
 * testing.c
 *
 * Created by Dimitrios Karageorgiou,
 *  for course "Parallel And Distributed Systems".
 *  Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
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

    next_task = rank + 1;
    prev_task = rank - 1;
    if (rank == tasks_num - 1) next_task = 0;
    if (rank == 0) prev_task = tasks_num - 1;

    // Load a chunk of the data matrix, based on the rank id of current process.
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

    // Load the whole labels data. Classification won't be distributed.
    matrix_t *labels = matrix_load_in_chunks(labels_fn, tasks_num, rank);

    MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&start, NULL);

    // Classify each point in local data chunk. The complete labels table will
    // be used.
    matrix_t *labeled_results = knn_labeling_distributed(
            results, matrix_get_rows(initial_data), k,
            labels, prev_task, next_task, tasks_num
    );

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

    // A simple MPI_Gather operation is used. No need to go for ring topology.
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


double get_elapsed_time(struct timeval start, struct timeval stop)
{
    double elapsed_time = (stop.tv_sec - start.tv_sec) * 1.0;
    elapsed_time += (stop.tv_usec - start.tv_usec) / 1000000.0;
    return elapsed_time;
}
