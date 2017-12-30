#!/bin/bash
#PBS -q see
#PBS -N knn_16_8
#PBS -j oe
#PBS -l nodes=16:ppn=8,mem=2gb

#
# local_submit.sh
#
# Created by Dimitrios Karageorgiou,
#   for course "Parallel And Distributed Systems".
#   Electrical and Computers Engineering Department, AuTh, GR - 2017-2018
#
# Script intented for testing the implementation on clusters that support
# qsub.
#
# Tests included:
#   -Uses both datasets provided under datasets folder.
#   -Tests for both blocking and non-blocking communications.
#   -Tests for k in range [1, 50].
#
# In order to change the number of processors, modify 'nodes' and 'ppn'
# atrributes at the beggining of this script.
#

module load openmpi-1.10-x86_64
module load gcc/7.2.0

cd $PBS_O_WORKDIR

#Print info about current node.
echo "Current cluster: " $(hostname)
echo "Working dir: " $(pwd)

ASYNC_EXECUTABLE=bin/non_blocking_knn
BLOCKING_EXECUTABLE=bin/blocking_knn
DATA_FILENAME=dataset/mnist_train
SVD_MODIFIER=_svd
LABELS_MODIFIER=_labels
FILE_ENDING=.karas
K_START=1
K_STOP=50

# Go into actual project directory. Here actual script starts from.
cd Distributed_Knn

make -s all

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
        for k in $(seq $K_START $K_STOP);
        do
            mpiexec -np $PBS_NP \
                    ./$my_executable $DATA_FILENAME$datamod$FILE_ENDING \
                    $DATA_FILENAME$LABELS_MODIFIER$datamod$FILE_ENDING $k
        done

        echo "======================================================"
    done

    echo "======================================================"
    echo "======================================================"
done
