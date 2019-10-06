#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int main(){
	/*
	int x=1;

	fork();
	fork();
	
	wait(NULL);

	printf("%d\n",getpid());
	printf("%d\n",x);
	*/

	int x;
	x=0;

	if(fork()==0){
		x=1;
	}
	else{
		wait(NULL);
	}

	printf("pid = %d, x = %d", getpid(),x);

	return 0;
}