#include <stdio.h>
#include <pthread.h>

void* hello(void*tid)
{
	printf("tid = %d\thelloworld\n", *(int*)tid);//context switching 문제가 생길 수 있다는 거지. 그래서 탭이 안나왔어

	return NULL;

}

int main()
{
	pthread_t thread[12];
	int tid[12];
	
	for(int i=0; i<12; i++)
	{
		tid[i]=i;
		pthread_create(&thread[i], NULL, hello, (void*)&tid[i]);
	}
	
	for(int i=0; i<12; i++)
	{
		pthread_join(thread[i], NULL);
	}

	return 0;

}