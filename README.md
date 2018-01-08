# **Distributed KNN**

## A hybrid MPI-OpenMP based implementation of a knn search and classification algorithm, aimed for use upon HPC systems.


Developed by *Dimitrios Karageorgiou*,\
during the course *Parallel And Distributed Systems*,\
*Aristotle University Of Thessaloniki, Greece,*\
*2017-2018.*

The provided implementation is a hybrid classifier, that utilizes both MPI
communications for communications between cluster's nodes and OpenMP for
parallel processing in the shared memory environment of each node.

Its original target was *HellasGrid* nodes, though it may run upon any cluster
and/or setup that provides at least an MPI 1.0 implementation.

### **How to compile:**
```
make all
```

In order to compile an implementation of MPI 1.0 or above, like OpenMPI, should
be present. Also, a compiler that supports OpenMP and C99 is required.

### **How to run on a shared memory setup:**

#### Non blocking communications:

```
export OMP_NUM_THREADS=<threads_per_process>
make run p=<num_of_processes>
```
or    
```
export OMP_NUM_THREADS=<threads_per_process>
make runp p=<num_of_processes> data=<path_to_datafile> labels=<path_to_labelfile> k=<nearest_neighhbors_num>
```

#### Blocking communications:

```
export OMP_NUM_THREADS=<threads_per_process>
make run_blocking p=<num_of_processes>
```
or
```
export OMP_NUM_THREADS=<threads_per_process>
make runp_blocking p=<num_of_processes> data=<path_to_datafile> labels=<path_to_labelfile> k=<nearest_neighhbors_num>
```

### **How to run on a cluster setup:**

For clusters that support *qsub* and *I2G_MPI_START* mechanism, there is a testing script under
*helper_scripts/local_cluster_submit* named *local_submit.sh*.

For submission upon *HellasGrid* (and further *EGI*) infrastracture, there is a JDL file and
two required scripts under *helper_scripts/hellasgrid_submit*.
