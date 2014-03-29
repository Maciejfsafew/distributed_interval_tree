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

  long long numbers = atol(argv[1]);
  long long range = atol(argv[2]);
  srand(time(NULL));
  printf("%lld\n",numbers);
  for(long long i = 0; i < numbers; i++ ){
    int insert = rand()%2;
    if(insert == 1){
      long long a = rand();
      a = rand()* a;
      a = a % range;
      printf("%d %lld\n",-1,a);
      continue;
    }
    long long begin = rand();
    begin = rand() * begin;
    begin = begin % range;
    long long end = rand();
    end = end * rand();
    end = end % range;
    if (begin>end)
      swap(begin,end);
    printf("%lld %lld\n",begin,end);
  }
  return 0;
}
