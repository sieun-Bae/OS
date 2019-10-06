#include <stdio.h>
#include <pthread.h>

void* plus(void* lol)
{
	int aa = *((int*)lol); 
	int bb = *((int*)lol+1); 

	printf("plus = %d\n", aa+bb);
	return NULL;
}

void* minus(void* lol)
{
	int aa = *((int*)lol); 
	int bb = *((int*)lol+1); 

	printf("minus = %d\n", aa-bb);
	return NULL;
}
void* multiply(void* lol)
{
	int aa = *((int*)lol); 
	int bb = *((int*)lol+1); 

	printf("multiply = %d\n", aa*bb);
	return NULL;
}
void* div(void* lol)
{
	int aa = *((int*)lol); 
	int bb = *((int*)lol+1); 

	printf("div = %d\n", aa/bb);
	return NULL;
}

int main()
{
	pthread_t thread[4];
	int tid[4];

	int lol[2] = {10,5};

	pthread_create(&thread[0], NULL, plus, (void*)lol);
	pthread_create(&thread[1], NULL, minus, (void*)lol);
	pthread_create(&thread[2], NULL, multiply, (void*)lol);
	pthread_create(&thread[3], NULL, div, (void*)lol);

	for(int i=0; i<4; i++)
	{
		pthread_join(thread[i], NULL);
	}

	return 0;

}