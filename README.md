<h1>Parallel implementation of interval tree<h1>

The main purpose of this project is to create parallel implementation of interval tree, that will allow to create very large models (range >1.000.000.000).
It is possible to create tree with 500.000.000 of nodes on a single machine. 
In order to create more nodes it is necessary to use the cluster of machines (it is also possible to use swap/store values on a hard disk). 

Usually it is possible to do some additional preprocessing in order to shrink the size of tree:
- Map keys to smaller range e.g [1000000000, 100000000001] -> [0,1]
- Create buckets e.g [1,2,3,4] -> [1,2] (approximation)
This implementation is useful when it is not possible to 


MPI - Message Passing Interface
C++

Prerequsitives:
MPI, C++, Interval Tree



FILES:

I. generator.cpp
g++ generator.cpp -o gen.out
./gen.out numbers range

./gen.out 1000 1000000 > test1000_1000000.in
Creates file test1000_1000000.in with 1000 operations (insert/query) in range [0,1000000-1].

Output format:

n
a1_1 a2_1
...
a1_n a2_n

If a1_i == -1 => insert value at index a2_i   // insert operation
else query interval [a1_i, a2_i]              // query operation



II.
interval_tree_brute_force.cpp
O(n*k)  n - operations, k - interval

./a.out < test100_100.in

Output:
Query results
a
b
c

III.
interval_tree_sequential_nlogn.cpp
Time: O(nlogn) 
Memory: O(n)    ~ 2 * range    (range MAX ~ 500.000.000 - 1000.000.000)
Type: int


./sequential.out range < test100000000_200000000  // range = 200.000.000


IV.
interval_tree_parallel_1_proc.cpp
Prototype
Time: O(n*log(range))
Memory: O(n)

V.
interval_tree_parallel.cpp
Parallel implementation (range MAX (theorethically) ~ machines * 200.000.000))
Time: ~ O(n*(log(n-k))+log(k)/m) ( + network overhead )  m - machines, k - number of layers on slaves side
Type: long long
time mpiexec -np procs ./parallel.out procs range levels < test1000_1000.in
procs - nr of processes
range - range of interval tree [0,range -1]
levels - layers on root
There is communication overhead in this implementation. 



VI. FINAL VERSION
interval_tree_parallel_batch.cpp
Optimized version of parallel implementation. (Reqest batching)
In progress


