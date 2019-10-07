// 참고 사이트 https://unabated.tistory.com/entry/%EA%B3%B5%EC%9C%A0-%EB%A9%94%EB%AA%A8%EB%A6%AC-shared-memory

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <sys/shm.h>
#include <sys/types.h>

int main(void)
{
	int shmid;
	int pid;

	int *cal_num;
	void *shared_memory = (void*)0;

	//shmget: 커널에 공유메모리 공간을 요청하기 위해 호출하는 시스템 호출 함수

	shmid = shmget((key_t)1234, sizeof(int), 0666|IPC_CREAT);
	//key value, size of minimum shared memory, 접근권한(실행권한은 줄 수 없음)과 생성방식 
	//											(IPC_EXCL은 이미 존재할경우 return error)
	if (shmid == -1)
	{
		perror("shmget failed: ");
		exit(0);
	}

	//shmat: 공유메모리에 접근할 수 있는 int형의 '식별자'를 얻음. 
	//		 이를 shmat을 이용하여 지금의 프로세스가 공유메모리를 사용가능하도록 덧붙임 작업을 해주어야함
	//		 공유메모리를 사용하기 위해 프로세스메모리에 붙임

	shared_memory = shmat(shmid, (void*)0, 0);
	// shmget을 이용하여 얻어낸 식별자, 메모리가 붙을 주소(0인 경우 커널이 명시), SHM_RDONLY는 읽기전용/없으면 읽고쓰기 가능
	if(shared_memory == (void*)-1)
	{
		perror("shmat faild: ");
		exit(0);
	}

	cal_num = (int*)shared_memory;

	pid=fork(); //child process 생성
	if (pid == 0)
	{
		shmid = shmget((key_t)1234, sizeof(int), 0);
		'"'
		if(shmid == -1)
		{
			perror("shmget failed: ");
			exit(0);
		}
		shared_memory = shmat(shmid, (void*)0, 0666|IPC_CREAT);
		if(shared_memory==(void*)-1)
		{
			perror("shmat failed: ");
			exit(0);
		}
		cal_num = (int*)shared_memory;
		*cal_num = 1;

		while(1)
		{
			*cal_num = *cal_num + 1;
			printf("child %d\n", *cal_num);
			sleep(1);
		}
	}
	else if(pid>0)
	{
		while(1)
		{
			sleep(1);
			printf("%d\n", *cal_num);
		}
	}

	return 0;
}