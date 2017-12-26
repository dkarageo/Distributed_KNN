An MPI based implementation of a knn search and classification algorithm,
aimed for use upon HPC systems.

Developed by Dimitrios Karageorgiou,
during the course "Parallel And Distributed Systems",
Aristotle University Of Thessaloniki, Greece,
2017-2018.

Its original target was HellasGrid nodes, though it may run upon any cluster
and/or setup that provides at least an MPI 1.0 implementation.

How to compile:
    make all

How to run:
    make runp k=#value_for_k_neighbors
