CC=mpicc
CFLAGS=-O3 -Wall -Wextra

all: non_blocking

non_blocking:
	$(CC) source/testing.c source/distributed_knn.c source/knn.c source/matrix.c \
		-o bin/non_blocking_knn $(CFLAGS) -lm -fopenmp

run:
	./bin/non_blocking_knn dataset/mnist_train.karas \
			dataset/mnist_train_labels.karas 30

runp:
	./bin/non_blocking_knn dataset/mnist_train.karas \
			dataset/mnist_train_labels.karas $(k)

runp_svd:
	./bin/non_blocking_knn dataset/mnist_train_svd.karas \
			dataset/mnist_train_labels_svd.karas $(k)

purge:
	-rm bin/*

clean:
	-rm bin/*.o
