#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <algorithm>
using namespace std;
////////////////////////////////////////////////////////////////////////////////////////
//GLOBAL_VARIABLES
////////////////////////////////////////////////////////////////////////////////////////
// nr of procs
int proc;
// total range of tree
long long range;
// range of single subree
typedef int num;
num sub_range;

// number of batched requests
int batch;
// proc nr
int world_rank;
// nr of processes
int world_size;

// lvls in root tree (layers in tree)
int lvl_of_root;

MPI::Status status;
// range of single sub tree
// value to add to get index of leaf [shift + val]
long long shift;
// the same - subtree
num sub_shift;

long long root_tree_size;

//size of root tree
long long size;
//total size of sub tree
num sub_size;
////////////////////////////////////////////////////////////////////////////////////////
// SUBNODES
////////////////////////////////////////////////////////////////////////////////////////
// nr of sub trees in subnode
int nr_of_subtrees;
// array of subtrees
num** trees;

void sub_insert(num * tree, num x);
num sub_query(num* tree, num a, num b);

//ROOT VARIABLES
num* tree;
//array of queries per process, query = triples
num** queries;
//backup - speedup
num** queries_backup;
//querys per process
num* queries_count;
//backup - speedup
int* queries_count_backup;
//array of pending requests
num* all_requests;
num* all_requests_backup;
//total count of requests
int total_count = 0;
int total_count_backup = 0;
//indexes all_requests array
int** requests_queries;
int** requests_queries_backup;

// owner of subtree
int* procOwner;
// owner's nr of tree
int* countNr;
//
num* procCounts;

// lines in file
long long n;
// slave buffer
num *buff;
// max_buff = max(queries_count)
// if max_buff > value : send all buffers
int max_buff = 0;

//Create subtrees
void initialize(int count){
  trees = new num *[count];
  sub_size = 1;
  while(sub_size<sub_range) sub_size *= 2;
  for(int i = 0; i < count; i ++){
    trees[i] = new num[2*sub_size];
    for(int j = 0; j < 2*sub_size; j ++)trees[i][j] = 0;
  }
}
void insert_root(long long x){
  long long begin = 0, end = range - 1, v = 1, lvl = 1;
  tree[v]++;
  while(lvl<lvl_of_root){
    long long mid = begin + (end - begin)/2;
    if(x>mid){
      v = 2 * v + 1;
      begin = mid + 1; 
    } else {
      v = 2 * v;
      end = mid;
    }
    tree[v]++;
    lvl++;
  }
  v = v - shift;

  queries[procOwner[v]][3*queries_count[procOwner[v]]]=-1;
  queries[procOwner[v]][3*queries_count[procOwner[v]]+1]=countNr[v];
  queries[procOwner[v]][3*queries_count[procOwner[v]]+2]=x-begin;
  requests_queries[procOwner[v]][queries_count[procOwner[v]]]=total_count;
  queries_count[procOwner[v]]++;

  max_buff = max(max_buff,queries_count[procOwner[v]]);

  all_requests[total_count]=-1;
  total_count++;
}
void lower(long long x){
  long long begin = 0, end = range -1, v = 1, lvl = 1;
  long long res = 0;
  while(lvl<lvl_of_root){
    long long mid = begin + (end - begin)/2;
    if(x > mid) {
      res += tree[2*v];
      begin = mid + 1;
      v = 2 * v + 1;
    } else {
      end = mid;
      v = 2 * v;
    }
    lvl ++;
  }
  v = v - shift;

  queries[procOwner[v]][3*queries_count[procOwner[v]]]=countNr[v];
  queries[procOwner[v]][3*queries_count[procOwner[v]]+1]=0;
  queries[procOwner[v]][3*queries_count[procOwner[v]]+2]=x-1-begin;
  requests_queries[procOwner[v]][queries_count[procOwner[v]]]=total_count;
  queries_count[procOwner[v]]++;

  max_buff = max(max_buff,queries_count[procOwner[v]]);
  all_requests[total_count]=res;
  total_count++;

}

void query_root(long long a, long long b){
  lower(b+1);
  lower(a);
}
void send(){
  for(int i = 0; i < proc; i ++){
    num w = queries_count_backup[i];
    if(w>0){
  	  MPI::COMM_WORLD.Recv(queries_backup[i],w,MPI_INT,i,0,status);
    }
  }
  
  for(int i = 0; i < proc; i ++){
    int w = queries_count[i];
    if(w>0){
  	  MPI::COMM_WORLD.Send(&w,1,MPI_INT,i,0);
  	  MPI::COMM_WORLD.Send(queries[i],3*w,MPI_INT,i,0);
    }
  }
  
  for(int i = 0; i < proc; i++){
	  int w = queries_count[i];
	  for(int j = 0; j < w; j++){
		  all_requests_backup[requests_queries_backup[i][j]]+=queries_backup[i][j];
	  }
	  queries_count_backup[i] = 0;
  }	
  int i = 0;
  
  while(i < total_count_backup){
	if(all_requests_backup[i]>-1){
	  printf("%d\n",all_requests_backup[i]-all_requests_backup[i+1]);
	  i+=2;
	} else {
	  //printf("%lld\n",all_requests[i]);
	  i++;
	}
  }
  
  swap(all_requests,all_requests_backup);
  swap(requests_queries,requests_queries_backup);
  swap(queries,queries_backup);
  swap(queries_count,queries_count_backup);
  swap(total_count,total_count_backup);
  
  total_count = 0;
  max_buff=0;
}
void parallel(){
  if ( world_rank == 0){
    tree = new num[size];
    for(int i = 0; i < size; i++) tree[i] = 0;
    procOwner = new int[shift];
    countNr = new int[shift];
    procCounts = new num[proc];
    for(int i = 0; i < proc; i++) procCounts[i] = 0;
    for(int i = 0; i < shift; i++){
      procOwner[i]=1+i%(proc-1);
      procCounts[procOwner[i]]++;
      countNr[i] = procCounts[procOwner[i]]-1;
    }
    for(int i = 1; i < proc; i++){
      MPI::COMM_WORLD.Send(&procCounts[i],1,MPI_INT,i,0);
    } 
    queries = new num*[proc];
    queries_backup = new num*[proc];
    requests_queries = new num*[proc];
    requests_queries_backup = new num*[proc];
    queries_count = new int[proc];
    queries_count_backup = new int[proc];
    all_requests = new num[(proc-1)*batch];
    all_requests_backup = new num[(proc-1)*batch];
    for(int i = 0; i < proc; i++){
      queries_count[i] = 0;
      queries_count_backup[i] = 0;
	  queries[i] = new num[3*batch];
	  queries_backup[i] = new num[3*batch];
	  requests_queries[i] = new int[3*batch];
	  requests_queries_backup[i] = new int[3*batch];	
	}
    
    scanf("%lld",&n);
    long long a,b; 
    for(int i = 0; i < n; i++){
      scanf("%lld %lld",&a,&b);
      if(a == -1){
        insert_root(b);
      } else {
        query_root(a,b);
      }
      if(max_buff >= batch-1){
        send();
      }  
    }
    send();
    send();
    
    num w = -1;
    for(int i = 1; i < proc; i++){
      MPI::COMM_WORLD.Send(&w,1,MPI_INT,i,0);
    }


    free(tree);
    free(procOwner);


  } else {
    MPI::COMM_WORLD.Recv(&nr_of_subtrees,1,MPI_INT,0,0,status);
    initialize(nr_of_subtrees);
    buff = new num[3*batch];
    while(true){
      num g=0;
      MPI::COMM_WORLD.Recv(&g,1,MPI_INT,0,0,status);
      
      if(g>=0){
        MPI::COMM_WORLD.Recv(buff,3*g,MPI_INT,0,0,status);
        for(int i = 0; i < 3*g; i +=3){

		if(buff[i] == -1){
		      sub_insert(trees[buff[i+1]],buff[i+2]);
		      buff[i/3]=0;   
		    } else {
			  buff[i/3]=sub_query(trees[buff[i]],buff[i+1],buff[i+2]);	
		    }
		}
		MPI::COMM_WORLD.Send(buff,g,MPI_INT,0,0);
      } else {
      
        break;
      }
    }
    free(buff);
  }
}

int main(int argc, char** argv) {
  MPI::Init();
  world_rank=MPI::COMM_WORLD.Get_rank();
  world_size=MPI::COMM_WORLD.Get_size();
  
 
  if(argc != 5){
    if(world_rank == 0)
      printf("Usage: \n./a.out nr_of_proc range lvl_of_root batch_size\n");
    return -1;
  }
  proc = atoi(argv[1]);

  range = atol(argv[2]);
  int w = 1; 
  while(w < range) w *= 2;
  range = w;

  lvl_of_root = atoi(argv[3]);

  batch = atoi(argv[4]);
  
  size = 1;
  for(int i = 0; i < lvl_of_root; i ++)size *= 2;
  shift = size/2;
  
  
  sub_size = 1;
  sub_range = range/shift;  

  while(sub_size<sub_range*2) sub_size *= 2;

  sub_shift = sub_size/2;
  

  double t0 = MPI_Wtime();
  double t1 = 0; 
  parallel();     
  t1 = MPI_Wtime();
//  printf("Proc: %d Range: %d Operations: %d Time: %f\n",proc,range,n, t1-t0);
  MPI_Finalize();
}


/////////////////////////////////////////////////////////////////////////////////////
// SUB PROCEDURES
/////////////////////////////////////////////////////////////////////////////////////
void sub_insert(num *sub_tree, num x){
  num v = sub_shift+x;
  sub_tree[v]++;
  while(v!=1){
    v/=2;
    sub_tree[v] = sub_tree[2*v] + sub_tree[2*v+1];
  }

}

num sub_query(num* sub_tree, num a, num b){
  if(b<a||a<0||b<0)return 0;
  num va = sub_shift+a;
  num vb = sub_shift+b;
  num wyn = sub_tree[va];
  if(va!= vb) wyn+=sub_tree[vb];
  while(va/2!=vb/2){
    if(va%2==0) wyn += sub_tree[va+1];
    if(vb%2==1) wyn += sub_tree[vb-1];
    va/=2; vb/=2;
  }
  return wyn;
}

