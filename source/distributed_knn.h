/**
 * distributed_knn_blocking.h
 *
 * Created by Dimitrios Karageorgiou,
 *  for course "Parallel And Distributed Systems".
 *  Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
 *
 * This header provides a collection of routines and macros that may be used
 * for distributed knn search and labeling operations, based on MPI async
 * communications routines.
 *
 * Macros defined in distributed_knn_blocking.h:
 *  -MPI_TAG_OBJECT
 *  -MPI_MASTER
 *
 * Functions defined in distributed_knn.h:
 *  -matrix_t *knn_labeling_distributed(struct KNN_Pair **knns, int points, int k,
 *                                      matrix_t *local_labels, int prev_task,
 *                                      int next_task, int tasks_num)
 *  -struct KNN_Pair **knn_search_distributed(matrix_t *local_data, int k,
 *                                            int prev_task, int next_task,
 *                                            int tasks_num)
 *  -void _update_knns(struct KNN_Pair **original, struct KNN_Pair **new,
 *                    int points, int k)
 *  -MPI_Request *_async_send_object(char *object, size_t length, int rank,
 *                                  int *handlerc)
 *  -MPI_Request *_async_recv_object(char **object, size_t *length,
 *                                   int rank, int *handlerc)
 *  -void _wait_async_com(MPI_Request *handlers, int handlerc)
 */

#ifndef __distributed_knn_h__
#define __distributed_knn_h__


#define MPI_TAG_OBJECT 1  // A tag to be used when sending/receiving byte arrays.
#define MPI_MASTER 0      // The master task MPI_COMM_WORLD.

/**
 * Finds the k-nearest-neighbors for the given local block, by utilizing
 * all blocks available in processes of MPI_COMM_WORLD communicator.
 *
 * Points in local block are also included in look up dataset,
 * though returned values will never contain as a neighbor of a point, the point
 * itself.
 *
 * Parameters:
 *  -local_data: A matrix containing the cords of the points their k-nearest-
 *          neigbors are searched. Every row of the matrix should contain the
 *          cords of a single point and can be of arbitrary size.
 *  -k: The number of nearest neigbors to be returned.
 *  -prev_task: The rank of previous node in MPI_COMM_WORLD.
 *  -next_task: The rank of next node in MPI_COMM_WORLD.
 *  -tasks_num: The overall number of tasks in MPI_COMM_WORLD.
 *
 * Returns:
 *  A 2D array of (distance, index) pairs. Pairs in each row are the k nearest
 *  neighbors for the corresponding point (the one in the same row) in
 *  local_data matrix.
 */
struct KNN_Pair **knn_search_distributed(matrix_t *local_data, int k,
                                         int prev_task, int next_task,
                                         int tasks_num);

 /**
  * Labels the nearest neighbors contained in given array by utilizing remote
  * labels available in other processes of MPI_COMM_WORLD.
  *
  * Parameters:
  *  -knns: A 2D array of struct KNN_Pair objects that contains a set of k
  *          nearest neighbors. Usually it should be the one provided by
  *          knn_search_distributed().
  *  -points: The number of points contained knns.
  *  -k: The number of nearest neigbors for each point contained in knns.
  *  -local_labels: A matrix containing the labels available to the current
  *          proccess.
  *  -prev_task: The rank of previous node in MPI_COMM_WORLD.
  *  -next_task: The rank of next node in MPI_COMM_WORLD.
  *  -tasks_num: The overall number of tasks in MPI_COMM_WORLD.
  *
  * Returns:
  *  A matrix containing the labels of the k nearest neighbors in knns 2D array.
  */
matrix_t *knn_labeling_distributed(struct KNN_Pair **knns, int points, int k,
                                   matrix_t *local_labels, int prev_task,
                                   int next_task, int tasks_num);

/**
 * Updates an object with nearest neighbors provided by one knn search, by
 * using another one provided by a lookup on a different data block.
 *
 * It is the main function that combines results of many knn searches, thus
 * allowing a knn search to be done in parts, every time with a portion of the
 * complete block.
 *
 * Parameters:
 *  -original: The 2D knn pairs array, to be updated. It usually is the result
 *          of the first knn search.
 *  -new: A 2D knn pairs array, used to update the original one. It usually is
 *          the result of all sebsequent knn searchs, after the initial one.
 *  -points: The number of points for which nearest neighbors are contained in
 *           original and new objects. This number is expected to be the same
 *           for both objects, and probably equal to the number of points this
 *           node has to find k nearest neighbors for. Otherwise, result is
 *           undefined
 *  -k: The number of nearest neighbors contained for every point in original
 *          and new objects. Both should contain the same number of nearest
 *          neighbors. Otherwise, results are undefined.
*/
void _update_knns(struct KNN_Pair **original, struct KNN_Pair **new,
                 int points, int k);

/**
 * An asynchronous send operation.
 *
 * Initializes a sends operation for the provided bytes array to process with
 * given rank and returns a handlers array for this operation.
 *
 * A call to _async_send_object should be matched by a call to _wait_async_com.
 *
 * Parameters:
 *  -object: A bytes array to be send to given rank.
 *  -length: Length of object.
 *  -rank: Destination rank.
 *  -handlerc: A reference to a destination to return the number of returned
 *          handlers.
 *
 * Returns:
 *  An array of handlers for current operation.
 */
MPI_Request *_async_send_object(char *object, size_t length, int rank,
                               int *handlerc);
/**
 * An asynchronous receive operation.
 *
 * Initializes a receive operation for a bytes array from process with given
 * rank and stores a pointer to it into object argument. Returns an array of
 * handlers for this operation.
 *
 * A call to _async_recv_object should be matched by a call to _wait_async_com.
 *
 * Parameters:
 *  -object: A reference to the location to return the reference to received
 *          array.
 *  -length: A reference to the location to return the length of the received
 *          object.
 *  -rank: The rank of the process from which the object will be received.
 *  -handlerc: A reference to a destination to return the number of returned
 *          handlers.
 *
 * Returns:
 *  An array of handlers for current operation.
 */
MPI_Request *_async_recv_object(char **object, size_t *length,
                               int rank, int *handlerc);

/**
 * Blocks until operations defined by provided handlers are complete.
 *
 * Parameters:
 *  -handlers: An array of handlers, as returned by async communication
 *          operations.
 *  -handlerc: The number of handlers contained in handlers.
 */
void _wait_async_com(MPI_Request *handlers, int handlerc);


#endif
