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
long long sub_range;

int batch;
// proc nr
int world_rank;
int world_size;

// lvls in root tree
int lvl_of_root;

MPI::Status status;
// range of single sub tree
long long shift;
long long sub_shift;
long long root_tree_size;
long long size;
long long sub_size;
////////////////////////////////////////////////////////////////////////////////////////
// SUBNODES
////////////////////////////////////////////////////////////////////////////////////////
// nr of sub trees in subnode
int nr_of_subtrees;
// array of subtrees
long long** trees;

void sub_insert(long long * tree, long long x);
long long sub_query(long long* tree, long long a, long long b);

//ROOT VARIABLES
long long* tree;
long long** queries;
long long** queries_backup;
int* queries_count;
int* queries_count_backup;
long long* all_requests;
long long* all_requests_backup;
int total_count = 0;
int total_count_backup = 0;
int** requests_queries;
int** requests_queries_backup;

int* procOwner;
int* countNr;
int* procCounts;
long long n;
long long *buff;
const int BUFF_SIZE = 1000;
int max_buff = 0;

void initialize(int count){
  trees = new long long *[count];
  sub_size = 1;
  while(sub_size<sub_range) sub_size *= 2;
  for(int i = 0; i < count; i ++){
    trees[i] = new long long[2*sub_size];
    for(int j = 0; j < 2*sub_size; j ++)trees[i][j] = 0;
#ifdef _DEBUG
    printf("tree pr %d nr %d size %lld\t",world_rank,i,sub_size*2); for(int j = 0; j < 2*sub_size; j++)printf("%lld ",trees[i][j]); printf("\n");
#endif
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
#ifdef _DEBUG
  printf("tree insert x(%lld) ",x); for(int i = 0; i < size; i++)printf("%lld ",tree[i]); printf("\n");
  printf("sub insert owner %d count %d sub val %lld q_count %d\n", procOwner[v], countNr[v],x-begin,queries_count[procOwner[v]]);
#endif
//  buff[0] = -1;
//  buff[1] = countNr[v];
//  buff[2] = x - begin;
//  MPI::COMM_WORLD.Send(buff,3,MPI_LONG_LONG_INT,procOwner[v],0);


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
  //return res;  
#ifdef _DEBUG
  printf("lower x %lld res %lld\n",x,res);
  printf("LOWER sub query v %lld owner %d countNr %d end %lld queries_count %d \n",v,procOwner[v],countNr[v],x-1-begin,queries_count[procOwner[v]]);
#endif

queries[procOwner[v]][3*queries_count[procOwner[v]]]=countNr[v];
queries[procOwner[v]][3*queries_count[procOwner[v]]+1]=0;
queries[procOwner[v]][3*queries_count[procOwner[v]]+2]=x-1-begin;
requests_queries[procOwner[v]][queries_count[procOwner[v]]]=total_count;
queries_count[procOwner[v]]++;

max_buff = max(max_buff,queries_count[procOwner[v]]);
all_requests[total_count]=res;
total_count++;

//  buff[0] = countNr[v];
//  buff[1] = 0;
//  buff[2] = x -1 - begin;
//  MPI::COMM_WORLD.Send(buff,3,MPI_LONG_LONG_INT,procOwner[v],0);
//  long long b = 0;
//  MPI::COMM_WORLD.Recv(&b,1,MPI_LONG_LONG_INT,procOwner[v],0,status);
//  return res + b;
  //return res + query(trees[procOwner[v]][countNr[v]],0,x-1-begin);
}

void query_root(long long a, long long b){
  lower(b+1);
  lower(a);
}
void send(){
  for(int i = 0; i < proc; i ++){
    int w = queries_count_backup[i];
    if(w>0){
  	  MPI::COMM_WORLD.Recv(queries_backup[i],w,MPI_LONG_LONG_INT,i,0,status);
    }
  }
  
  for(int i = 0; i < proc; i ++){
    int w = queries_count[i];
    if(w>0){
  	  MPI::COMM_WORLD.Send(&w,1,MPI_INT,i,0);
  	  MPI::COMM_WORLD.Send(queries[i],3*w,MPI_LONG_LONG_INT,i,0);
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
	  printf("%lld\n",all_requests_backup[i]-all_requests_backup[i+1]);
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
    tree = new long long[size];
    for(int i = 0; i < size; i++) tree[i] = 0;
    procOwner = new int[shift];
    countNr = new int[shift];
    procCounts = new int[proc];
    for(int i = 0; i < proc; i++) procCounts[i] = 0;
    for(int i = 0; i < shift; i++){
      procOwner[i]=1+rand()%(proc-1);
      procCounts[procOwner[i]]++;
      countNr[i] = procCounts[procOwner[i]]-1;
    }
#ifdef _DEBUG

    printf("proc Owner "); for(int i = 0; i < shift; i++)printf("%d ",procOwner[i]); printf("\n");
    printf("count Nr "); for(int i = 0; i < shift; i++)printf("%d ",countNr[i]); printf("\n");
    printf("proc Counts "); for(int i = 0; i < proc; i++)printf("%d ",procCounts[i]); printf("\n");
#endif
    for(int i = 1; i < proc; i++){
      MPI::COMM_WORLD.Send(&procCounts[i],1,MPI_INT,i,0);
    } 
    queries = new long long*[proc];
    queries_backup = new long long*[proc];
    requests_queries = new int*[proc];
    requests_queries_backup = new int*[proc];
    queries_count = new int[proc];
    queries_count_backup = new int[proc];
    all_requests = new long long[(proc-1)*batch];
    all_requests_backup = new long long[(proc-1)*batch];
    for(int i = 0; i < proc; i++){
      queries_count[i] = 0;
      queries_count_backup[i] = 0;
	  queries[i] = new long long[3*batch];
	  queries_backup[i] = new long long[3*batch];
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
    
    long long w = -1;
    for(int i = 1; i < proc; i++){
      MPI::COMM_WORLD.Send(&w,1,MPI_INT,i,0);
    }


    free(tree);
    free(procOwner);


  } else {
    MPI::COMM_WORLD.Recv(&nr_of_subtrees,1,MPI_INT,0,0,status);
#ifdef _DEBUG
    printf("Init proc %d nr_of_subtrees %d\n",world_rank,nr_of_subtrees);
#endif
    initialize(nr_of_subtrees);
#ifdef _DEBUG
    printf("Initialized proc %d\n",world_rank);
#endif
    buff = new long long[3*batch];
    while(true){
      int g=0;
#ifdef _DEBUG
      printf("Waiting for size proc %d count %d\n",world_rank,g);
#endif
      MPI::COMM_WORLD.Recv(&g,1,MPI_INT,0,0,status);
#ifdef _DEBUG
      printf("Received proc %d count %d\n",world_rank,g);
#endif

      
      if(g>=0){
        MPI::COMM_WORLD.Recv(buff,3*g,MPI_LONG_LONG_INT,0,0,status);
#ifdef _DEBUG
      printf("Inserted proc %d values %lld %lld %lld\n",world_rank,buff[0],buff[1],buff[2]);
#endif
        for(int i = 0; i < 3*g; i +=3){

#ifdef _DEBUG
      printf("Got values %lld %lld %lld\n",buff[i],buff[i+1],buff[i+2]);
#endif
			if(buff[i] == -1){
		      sub_insert(trees[buff[i+1]],buff[i+2]);
		      buff[i/3]=0;   
		    } else {
			  buff[i/3]=sub_query(trees[buff[i]],buff[i+1],buff[i+2]);	
		    }
		}
		MPI::COMM_WORLD.Send(buff,g,MPI_LONG_LONG_INT,0,0);
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
#ifdef _DEBUG
  if(world_rank == 0)
    printf("proc %d\n",proc);
#endif

  range = atoi(argv[2]);
  int w = 1; 
  while(w < range) w *= 2;
  range = w;
#ifdef _DEBUG
  if(world_rank == 0)  
    printf("range %lld\n",range);
#endif

  lvl_of_root = atoi(argv[3]);
#ifdef _DEBUG
  if(world_rank == 0)
    printf("lvl_of_root %d\n", lvl_of_root);
#endif

  batch = atoi(argv[4]);
  
  size = 1;
  for(int i = 0; i < lvl_of_root; i ++)size *= 2;
  shift = size/2;
  
  
  sub_size = 1;
  sub_range = range/shift;  

  while(sub_size<sub_range*2) sub_size *= 2;

  sub_shift = sub_size/2;
#ifdef _DEBUG
  if(world_rank == 0)
    printf("size %lld shift %lld sub_size %lld sub_range %lld sub_shift %lld\n",size,shift,sub_size,sub_range,sub_shift);
#endif
  

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
void sub_insert(long long *sub_tree, long long x){
#ifdef _DEBUG
  printf("SUB_INSERT proc %d x %lld",world_rank,x); 
#endif
  long long v = sub_shift+x;
  sub_tree[v]++;
  while(v!=1){
    v/=2;
    sub_tree[v] = sub_tree[2*v] + sub_tree[2*v+1];
  }
#ifdef _DEBUG
  printf("sub tree insert "); for(int i = 0; i < 2*sub_size; i++)printf("%lld ",sub_tree[i]); printf("\n");
#endif

}

long long sub_query(long long* sub_tree, long long a, long long b){
#ifdef _DEBUG
  printf("SubQuery %lld %lld\n",a,b);
#endif
  if(b<a||a<0||b<0)return 0;
  long long va = sub_shift+a;
  long long vb = sub_shift+b;
  long long wyn = sub_tree[va];
  if(va!= vb) wyn+=sub_tree[vb];
  while(va/2!=vb/2){
    if(va%2==0) wyn += sub_tree[va+1];
    if(vb%2==1) wyn += sub_tree[vb-1];
    va/=2; vb/=2;
  }
#ifdef _DEBUG
  printf("subquery a %lld b %lld wyn %lld\n", a,b,wyn);
#endif
  return wyn;
}

