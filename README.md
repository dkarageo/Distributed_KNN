# **Distributed KNN**

## An MPI based implementation of a knn search and classification algorithm, aimed for use upon HPC systems.


Developed by *Dimitrios Karageorgiou*,\
during the course *Parallel And Distributed Systems*,\
*Aristotle University Of Thessaloniki, Greece,*\
*2017-2018.*

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
make run p=<num_of_processes>
```
or    
```
make runp p=<num_of_processes> data=<path_to_datafile> labels=<path_to_labelfile> k=<nearest_neighhbors_num>
```

#### Blocking communications:

```
make run_blocking p=<num_of_processes>
```
or
```
make runp_blocking p=<num_of_processes> data=<path_to_datafile> labels=<path_to_labelfile> k=<nearest_neighhbors_num>
```

In order to run in a cluster setup, compile using *make all* and then retrieve
compiled binaries out of *bin* folder and wrap them according to cluster's
manual.
