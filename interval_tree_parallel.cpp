#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int proc;
int range;
int world_rank;
int world_size;
int n;
int lvl_of_root;

// TREES
int nr_of_trees;
int** trees[22];
int sub_size;

void insert(int *tree, int x);
int query(int* tree, int a, int b);
//ROOT

int shift;
int size;
int* tree;
int* procOwner;
int* countNr;
int* procCounts;

void initialize(int pr_id, int count, int sub_range){
  trees[pr_id] = new int *[count];
  sub_size = 1;
  while(sub_size<sub_range) sub_size *= 2;
  for(int i = 0; i < count; i ++){
    trees[pr_id][i] = new int[2*sub_size];
    for(int j = 0; j < 2*sub_size; j ++)trees[pr_id][i][j] = 0;
    printf("tree pr %d nr %d size %d\t",pr_id,i,sub_size*2); for(int j = 0; j < 2*sub_size; j++)printf("%d ",trees[pr_id][i][j]); printf("\n");
  }
}
void insertR(int x){
  int begin = 0, end = range - 1, v = 1, lvl = 1;
  tree[v]++;
  while(lvl<lvl_of_root){
    int mid = begin + (end - begin)/2;
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
  insert(trees[procOwner[v]][countNr[v]],x);
}
int lower(int x){
  int begin = 0, end = range -1, v = 1, lvl = 1;
  int res = 0;
  while(lvl<lvl_of_root){
    int mid = begin + (end - begin)/2;
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

  return res + query(trees[procOwner[v]][countNr[v]],begin,x-1);
}

int queryR(int a, int b){
  return lower(b+1)-lower(a);
}

void parallel(){
        //    MPI::COMM_WORLD.Send(buff, s, MPI_BYTE, j, 0);
        //  MPI::COMM_WORLD.Recv(recv, s, MPI_BYTE, 0, 0, status);
  if ( world_rank == 0){
    size = 1;
    for(int i = 0; i < lvl_of_root; i ++)size *= 2;
    shift = size/2;   
    printf("size %d shift %d \n", size, shift);
    tree = new int[size];
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
    printf("proc Owner "); for(int i = 0; i < shift; i++)printf("%d ",procOwner[i]); printf("\n");
    printf("count Nr "); for(int i = 0; i < shift; i++)printf("%d ",countNr[i]); printf("\n");
    printf("proc Counts "); for(int i = 0; i < proc; i++)printf("%d ",procCounts[i]); printf("\n");
    for(int i = 1; i < proc; i++){
      initialize(i,procCounts[i],range/shift);
    } 
    
    scanf("%d",&n);
    int a,b; 
    for(int i = 0; i < n; i++){
      scanf("%d %d",&a,&b);
      if(a == -1){
        insertR(b);
      } else {
        int sum = queryR(a,b);
        printf("%d\n",sum);
      }
    }

    

    free(tree);
    free(procOwner);


  } else {
    // NOT ROOT

  }
}

int main(int argc, char** argv) {
  MPI::Init();
  world_rank=MPI::COMM_WORLD.Get_rank();
  world_size=MPI::COMM_WORLD.Get_size();
  proc = atoi(argv[1]);
  printf("proc %d\n",proc);
  range = atoi(argv[2]);
  int w = 1; 
  while(w < range) w *= 2;
  range = w;
  printf("range %d\n",range);
  lvl_of_root = atoi(argv[3]);
  printf("lvl_of_root %d\n", lvl_of_root);
  double t0 = MPI_Wtime();
  double t1 = 0; 
  parallel();     
  t1 = MPI_Wtime();
//  printf("Proc: %d Range: %d Operations: %d Time: %f\n",proc,range,n, t1-t0);
  MPI_Finalize();
}

void insert(int *tree, int x){
  int v = size+x;
  tree[v]++;
  while(v!=1){
    v/=2;
    tree[v] = tree[2*v] + tree[2*v+1];
  }
}

int query(int* tree, int a, int b){
  int va = size+a;
  int vb = size+b;
  int wyn = tree[va];
  if(va!= vb) wyn+=tree[vb];
  while(va/2!=vb/2){
    if(va%2==0) wyn += tree[va+1];
    if(vb%2==1) wyn += tree[vb-1];
    va/=2; vb/=2;
  }
  return wyn;
}

