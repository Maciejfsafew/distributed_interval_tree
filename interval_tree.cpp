#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int proc;
int range;
int iterations;
int world_rank;
int world_size;


void sequential(){

}

void parallel(){
        //    MPI::COMM_WORLD.Send(buff, s, MPI_BYTE, j, 0);
        //  MPI::COMM_WORLD.Recv(recv, s, MPI_BYTE, 0, 0, status);
 
}

int main(int argc, char** argv) {
  MPI::Init();
  world_rank=MPI::COMM_WORLD.Get_rank();
  world_size=MPI::COMM_WORLD.Get_size();
  
  char isSequential = argv[1][0];
  proc = atoi(argv[2]);
  range = atoi(argv[3]);
  iterations = atoi(argv[4]); 
  double t0 = MPI_Wtime();
  double t1 = 0; 
  if (isSequential == 'T'){
    sequential();
  } else {
    parallel();     
  }
  t1 = MPI_Wtime();

  printf("Proc: %d Range: %d Operations: %d Time: %f\n",proc,range,iterations, t1-t0);
  
  MPI_Finalize();
}
