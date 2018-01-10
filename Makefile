CC=mpicc
CFLAGS=-O3 -Wall -Wextra -lm -fopenmp -std=c99

results=""

all: non_blocking blocking

non_blocking: bin_dir
	$(CC) source/testing.c source/distributed_knn.c source/knn.c source/matrix.c \
		-o bin/non_blocking_knn $(CFLAGS)

blocking: bin_dir
	$(CC) source/testing.c source/distributed_knn_blocking.c source/knn.c source/matrix.c \
		-o bin/blocking_knn $(CFLAGS) -D BLOCKING_COMMUNICATIONS

bin_dir:
	mkdir -p bin

run:
	mpirun -np $(p) ./bin/non_blocking_knn dataset/mnist_train.karas \
			dataset/mnist_train_labels.karas 30 dataset/mnist_train_results.karas

runp:
	mpirun -np $(p) ./bin/non_blocking_knn $(data) $(labels) $(k) $(results)

run_blocking:
	mpirun -np $(p) ./bin/blocking_knn dataset/mnist_train.karas \
			dataset/mnist_train_labels.karas 30 dataset/mnist_train_results.karas

runp_blocking:
	mpirun -np $(p) ./bin/blocking_knn $(data) $(labels) $(k) $(results)

purge:
	-rm bin/*

clean:
	-rm bin/*.o
