#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
int main(){

	int i;
	printf("preparing to execute the piano\n");
	for(i=3;i>0;i--){
		printf("%d\n",i);
		sleep(1);
	}
	printf("switching execution now\n");
	char *args[] = {"./piano","1",(char*) 0};
	execv("./piano",args);
	printf("coudln't do it\n");
	return 0;
}
