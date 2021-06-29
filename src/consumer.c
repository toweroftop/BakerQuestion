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
#include<sys/msg.h>
#include<sys/ipc.h>

#define MSGSIZE 10
#define CAKE 0
#define BREAD 1

struct msgstr {
    long MType;
    char p[0];
};

struct msgstr *msg;

int main(int argn, char *argc[])
{
    int i, msgid;
    key_t key;
    char *inc;
    key = ftok(argc[1], 100);
    msgid = msgget(key, IPC_CREAT | 600);
    while(1)
    {
        printf("Input number 0 is cake   1 is bread\n");
        inc = (char *)malloc(sizeof(char) * MSGSIZE);
        fgets(inc, MSGSIZE, stdin);
        msg = malloc(sizeof(struct msgstr) + strlen(inc) + 1);
        msg->MType = 1;
        memcpy(&msg->p, inc, strlen(inc));
        msgsnd(msgid, &msg, 1, 0);
        free(inc);
        free(msg);
    }
}
