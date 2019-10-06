#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<unistd.h>

int main()
{
	pid_t pid;2

	pid = fork();

	if(pid<0)
	{
		fprintf(stderr, "Fork Failed");
	}
	else if (pid == 0)
	{
		execlp("/bin/ls", "ls", NULL);//=param x
	}
	else
	{
	//	wait(NULL);
		printf("Child Complete");
	}

	return 0;
}
