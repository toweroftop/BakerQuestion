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
#include<sys/msg.h>
#include<sys/ipc.h>
#include<signal.h>

#define MSGSIZE 10
#define CAKE 0
#define BREAD 1

//SIGUSR1 CAKE
//SIGUSR2 BREAD
//SIGALRM Close Shop

struct buynode {
    int num; //the order number
	int kind;
    struct buynode *next;
};

struct twoV {
	int int1;
	int int2;
};

struct msgstr {
    long MType;
    char p[MSGSIZE];
};

struct msgstr *msg;
struct buynode *Shead = NULL, *Sstep = NULL;

pthread_cond_t pCond;
pthread_mutex_t pMutex, psig, pbaker, pnode;

int breadn = 10, caken = 10;
int orderCake = 0, orderBread = 0;

void sigu1(int sig);
void sigu2(int sig);
void sigQ(int sig);
int addnode(struct buynode *head, struct buynode *cur, int ordernum, int kind);
int rmnode(struct buynode *head, struct buynode *cur, struct twoV *ret);
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
	//scanf("%d", &breadn);
	//scanf("%d", &cake);
	//scanf("%d", &threadN);
	//=====================================

	//Init Value
	//=====================================
	//MSG
	key = (key_t)1234;	//key = ftok(argc[1], 100);
	msgid = msgget(key, IPC_CREAT | 0666);
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
	//SIGNAL REGISTER
	signal(SIGUSR1, sigu1); //CAKE
	signal(SIGUSR2, sigu2); //BREAD
	signal(SIGALRM, sigQ); //Close Shop

	//======================================
	//PThread
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
		recV = rmnode(Shead, Sstep, getv);
		if (recV == -1)
		{
			pthread_mutex_unlock(&pbaker);
			continue;
		}
		if (breadn <= 0 && caken <= 0)
		{
			printf("Sell out\n");
			pthread_exit(NULL);
		}
		if (getv != NULL)
		{
			if (getv->int1 == CAKE)
			{
				if (caken > 0)
				{
					caken--;
					printf("Order%d, sell a cake, left cake number: %d\n", getv->int2, caken);
				}
				else
				{
					printf("Cake has been sold out\n");
				}
			}
			else if (getv->int1 == BREAD)
			{
				if (breadn > 0)
				{
					breadn--;
					printf("Order%d, sell a bread, left bread number: %d\n", getv->int2, breadn);
				}
				else
				{
					printf("Bread has been sold out\n");
				}
			}
		}
		else
		{
			printf("getv is NULL\n");
		}
		sleep(0.1);
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
	addnode(Shead, Sstep, orderCake, CAKE);
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
	addnode(Shead, Sstep, orderBread, BREAD);
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

int addnode(struct buynode *head, struct buynode *cur, int ordernum, int kind)
{
	//return
	// 1 set head
	// 2 set current
	pthread_mutex_lock(&pnode);
	struct buynode *t;
	if (cur == NULL)
	{
		cur = malloc(sizeof(struct buynode));
		cur->num = ordernum;
		cur->kind = kind;
		cur->next = NULL;
		head = cur;
		pthread_mutex_unlock(&pnode);
		return 1;
	}
	else if (head == NULL)
	{
		head = malloc(sizeof(struct buynode));
		head->num = ordernum;
		head->next = NULL;
		head->kind = kind;
		cur = head;
		pthread_mutex_unlock(&pnode);
	}
	else
	{
		t = malloc(sizeof(struct buynode));
		t->num = ordernum;
		t->next = NULL;
		t->kind = kind;
		cur->next = t;
		cur = t;
		pthread_mutex_unlock(&pnode);
		return 2;
	}
}

//NULL is empty queue int1 is kind int2 is num
int rmnode(struct buynode *head, struct buynode *cur, struct twoV *ret)
{
	pthread_mutex_lock(&pnode);
	struct buynode *t, *t2;
	if (head == NULL)
	{
		pthread_mutex_unlock(&pnode);
		return -1;
	}
	else if (head->next == NULL)
	{
		ret = malloc(sizeof(struct twoV));
		ret->int1 = head->kind;
		ret->int2 = head->num;
		t = cur;
		t2 = head;
		free(t);
		free(t2);
		head = NULL;
		cur = NULL;
		pthread_mutex_unlock(&pnode);
		return ret->int1;
	}
	else
	{
		ret = malloc(sizeof(struct twoV));
		ret->int1 = head->kind;
		ret->int2 = head->num;
		t = head;
		free(t);
		head = cur;
		pthread_mutex_unlock(&pnode);
		return ret->int1;
	}
}