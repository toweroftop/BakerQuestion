/*
	Baker Qustion
	Time:2021-06-29
	baker.c
*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pthread.h>
#include<string.h>
#include<signal.h>

#define CAKE 0
#define BREAD 1
#define MAXSIZE 50
#define DEBUG 0


//SIGUSR1 CAKE
//SIGUSR2 BREAD
//SIGALRM Close Shop

struct buynode {
    int num; //the order number
	int kind;
	int next;
};

struct twoV {
	int int1;
	int int2;
};

struct buynode *Sary = NULL;

pthread_cond_t pCond;
pthread_mutex_t pMutex, psig, pbaker, pnode;

int breadn = 10, caken = 10;
int orderCake = 0, orderBread = 0;
int head = -1, cur = -1;
int DB = 1;

void sigu1(int sig);
void sigu2(int sig);
void sigQ(int sig);
int addnode(struct buynode *ary, int *headq, int *curq, int ordernum, int kind);
int rmnode(struct buynode *ary, int *headq, int *curq, struct twoV *ret);
void *baker(void *data);

//-1 MutexCond -2 thread
int main(int argn, char *argc[])
{
	pthread_t *pn, pConsumer;
	key_t key;
	int i, msgid, bval, threadN = 5;

	//Output PID
	printf("PID: %d\n\n\n", getpid());

	//=====================================
	//Input args
	printf("Input bread number\n");
	scanf("%d", &breadn);
	printf("Input cake number\n");
	scanf("%d", &caken);
	printf("Input baker number\n");
	scanf("%d", &threadN);
	//=====================================

	//if (argn > 1)
	//{
	//	if (strcmp(argc[1], "--debug") == 0)
	//		DB == 1;
	//	else if (strcmp(argc[1], "--Debug") == 0)
	//		DB == 1;
	//	else if (strcmp(argc[1], "-debug") == 0)
	//		DB == 1;
	//	else if (strcmp(argc[1], "-Debug") == 0)
	//		DB == 1;
	//}

#ifndef DEBUG
	if (DB) printf("main init head %d cur: %d\n", head, cur);
#endif

	//Init Value
	//=====================================
	//COND
	if ((bval = pthread_cond_init(&pCond, NULL)) != 0)
	{
		printf("Cond Failure\n");
		return -1;
	}
	//MUTEX
	bval = 0;
	bval = pthread_mutex_init(&pMutex, NULL);
	bval += pthread_mutex_init(&psig, NULL);
	bval += pthread_mutex_init(&pbaker, NULL);
	bval += pthread_mutex_init(&pnode, NULL);
	if ((bval) != 0)
	{
		printf("Mutex Failure\n");
		return -1;
	}

	//buynode Init
	Sary = malloc(sizeof(struct buynode) * MAXSIZE);

	//SIGNAL REGISTER
	signal(SIGUSR1, sigu1); //CAKE
	signal(SIGUSR2, sigu2); //BREAD
	signal(SIGALRM, sigQ); //Close Shop

	//======================================
	//PTHREAD
	//Init
	pn = malloc(sizeof(pthread_t) * threadN);
	//Create
	for (i = 0; i < threadN; i++)
	{
		pthread_mutex_lock(&pMutex);
		int j = i + 1;
		bval = pthread_create(&pn[i], NULL, baker, &j);
		if (bval != 0)
		{
			printf("thread %d create failure\n", i);
			return -2;
		}

	}

	for (i = 0; i < threadN; i++)
	{
		bval = pthread_join(pn[i], NULL);
		if (bval != 0)
		{
			printf("thread %d join failure\n", i);
			return -2;
		}

	}

	return 0;
}

void *baker(void *data)
{
	int sig = *((int *)data);
	printf("Baker %d is ready\n", sig);
	pthread_mutex_unlock(&pMutex);

	struct twoV *getv;
	getv = malloc(sizeof(struct twoV));
	int recV = 0;
	getv->int1 = -1;
	getv->int2 = -1;
	//int1 is kind int2 is num
	while (1)
	{
		pthread_mutex_lock(&pbaker);
		pthread_cond_wait(&pCond, &pbaker);
		recV = rmnode(Sary, &head, &cur, getv);
#ifndef DEBUG
		if (DB) printf("baker head %d cur: %d\n", head, cur);
		if (DB) printf("recV: %d\n", recV);
#endif
		if (recV == -1)
		{
			printf("Baker %d No order\n", sig);
			pthread_mutex_unlock(&pbaker);
			continue;
		}
		if (breadn <= 0 && caken <= 0)
		{
			printf("Baker %d Sell out\n", sig);
			pthread_exit(NULL);
		}
		if (getv != NULL)
		{
			if (getv->int1 == CAKE)
			{
				if (caken > 0)
				{
					caken--;
					printf("Order %d, baker %d sell a cake, left cake number: %d\n", getv->int2, sig, caken);
				}
				else
				{
					printf("Baker %d reported cake has been sold out\n", sig);
				}
			}
			else if (getv->int1 == BREAD)
			{
				if (breadn > 0)
				{
					breadn--;
					printf("Order %d, baker %d sell a bread, left bread number: %d\n", getv->int2, sig, breadn);
				}
				else
				{
					printf("Baker %d reported bread has been sold out\n", sig);
				}
			}
#ifndef DEBUG
			else
			{
				printf("getv kind is unknown getvKind: %d getvNum: %d\n", getv->int1, getv->int2);
			}
#endif
		}
		else
		{
			printf("Baker %d reported getv is NULL\n", sig);
		}
		sleep(0.2);
		pthread_mutex_unlock(&pbaker);
	}

}

void sigu1(int sig)
{
	//SIGUSR1 CAKE
	//SIGUSR2 BREAD
	//SIGINT Close Shop
	pthread_mutex_lock(&psig);
	orderCake++;
	printf("Receive a order Cake, current Order num: %d\n", orderCake);
	addnode(Sary, &head, &cur, orderCake, CAKE);
#ifndef DEBUG
	if (DB) printf("sigu1 head %d cur: %d\n", head, cur);
#endif
	pthread_cond_signal(&pCond);
	pthread_mutex_unlock(&psig);
}

void sigu2(int sig)
{
	//SIGUSR1 CAKE
	//SIGUSR2 BREAD
	//SIGINT Close Shop
	pthread_mutex_lock(&psig);
	orderBread++;
	printf("Receive a order Bread, current Order num: %d\n", orderBread);
	addnode(Sary, &head, &cur, orderBread, BREAD);
#ifndef DEBUG
	if (DB) printf("sigu2 head %d cur: %d\n", head, cur);
#endif
	pthread_cond_signal(&pCond);
	pthread_mutex_unlock(&psig);
}

void sigQ(int sig)
{
	//SIGUSR1 CAKE
	//SIGUSR2 BREAD
	//SIGINT Close Shop
	pthread_mutex_lock(&psig);
	printf("Close Shop\n");
	sleep(0.5);
	exit(0);
	pthread_mutex_unlock(&psig);
}

int addnode(struct buynode *ary, int *headq, int *curq, int ordernum, int kind)
{
	//return
	// 1 set head
	// 2 set current
	pthread_mutex_lock(&pnode);
	if (*(curq) == -1)
	{
#ifndef DEBUG
		if (DB) printf("addnode cur-1  1 head %d cur: %d\n", *(headq), *(curq));
#endif
		if (*(headq) != -1)
			*(curq) = *(headq);
		else
		{
			*(curq) = 0;
			*(headq) = 0;
		}
		ary[*(curq)].num = ordernum;
		ary[*(curq)].kind = kind;
		ary[*(curq)].next = -1;
#ifndef DEBUG
		if (DB) printf("addnode cur-1  2 head %d cur: %d\n", *(headq), *(curq));
		if (DB) printf("addnode cur-1 arg[%d].num=%d arg[%d].kind=%d arg[%d].next=%d\n", *(curq), ary[*(curq)].num, *(curq), ary[*(curq)].kind, *(curq), ary[*(curq)].next);
#endif
		pthread_mutex_unlock(&pnode);
		return 1;
	}
	else if (*(headq) == -1)
	{
		*(headq) = 0;
		ary[0].num = ordernum;
		ary[0].next = -1;
		ary[0].kind = kind;
		*(curq) = 0;
#ifndef DEBUG
		if (DB) printf("addnode head-1 head %d cur: %d\n", *(headq), *(curq));
		if (DB) printf("addnode head-1 arg[%d].num=%d arg[%d].kind=%d arg[%d].next=%d\n", *(curq), ary[*(curq)].num, *(curq), ary[*(curq)].kind, *(curq), ary[*(curq)].next);
#endif
		pthread_mutex_unlock(&pnode);
	}
	else
	{
		if (*(curq) == MAXSIZE - 1)
		{
			*(curq) = 0;
			ary[MAXSIZE - 1].next = 0;
		}
		else
		{
			ary[*(curq)].next = *(curq) + 1;
			*(curq) += 1;
		}
		ary[*(curq)].num = ordernum;
		ary[*(curq)].kind = kind;
		ary[*(curq)].next = -1;
#ifndef DEBUG
		if (DB) printf("addnode else head %d cur: %d\n", *(headq), *(curq));
		if (DB) printf("addnode else arg[%d].num=%d arg[%d].kind=%d arg[%d].next=%d\n", *(curq), ary[*(curq)].num, *(curq), ary[*(curq)].kind, *(curq), ary[*(curq)].next);
#endif
		pthread_mutex_unlock(&pnode);
		return 2;
	}
}

//NULL is empty queue int1 is kind int2 is num
int rmnode(struct buynode *ary, int *headq, int *curq, struct twoV *ret)
{
	pthread_mutex_lock(&pnode);
	//struct buynode *t, *t2;
	if (*(headq) == -1)
	{
#ifndef DEBUG
		if (DB) printf("rmnode head-1 %d cur: %d\n", *(headq), *(curq));
#endif
		pthread_mutex_unlock(&pnode);
		return -1;
	}
	else if (ary[*(headq)].next == -1)
	{
		//ret = malloc(sizeof(struct twoV));
		ret->int1 = ary[*(headq)].kind;
		ret->int2 = ary[*(headq)].num;
		*(headq) = -1;
		*(curq) = -1;
#ifndef DEBUG
		if (DB) printf("rmnode ary[head].next-1 %d cur: %d\n", *(headq), *(curq));
#endif
		pthread_mutex_unlock(&pnode);
		return ret->int1;
	}
	else
	{
		//ret = malloc(sizeof(struct twoV));
		ret->int1 = ary[*(headq)].kind;
		ret->int2 = ary[*(headq)].num;
		*(headq) = ary[*(headq)].next;
#ifndef DEBUG
		if (DB) printf("rmnode else head: %d cur: %d\n", *(headq), *(curq));
		if (DB) printf("rmnode else arg[%d].num=%d arg[%d].kind=%d arg[%d].next=%d\n", *(headq), ary[*(headq)].num, *(headq), ary[*(curq)].kind, *(headq), ary[*(curq)].next);
#endif
		pthread_mutex_unlock(&pnode);
		return ret->int1;
	}
}