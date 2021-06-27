/* 
Baker Qustion
Time:2021-06-27
Baker
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pthread.h>
#include<string.h>

void *p1FUN(void *data);
void *p2FUN(void *data);

pthread_cond_t pCond;
pthread_mutex_t pMutex;


int main(int argn, char *argc[])
{
	pthread_t p1, p2;
	pthread_attr pAttr;
	int backv;

	//Init pthread Attribute
	if ((backv = pthread_attr_init(&pAttr)) != 0)
	{
		printf("pthread attr init failure\n");
		return -1;
	}

	//Init Cond Mutex
	if ((backv = pthread_cond_init(pCond, NULL)) != 0)
	{
		printf("Pthread Cond Init Failure.n\");
		perror("Cond Init Failure\n");
		return -1;
	}
	if ((backv = pthread_mutex_init(pMutex, NULL)) != 0)
	{
		printf("Pthread Mutex Init Failure.\n");
		perror("Mutex Init Failure\n");
		return -1;
	}

	//POSIX thread Init
	if ((pthread_create(&p1, &pAttr, p1FUN, NULL)) != 0)
	{
		printf("thread 1 create failure\n");
		return -2;
	}
	if ((pthread_create(&p2, &pAttr, p2FUN, NULL)) != 0)
	{
		printf("thread 2 create failure\n");
		return -2;
	}

	pthread_mutex_destory(&pMutex);
	pthread_cond_destory(&pCond);
	return 0;
}

void *p1FUN(void *data)
{
	int num = 0;
}

void *p2FUN(void *data)
{
	int num = 0;
}