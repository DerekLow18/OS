#include <stdio.h>

void swap(int *x, int *y){
  tmp = (int *) malloc(sizeof(int));
	tmp = *x;
	*x = *y;
	*y = tmp;
}


int main(){
	int x,y;
	x = 1;
	y = 2;
	printf("%d\t%d\n",x,y);
	swap(&x,&y);
	printf("%d\t%d\n",x,y);
}
