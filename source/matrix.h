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
 */
matrix_t *matrix_create(int32_t rows, int32_t cols);

/**
 * Destroys a matrix object and releases its resources.
 */
void matrix_destroy(matrix_t *matrix);

int32_t matrix_get_cols(matrix_t *matrix);
int32_t matrix_get_rows(matrix_t *matrix);

matrix_t *matrix_load_in_chunks(const char *filename,
								int32_t chunks_num,
								int32_t req_chunk);
int32_t matrix_get_chunk_offset(matrix_t *matrix);

double matrix_get_cell(matrix_t *matrix, int32_t row, int32_t col);
void matrix_set_cell(matrix_t *matrix, int32_t row, int32_t col, double value);

char *matrix_serialize(matrix_t *matrix, size_t *bytec);
matrix_t *matrix_deserialize(char *bytes, size_t bytec);


#endif
