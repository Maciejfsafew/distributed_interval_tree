#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ctime>
#include <algorithm>
using namespace std;
//Input generator

//parameters:
//-number of input values
//-range of values


//output
//ask for interval => begin end
//insert value => -1 index
int main(int argc, char** argv) {

  int numbers = atoi(argv[1]);
  int range = atoi(argv[2]);
  srand(time(NULL));
  printf("%d\n",numbers);
  for(int i = 0; i < numbers; i++ ){
    int insert = rand()%2;
    if(insert == 1){
      printf("%d %d\n",-1,rand()%range);
      continue;
    }
    int begin = rand()%range;
    int end = rand()%range;
    if (begin>end)
      swap(begin,end);
    printf("%d %d\n",begin,end);
  }
  return 0;
}
