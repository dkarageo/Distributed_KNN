#!/bin/bash
#PBS -q see
#PBS -N knn_4_8_mpich2_single
#PBS -j oe
#PBS -l nodes=4:ppn=8,mem=2gb,walltime=02:00:00

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

module load gcc/7.2.0


ASYNC_EXECUTABLE=bin/non_blocking_knn
BLOCKING_EXECUTABLE=bin/blocking_knn
DATA_FILENAME=dataset/mnist_train
SVD_MODIFIER=_svd
LABELS_MODIFIER=_labels
TEST_MODIFIER=_results
INDEXES_MODIFIER=_indexes
FILE_ENDING=.karas
K_START=1
K_STOP=50
MPI_FLAVOR=MPICH2

# Define the number of threads to be used by each process.
export OMP_NUM_THREADS=8
# Enable in order to spawn 1 process per node.
export I2G_MPI_SINGLE_PROCESS=1
# For processs per node <= 4, two requsted nodes may actually get allocated
# on the same node. If that's the case, comment I2G_MPI_SIGNLE_PROCESS and
# define explicity total number of processes.
#export I2G_MPI_NP=8
#export I2G_MPI_PER_NODE=2

cd $PBS_O_WORKDIR
export NP=$(cat $PBS_NODEFILE | wc -l)

# Convert flavor to lowercase for passing to mpi-start.
MPI_FLAVOR_LOWER=`echo $MPI_FLAVOR | tr '[:upper:]' '[:lower:]'`
# Pull out the correct paths for the requested flavor.
eval MPI_PATH=`printenv MPI_${MPI_FLAVOR}_PATH`
# Ensure the prefix is correctly set. Don't rely on the defaults.
eval I2G_${MPI_FLAVOR}_PREFIX=$MPI_PATH
export I2G_${MPI_FLAVOR}_PREFIX


#Print info about current node.
echo "Current cluster: " $(hostname)
echo "Working dir: " $(pwd)

# Go into actual project directory. Here actual script starts from.
cd Distributed_KNN

make all

# Do the same tests for async and blocking communications.
for my_executable in $ASYNC_EXECUTABLE $BLOCKING_EXECUTABLE;
do
    echo "======================================================"
    echo "======================================================"
    echo "Executable used:" $my_executable

	# Touch the executable. It exist must for the shared file system check.
	# If it does not, then mpi-start may try to distribute the executable
	# when it shouldn't.
	touch $my_executable

    # Run tests for both normal and svd datasets.
    for datamod in "" $SVD_MODIFIER;
    do
        echo "======================================================"
        echo "Dataset used:" $DATA_FILENAME$datamod

        # Run for the given range of k nearest neighbors.
        for k in $(seq $K_START $K_STOP);
        do
    	    export I2G_MPI_TYPE=$MPI_FLAVOR_LOWER
    	    export I2G_MPI_APPLICATION=$my_executable
    	    export I2G_MPI_APPLICATION_ARGS="$DATA_FILENAME$datamod$FILE_ENDING $DATA_FILENAME$LABELS_MODIFIER$datamod$FILE_ENDING $k $DATA_FILENAME$TEST_MODIFIER$datamod$FILE_ENDING" $DATA_FILENAME$TEST_MODIFIER$INDEXES_MODIFIER$datamod$FILE_ENDING"
    	    $I2G_MPI_START
	done

        echo "======================================================"
    done

    echo "======================================================"
    echo "======================================================"
done
