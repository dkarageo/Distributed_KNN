CC=mpicc
CFLAGS=-O3 -Wall -Wextra -lm -fopenmp

all: non_blocking blocking

non_blocking:
	$(CC) source/testing.c source/distributed_knn.c source/knn.c source/matrix.c \
		-o bin/non_blocking_knn $(CFLAGS)

blocking:
	$(CC) source/testing.c source/distributed_knn_blocking.c source/knn.c source/matrix.c \
		-o bin/blocking_knn $(CFLAGS) -D BLOCKING_COMMUNICATIONS

run:
	mpirun -np $(p) ./bin/non_blocking_knn dataset/mnist_train.karas \
			dataset/mnist_train_labels.karas 30

runp:
	mpirun -np $(p) ./bin/non_blocking_knn $(data) $(labels) $(k)

run_blocking:
	mpirun -np $(p) ./bin/blocking_knn dataset/mnist_train.karas \
			dataset/mnist_train_labels.karas 30

runp_blocking:
	mpirun -np $(p) $(data) $(labels) $(k)

purge:
	-rm bin/*

clean:
	-rm bin/*.o
