#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int range=1000;
int n;

int* tree;

void sequential(){
  scanf("%d",&n);
  tree = new int[range];
  for(int i = 0; i < range; i++)tree[i]=0;
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
  
}


int main(int argc, char** argv) {
  sequential();
  return 0;
}
