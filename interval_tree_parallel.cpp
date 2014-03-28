#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int proc;
int range;
int world_rank;
int world_size;
int n;
bool test = false;

int* tree;
int size;
void insert(int x){
  int v = size+x;
  tree[v]++;
  while(v!=1){
    v/=2;
    tree[v] = tree[2*v] + tree[2*v+1];
  }
}

int query(int a, int b){
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
void sequential(){
  scanf("%d",&n);
  if(test){
    tree = new int[n];
    for(int i = 0; i < n; i++)tree[i]=0;
    int a,b;
    for(int i = 0; i < n; i++){
      scanf("%d %d",&a,&b);
      if(a == -1){
        tree[b]++;
      } else {
        int sum = 0;
        for(int j = a; j <= b; j++)
          sum+=tree[j];
        printf("%d\n",sum);
      }
    }
    free(tree);     
  } else {
    size = 1;
    int a,b;
    while(size<n) size = size * 2;
    tree = new int[2 * size - 1];
    for(int i = 0; i < 2 * size -1; i++)
      tree[i]=0;
    for(int i = 0; i < n; i++){
      scanf("%d %d",&a,&b);
      if(a == -1){
        insert(b); 
      } else {
        int sum = query(a,b);
        printf("%d\n",sum);
      }
    }

    free(tree);
  }
}

int* min_nodes;
const int MAIN_TREE_LVL = 10;
const int MAIN_TREEE_SIZE = 1024*2-1;
void parallel(){
        //    MPI::COMM_WORLD.Send(buff, s, MPI_BYTE, j, 0);
        //  MPI::COMM_WORLD.Recv(recv, s, MPI_BYTE, 0, 0, status);
  if ( world_rank == 0){
    min_nodes = new int[proc];
    int r = range / proc;
    min_nodes[0]=0;
    for(int i =1; i < proc; i ++)
      min_nodes[i] = r + min_nodes[i-1];
  
    for(int i = 0; i < proc; i ++)
      printf("%d ", min_nodes[i]);
    printf("\n");
   
    tree = new int[MAIN_TREE_SIZE];
    for(int i = 0; i < MAIN_TREE_SIZE; i ++)
      tree[i] = 0;
  
    for(int i = 0; i < n; i++){
      scanf("%d %d",&a,&b);
      if(a == -1){
        insert(b);
      } else {
        int sum = query(a,b);
        printf("%d\n",sum);
      }
    }

    

    free(min_nodes);
    free(tree);

  } else {


  }
}

int main(int argc, char** argv) {
  MPI::Init();
  world_rank=MPI::COMM_WORLD.Get_rank();
  world_size=MPI::COMM_WORLD.Get_size();
  
  char isSequential = argv[1][0];
  proc = atoi(argv[2]);
  range = atoi(argv[3]);
  double t0 = MPI_Wtime();
  double t1 = 0; 
  if (isSequential == 'T'){
    sequential();
  } else {
    parallel();     
  }
  t1 = MPI_Wtime();

//  printf("Proc: %d Range: %d Operations: %d Time: %f\n",proc,range,n, t1-t0);
  
  MPI_Finalize();
}
