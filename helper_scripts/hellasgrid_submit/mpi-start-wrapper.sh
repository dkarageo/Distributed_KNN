#!/bin/bash
# mpi-start-wrapper.sh
#
# Created by Dimitrios Karageorgiou,
#   for course "Parallel And Distributed Systems".
#   Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
#
# Script intented for testing the implementation on HellasGrid nodes.
#
# Tests included:
#   -Uses both datasets provided under datasets folder.
#   -Tests for both blocking and non-blocking communications.
#   -Tests for k in range [1, 50].
#
# Submit using glite-wms interface (or later DIRAC interface), using
# provided distributed_knn.jdl file.
#

ASYNC_EXECUTABLE=$(pwd)/non_blocking_knn
BLOCKING_EXECUTABLE=$(pwd)/blocking_knn
DATA_FILENAME=mnist_train
SVD_MODIFIER=_svd
LABELS_MODIFIER=_labels
FILE_ENDING=.karas
K_START=1
K_STOP=50
MPI_FLAVOR=OPENMPI

# Convert flavor to lowercase for passing to mpi-start.
MPI_FLAVOR_LOWER=`echo $MPI_FLAVOR | tr '[:upper:]' '[:lower:]'`

# Pull out the correct paths for the requested flavor.
eval MPI_PATH=`printenv MPI_${MPI_FLAVOR}_PATH`

# Ensure the prefix is correctly set. Don't rely on the defaults.
eval I2G_${MPI_FLAVOR}_PREFIX=$MPI_PATH
export I2G_${MPI_FLAVOR}_PREFIX

#Print info about current node.
echo "Current cluster:" $(hostname)
echo "Working dir:" $(pwd)

# Do the same tests for async and blocking communications.
for my_executable in $ASYNC_EXECUTABLE $BLOCKING_EXECUTABLE;
do
    echo "======================================================"
    echo "======================================================"
    echo "Executable used:" $my_executable

    # Run tests for both normal and svd datasets.
    for datamod in "" $SVD_MODIFIER;
    do
        echo "======================================================"
        echo "Dataset used:" $DATA_FILENAME$datamod

        # Run for the given range of k nearest neighbors.
        for i in $(seq $K_START $K_STOP);
        do
            # Touch the executable. It exist must for the shared file system check.
            # If it does not, then mpi-start may try to distribute the executable
            # when it shouldn't.
            touch $my_executable

            # Setup for mpi-start.
            export I2G_MPI_APPLICATION=$my_executable
            export I2G_MPI_APPLICATION_ARGS="$DATA_FILENAME$datamod$FILE_ENDING $DATA_FILENAME$LABELS_MODIFIER$datamod$FILE_ENDING $i"
            export I2G_MPI_TYPE=$MPI_FLAVOR_LOWER
            export I2G_MPI_PRE_RUN_HOOK=mpi-hooks.sh
            #export I2G_MPI_POST_RUN_HOOK=mpi-hooks.sh

            # If these are set then you will get more debugging information.
            #export I2G_MPI_START_VERBOSE=1
            #export I2G_MPI_START_DEBUG=1

            # Invoke mpi-start.
            $I2G_MPI_START
        done

        echo "======================================================"
    done

    echo "======================================================"
    echo "======================================================"
done
