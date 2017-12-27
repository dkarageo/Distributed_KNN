/**
 * matrix.h
 *
 * Created by Dimitrios Karageorgiou,
 *  for course "Parallel And Distributed Systems".
 *  Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
 *
 * matrix.h defines routines that may be used for creating and managing
 * matrices of doubles.
 *
 * Types defined in knn.h:
 *  -matrix_t (opaque)
 *
 * Functions defined in knn.h:
 *	-matrix_t *matrix_create(int32_t rows, int32_t cols)
 *	-void matrix_destroy(matrix_t *matrix)
 *	-int32_t matrix_get_cols(matrix_t *matrix)
 *	-int32_t matrix_get_rows(matrix_t *matrix)
 *	-double matrix_get_cell(matrix_t *matrix, int32_t row, int32_t col)
 *	-int32_t matrix_get_chunk_offset(matrix_t *matrix)
 *	-void matrix_set_cell(matrix_t *matrix, int32_t row, int32_t col, double value)
 *	-matrix_t *matrix_load_in_chunks(const char *filename,
 *	   								 int32_t chunks_num,
 * 									 int32_t req_chunk)
 *	-char *matrix_serialize(matrix_t *matrix, size_t *bytec)
 *	-matrix_t *matrix_deserialize(char *bytes, size_t bytec)
 */

#ifndef __matrix_h__
#define __matrix_h__

#include <stdint.h>


typedef struct {
	double **data;             // Actual data of the matrix.
	int32_t rows;              // Rows counter of the matrix.
	int32_t cols;              // Columns counter of the matrix.
    int32_t chunk_offset;      // Offset of this matrix, in its container
                               // array, if it belongs to any.
} matrix_t;


/**
 * Creates a new empty matrix object.
 *
 * Elements are not initialized, and their initial value is undefined.
 *
 * Parameters:
 *	-rows: Number of rows the new matrix will contain.
 *	-cols: Number of cols the new matrix will contain.
 *
 * On successful creation returns the matrix object. On failure, returns
 * NULL.
 */
matrix_t *matrix_create(int32_t rows, int32_t cols);

/**
 * Destroys a matrix object and releases its resources.
 *
 * Parameters:
 *	-matrix: The matrix to destroy.
 */
void matrix_destroy(matrix_t *matrix);

/**
 * Returns number of columns of the given matrix.
 */
int32_t matrix_get_cols(matrix_t *matrix);

/**
 * Returns number of rows of the given matrix.
 */
int32_t matrix_get_rows(matrix_t *matrix);

/**
 * Loads a chunk of a matrix object stored to filesystem.
 *
 * Matrix is separated in chunks only by rows. Thus, each chunk will contain
 * a portions of the total rows, though each one with all its columns.
 * Chunks should be as equal in size as possible. When differ,
 * the smallest from the biggest chunk won't differ by more than 1 rows.
 * Chunks are 0-indexed and their indexes range into [0, chunks_num-1].
 *
 * In order to load the complete matrix, chunks_num can
 * be set to 1 and req_chunk to 0.
 *
 * When loading a chunk, the offset of its first row from the beggining of the
 * complete matrix can be queried by using matrix_get_chunk_offset() function.
 *
 * Parameters:
 *	-filename: A path to a file that stores a matrix object.
 * 	-chunks_num: The total number of chunks the matrix should be divided into.
 *	-req_chunk: The index of chunk to be loaded. It ranges into [0, chunks_num-1].
 *
 * Returns:
 *	On successful loading, the matrix object corresponding to requested chunk.
 *	On failure, returns NULL.
 */
matrix_t *matrix_load_in_chunks(const char *filename,
								int32_t chunks_num,
								int32_t req_chunk);

/**
 * Returns the offset of the first row of current matrix chunk
 * from the beggining of the complete matrix.
 */
int32_t matrix_get_chunk_offset(matrix_t *matrix);

/**
 * Returns the value of matrix cell.
 */
double matrix_get_cell(matrix_t *matrix, int32_t row, int32_t col);

/**
 * Sets the value of a matrix cell to the given one.
 */
void matrix_set_cell(matrix_t *matrix, int32_t row, int32_t col, double value);

/**
 * Serializes the given matrix object.
 *
 * Parameters:
 *	-matrix: The matrix to serialize.
 *	-bytec: A reference to the location to write the size in bytes of serialized
 *			object.
 *
 * Returns:
 *	On success, a reference to the serialized object. On failure, it returns
 *	NULL.
 */
char *matrix_serialize(matrix_t *matrix, size_t *bytec);

/**
 * Inflates a matrix object, out of its serial representaton.
 *
 * Parameters:
 *	-bytes: A reference to the serial representation of the matrix.
 *	-bytec: The size of the serial representation.
 *
 * Returns:
 *	On success returns a matrix object. On failure returns NULL.
 */
matrix_t *matrix_deserialize(char *bytes, size_t bytec);

#endif
