/*
	Baker Qustion
	Time:2021-06-27
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

#define MSGSIZE 10
#define CAKE 0
#define BREAD 1

struct buynode {
    int num; //the sign of the thing that consumer wants to buy.
	int kind;
    struct buynode *next;
};

struct msgstr {
    long MType;
    char p[MSGSIZE];
};

struct msgstr *msg;
struct buynode *Shead = NULL, *Sstep = NULL;

void *baker(void *data);
void *consumer(void *data);
void *BakerRest(void *data);

int addnode(struct buynode *qhead, struct buynode *cur, int curnum, int kind);
int rmnode(struct buynode *qhead, struct buynode *cur);

pthread_cond_t *pCond, pmCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t *pMutex, pmMutex = PTHREAD_MUTEX_INITIALIZER, nodeMutex = PTHREAD_MUTEX_INITIALIZER;
int n = 0, caken = 1, breadn = 1; //need to sell
int breadord = 0, cakeord = 0; //receive bread and cake order number
int msgid;

int main(int argn, char *argc[])
{
	pthread_t *p1, p2; //p1 baker p2 consumer
	pthread_attr_t pAttr;
	int backv, i, bakerRet = -1;
	key_t key;
	key = ftok(argc[1], 100);
	msgid = msgget(key, IPC_CREAT | 0600);
	struct buynode *head = NULL;
	msg = malloc(sizeof(struct msgstr));

	printf("Input baker number:\n");
	scanf("%d", &n);
	printf("Input cake number:\n");
	scanf("%d", &caken);
	printf("Input bread number:\n");
	scanf("%d", &breadn);

	pMutex = malloc(sizeof(pthread_mutex_t) * (n + 1));
	pCond = malloc(sizeof(pthread_cond_t) * (n + 1));
	//Init pthread Attribute
	if ((backv = pthread_attr_init(&pAttr)) != 0)
	{
		printf("pthread attr init failure\n");
		return -1;
	}
	//Init Cond Mutex
	for (i = 0; i <= n; i++) {
		backv = pthread_cond_init(&pCond[i], NULL);
		if (backv != 0)
		{
			printf("Pthread Cond %d Init Failure.\n", i);
			perror("Cond Init Failure\n");
			return -1;
		}
	}
	for (i = 0; i <= n; i++) {
		backv = pthread_mutex_init(&pMutex[i], NULL);
		if (backv != 0)
		{
			printf("Pthread Mutex %d Init Failure.\n", i);
			perror("Mutex Init Failure\n");
			return -1;
		}
	}
	//POSIX thread Init

	if ((pthread_create(&p2, &pAttr, consumer, NULL)) != 0)
	{
		printf("thread 2 create failure\n");
		return -2;
	}
	else
		printf("Start to receive orders\n");

	p1 = malloc(sizeof(pthread_t) * n);
	pthread_mutex_lock(&pmMutex);
	for (i = 0; i < n; i++) {
		int j = i + 1;
		backv = pthread_create(&p1[i], &pAttr, baker, &j);
		if (backv != 0)
		{
			printf("backer thread create failure\n");
			return -2;
		}
		pthread_cond_wait(&pmCond, &pmMutex); //function signal num set finish
	}	
	pthread_mutex_unlock(&pmMutex);
	

	for (i = 0; i < n; i++)
	{
		backv = pthread_join(p1[i], (void *)(&bakerRet));
		pthread_mutex_destroy(&pMutex[i]);
		pthread_cond_destroy(&pCond[i]);
		if (backv != 0)
		{
			printf("backer thread join failure\n");
			return -2;
		}
		else
			printf("Baker %d go home\n", bakerRet);
	}
	pthread_mutex_destroy(&pmMutex);
	pthread_cond_destroy(&pmCond);
	return 0;
}

void *baker(void *data)
{
	int num = 0;
	int sig = *((int*)data);
	pthread_t s1;
	printf("baker %d working.\n", sig);
	pthread_cond_signal(&pmCond);

	//work
	while (1)
	{
		pthread_mutex_lock(&pMutex[0]);
		num = rmnode(Shead, Sstep);
		if (breadn <= 0 && caken <= 0)
		{
			printf("Sell Out\n");
			break;
		}
		if (num == CAKE)
		{
			if (caken > 0)
			{
				caken--;
				printf("Baker %d Sell a cake, cake left number: %d\n", sig, caken);
			}
			else
			{
				printf("Cake has sold out\n");
			}
		}
		else if (num == BREAD)
		{
			if (breadn > 0)
			{
				breadn--;
				printf("Baker %d Sell a bread, bread left number: %d\n", sig, breadn);
			}
			else
			{
				printf("Bread has sold out\n");
			}
		}
		else
		{
			pthread_cond_wait(&pCond[0], &pMutex[0]);
			printf("Nothing\n");
		}
		pthread_mutex_unlock(&pMutex[0]);
		pthread_mutex_lock(&pMutex[sig]);
		pthread_create(&s1, NULL, BakerRest, &sig);
		pthread_cond_wait(&pCond[sig], &pMutex[sig]);
		pthread_mutex_unlock(&pMutex[sig]);
	}
	pthread_exit(&sig);
}

void *consumer(void *data)
{
	int num = 0;
	printf("Receiving Orders...\n");
	while (1)
	{
		memset(msg, '\0', sizeof(struct msgstr));
		msgrcv(msgid, msg, MSGSIZE, 1, 0);
		if(msg == NULL) continue;
		if (strlen(msg->p) > 0)
		{
			if ((msg->p[0] - 48) == BREAD)
			{
				breadord++;
				addnode(Shead, Sstep, breadn, 1);
				printf("Receive bread(break order: %d)\n", breadord);
				pthread_cond_signal(&pCond[0]);
			}
			else if ((msg->p[0] - 48) == CAKE)
			{
				cakeord++;
				addnode(Shead, Sstep, caken, 0);
				printf("Receive cake(cake order: %d)\n", cakeord);
				pthread_cond_signal(&pCond[0]);
			}
			else
				printf("unknown kind\n");
		}
		//else
		//	printf("bad order\n");
	}
}

void *BakerRest(void *data)
{
	int sig = *((int *)data);
	sleep(1);
	pthread_cond_signal(&pCond[sig]);
	return NULL;
}

//qhead is queue's head, cur is current
int addnode(struct buynode *qhead, struct buynode *cur, int curnum, int kind) //curnum is cake or bread number
{
	//return
	// 1 set head
	// 2 set current
	pthread_mutex_lock(&nodeMutex);
	struct buynode *t;
	if (cur == NULL)
	{
		cur = malloc(sizeof(struct buynode));
		cur->num = curnum;
		cur->kind = kind;
		cur->next = NULL;
		qhead = cur;
		return 1;
	}
	else
	{
		t = malloc(sizeof(struct buynode));
		t->num = curnum;
		t->next = NULL;
		t->kind = kind;
		cur->next = t;
		cur = t;
		return 2;
	}
	pthread_mutex_unlock(&nodeMutex);
}

//qhead is queue's head, cur is current
int rmnode(struct buynode *qhead, struct buynode *cur)
{
	//return the num of the node that was be removed
	pthread_mutex_lock(&nodeMutex);
	int ret = -1;
	if (qhead == NULL)
	{
		return -1;
	}
	else if (qhead->next == NULL)
	{
		ret = qhead->kind;
		free(cur);
		free(qhead);
		qhead = NULL;
		cur = NULL;
		return ret;
	}
	else
	{
		ret = qhead->kind;
		free(qhead);
		qhead = cur;
		return ret;
	}
	pthread_mutex_unlock(&nodeMutex);
}
