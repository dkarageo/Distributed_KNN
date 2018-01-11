#!/bin/sh

#
# This function will be called before the MPI executable is started.
# You can, for example, compile the executable itself.
#
pre_run_hook () {

    # Compile the program.
    # echo "Compiling ${I2G_MPI_APPLICATION}"

    # Unpack the program.
    tar -xzf distributed_knn.tar.gz

    # Build it.
    cd Distributed_KNN
    #echo "Executing make from " $(pwd)
    make -s all
    cd ../

    # Copy executables and data to initial dir.
    #echo "Copying resources to root: " $(pwd)
    cp Distributed_KNN/bin/non_blocking_knn .
    cp Distributed_KNN/bin/blocking_knn .
    cp Distributed_KNN/dataset/mnist_train.karas .
    cp Distributed_KNN/dataset/mnist_train_labels.karas .
    cp Distributed_KNN/dataset/mnist_train_svd.karas .
    cp Distributed_KNN/dataset/mnist_train_labels_svd.karas .
    #ls -la

    # Everything's OK.
    #echo "Successfully compiled ${I2G_MPI_APPLICATION}"

    return 0
}

#
# This function will be called before the MPI executable is finished.
# A typical case for this is to upload the results to a storage element.
#
post_run_hook () {

    #echo "Executing post hook."
    # echo "Finished the post hook."

    return 0
}
