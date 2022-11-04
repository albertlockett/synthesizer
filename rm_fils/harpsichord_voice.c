#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[]){

	int i = 0;
	FILE * f_ptr;
	f_ptr = fopen("process_output","w");
	fprintf(f_ptr, "Harpsichord starting\n");
	fclose(f_ptr);	


	while(1){
		f_ptr = fopen("process_output","a");
		if(f_ptr == NULL){
			fprintf(stderr, "opening file failed\n");
			exit(1);
		}	
		fprintf(f_ptr, "%d - Harpsichord\n", i++);
		fclose(f_ptr);
		sleep(1);
	}
}
