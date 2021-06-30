/*
Baker Qustion
Time:2021-06-28
consumer.c
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pthread.h>
#include<string.h>

#define CAKE 0
#define BREAD 1

//SIGUSR1 CAKE
//SIGUSR2 BREAD
//SIGALRM Close Shop

int main(int argn, char *argc[])
{
	int i, inn = -1, pr;
	printf("Input PID\n");
	scanf("%d", &pr);
	printf("Input number 0 is cake   1 is bread   2 is close\n");
    while(1)
    {
		scanf("%d", &inn);
		if (inn == 0)
		{
			kill(pr, SIGUSR1);
		}
		else if (inn == 1)
		{
			kill(pr, SIGUSR2);
		}
		else if(inn == 2)
		{
			kill(pr, SIGALRM);
			return 0;
		}
		else
		{
			printf("Unknown Signal\n");
		}
    }
	return 0;
}
