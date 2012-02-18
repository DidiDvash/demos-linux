#include <stdio.h> // for printf(3)
#include <stdlib.h> // for rand(3), srand(3)

int main() {
	int seed;
	int i;
	printf("give me a seed :");
	int ret=scanf("%d",&seed);
	if(ret!=1) {
		perror("scanf");
		return -1;
	}
	srand(seed);
	for(i=0;i<10;i++) {
		printf("rand gave me %d\n",rand());
	}
	return 0;
}