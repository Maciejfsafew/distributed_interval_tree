Parallel implementation of interval tree
========================================

The main purpose of this project is to create parallel implementation of interval tree, that will allow to create very large models (range >100.000.000). It is possible to create tree with ~100.000.000-200.000.000 of nodes on a single machine. In order to create more nodes it is necessary to use the cluster of machines (it is also possible to use swap/store values on a hard disk). 

Usually it is possible to do some additional preprocessing in order to shrink the size of tree:

- Map keys to smaller range e.g [ 1000000000, 100000000001 ] -> [ 0 , 1 ]

- Create buckets (approximation) e.g [ 1 , 2 , 3 , 4 ] -> [ 1, 2 ] 

This implementation is useful when it is not possible to apply aforementioned technics.


Prerequsitives:

MPI, C++, Interval Tree



#FILES

1.  GENERATOR
  
    generator.cpp

    ```bash
    g++ generator.cpp -o gen.out
    ./gen.out numbers range
    
    ./gen.out 1000 1000000 > test1000_1000000.in
    ```
    Creates test1000_1000000.in file with 1000 operations (insert/query) in range [0,1000000-1].

    Output format:
    ```text
    n
    a1_1 a2_1
    ...
    a1_n a2_n
    ```
    If a1_i == -1 => insert value at index a2_i   // insert operation
    else query interval [a1_i, a2_i]              // query operation



2.  BRUTE FORCE

    interval_tree_brute_force.cpp
    ```
    Time: O(n*r)  n - operations, r - range
    Memory: O(r)
    Type: int
    ```
    ```bash
    ./a.out < test100_100.in
    ```
    Output:
    ```
    Query results
    a
    b
    c
    ```

3.  SEQUENTIAL NLOGN

    interval_tree_sequential_nlogn.cpp
    ```
    Time: O(nlogr)   (r = range)
    Memory: O(r)    ~ 2 * range    (range MAX ~ 100.000.000 - 200.000.000)
    Type: int
    ```
    ```bash
    ./sequential.out range < test100000000_200000000  // range = 200.000.000
    ```

4.  PARALLEL
  
    interval_tree_parallel_batch.cpp
    ```
    Parallel implementation 
    Tests:
    Range -  tested with 1.000.000.000 (long longs) 
    Total Tree size ~ 2.000.000.000, size ~ 15 GB (couple of machines)
    ```
    ``` 
    Time: ~ O(n*(log(n-k))+log(k)/m) ( + network overhead )  m - machines, k - number of layers on slaves side
    Memory: ~ O(r)
    Type: long long
    ```
    ```bash
    $ time mpiexec -np procs ./parallel.out procs range levels batch < test1000_1000.in
    procs - nr of processes
    range - range of interval tree [0,range -1]
    levels - layers on root
    batch - grouping requests
    ```



