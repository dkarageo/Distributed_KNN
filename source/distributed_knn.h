/**
 * distributed_knn.h
 *
 * Created by Dimitrios Karageorgiou,
 *  for course "Parallel And Distributed Systems".
 *  Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
 */

#ifndef __distributed_knn_h__
#define __distributed_knn_h__


#define MPI_TAG_OBJECT 1
#define MPI_MASTER 0

/**
 *
 */
struct KNN_Pair **knn_search_distributed(matrix_t *local_data, int k,
                                         int prev_task, int next_task,
                                         int tasks_num);

/**
 *
 */
matrix_t *knn_labeling_distributed(struct KNN_Pair **knns, int points, int k,
                                   matrix_t *local_labels, int prev_task,
                                   int next_task, int tasks_num);

/**
 *
 */
void _update_knns(struct KNN_Pair **original, struct KNN_Pair **new,
                 int points, int k);

/**
 *
 */
MPI_Request *_async_send_object(char *object, size_t length, int rank,
                               int *handlerc);
/**
 *
 */
MPI_Request *_async_recv_object(char **object, size_t *length,
                               int rank, int *handlerc);

/**
 *
 */
void _wait_async_com(MPI_Request *handlers, int handlerc);


#endif