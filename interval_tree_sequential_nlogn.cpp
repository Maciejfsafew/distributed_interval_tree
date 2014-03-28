#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int range;
int n;

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
  size = 1;
  int a,b;
  while(size<range) size = size * 2;
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

int main(int argc, char** argv) {
  range = atoi(argv[1]);
  sequential();
}
