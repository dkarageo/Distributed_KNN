#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "matrix.h"


matrix_t *matrix_create(int32_t rows, int32_t cols)
{
	// Allocate a new matrix_t object.
	matrix_t *matrix = (matrix_t *) malloc(sizeof(matrix_t));
	if (matrix == NULL) return NULL;

	matrix->rows = rows;
	matrix->cols = cols;

	// Allocate memory for rows * cols doubles.
	matrix->data = (double **) malloc(sizeof(double *) * rows);
	if (matrix->data == NULL) return NULL;  // -- Memory leak, cleanup pending. --

	for (int32_t i = 0; i < rows; i++) {
		matrix->data[i] = (double *) malloc(sizeof(double) * cols);
		if (matrix->data[i] == NULL) return NULL;  // -- Memory leak, cleanup pending. --
	}

    // A newly created matrix is always considered to be the first in its
    // container array.
    matrix->chunk_offset = 0;

	return matrix;
}


void matrix_destroy(matrix_t *matrix)
{
    for (int32_t i = 0; i < matrix->rows; i++) {
		free(matrix->data[i]);
	}
	free(matrix);
}


int32_t matrix_get_rows(matrix_t *matrix)
{
    return matrix->rows;
}


int32_t matrix_get_cols(matrix_t *matrix)
{
    return matrix->cols;
}


matrix_t *matrix_load_in_chunks(const char *filename,
								int32_t chunks_num,
								int32_t req_chunk)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("ERROR: matrix_load_in_chunks : Failed to open %s\n", filename);
        return NULL;
    }

	int32_t total_rows;  // Total rows contained in file.
	int32_t rows;  // Rows contained in requested chunk.
	int32_t cols;  // Columns of the matrix.

	// Read total rows counter.
	int rc = fread(&total_rows, sizeof(int32_t), 1, f);
	if (rc != 1) {
		printf("ERROR: Failed to read rows number.\n");
		exit(-1);
	}

	// Read columns counter.
	rc = fread(&cols, sizeof(int32_t), 1, f);
	if (rc != 1) {
		printf("ERROR: Failed to read cols number.\n");
		exit(-1);
	}

	// Break the file into given number of chunks and set file
	// position to the beggining of the requested chunk.
	rows = total_rows / chunks_num;
	int remaining = total_rows % chunks_num;
	long int offset;
	if (req_chunk < remaining) {
		rows++;
		offset = req_chunk * rows;
	} else {
		offset = ((rows + 1) * remaining) + (rows * (req_chunk - remaining));
	}

	rc = fseek(f, sizeof(double) * offset * cols, SEEK_CUR);

	matrix_t *matrix = matrix_create(rows, cols);
    matrix->chunk_offset = offset;  // Set the offset of the matrix object
                                    // to the number of rows from the beggining
                                    // of the file, till the first row to
                                    // be included in this matrix.

	// Read data from file, row by row.
	for(int32_t i = 0; i < rows; i++) {
		rc = fread(matrix->data[i], sizeof(double), cols, f);
		if (rc != cols) {
			printf("Failed in reading col %d. \n", i+1);
			exit(-1);
		}
	}

	fclose(f);

	return matrix;
}

int32_t matrix_get_chunk_offset(matrix_t *matrix)
{
    return matrix->chunk_offset;
}

double matrix_get_cell(matrix_t *matrix, int32_t row, int32_t col)
{
	return matrix->data[row][col];
}


void matrix_set_cell(matrix_t *matrix, int32_t row, int32_t col, double value)
{
    matrix->data[row][col] = value;
}

char *matrix_serialize(matrix_t *matrix, size_t *bytec)
{
    int32_t rows = matrix_get_rows(matrix);
    int32_t cols = matrix_get_cols(matrix);
    int32_t offset = matrix_get_chunk_offset(matrix);

    // Allocate space for 3 ints (rows and columns counter, offset) and all cells.
    *(bytec) = sizeof(int32_t) * 3 + sizeof(double) * rows * cols;

    char *serialized = (char *) malloc (sizeof(char) * (*bytec));
    if (!serialized) {
        printf("ERROR: matrix_serialize : Failed to allocate memory.\n");
        return NULL;
    }

    // Write matrix to its serialized form.
    char *buffer = serialized;

    memcpy(buffer, &rows, sizeof(int32_t)); buffer += sizeof(int32_t);
    memcpy(buffer, &cols, sizeof(int32_t)); buffer += sizeof(int32_t);
    memcpy(buffer, &offset, sizeof(int32_t)); buffer += sizeof(int32_t);

    for (int32_t i = 0; i < rows; i++) {
        memcpy(buffer, matrix->data[i], sizeof(double) * cols);
        buffer += sizeof(double) * cols;
    }

    return serialized;
}

matrix_t *matrix_deserialize(char *bytes, size_t bytec)
{
    char *buffer = bytes;

    int32_t rows;
    int32_t cols;
    int32_t offset;

    memcpy(&rows, buffer, sizeof(int32_t)); buffer += sizeof(int32_t);
    memcpy(&cols, buffer, sizeof(int32_t)); buffer += sizeof(int32_t);
    memcpy(&offset, buffer, sizeof(int32_t)); buffer += sizeof(int32_t);

    if (bytec != sizeof(int32_t) * 3 + sizeof(double) * rows * cols) {
        printf("ERROR: matrix_deserialize : Given and actual size not matching.\n");
        return NULL;
    }

    matrix_t *matrix = matrix_create(rows, cols);
    matrix->chunk_offset = offset;

    for (int32_t i = 0; i < rows; i++) {
        memcpy(matrix->data[i], buffer, sizeof(double) * cols);
        buffer += sizeof(double) * cols;
    }

    return matrix;
}
