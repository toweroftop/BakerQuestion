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
	char *inc = (char *)malloc(sizeof(char *));
	double t = 0.1;
	printf("Input PID\n");
	scanf("%d", &pr);
	printf("Input sleeep time(double) (if is 0 will be 1)\n");
	scanf("%lf", &t);
	if (t <= 0) t = 1;
	printf("Input number 0 is cake   1 is bread   2 is close\n");
	i = 0;
	printf("Input a array\n");
	scanf("%s", inc);
    while(1)
    {
		if (inc[i] == '\0') break;
		inn = inc[i] - 48;
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
		sleep(t);
		i++;
    }
	return 0;
}
