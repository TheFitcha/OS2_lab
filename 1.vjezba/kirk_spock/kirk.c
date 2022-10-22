#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>

struct my_msgbuf{
	long mtype;
	char mtext[200];
};

int main(void){
	struct my_msgbuf buf;
	int msqid;
	key_t key;
	char text[] = "Kirk: We are attacked. Spock, send reinforcements.";

	key = ftok("key", 20);

	if((msqid = msgget(key, 0666 | IPC_CREAT)) == -1){
		perror("msgget");
		exit(1);
	}

	memcpy(buf.mtext, text, strlen(text)+1);
	buf.mtype = 1;

	if(msgsnd(msqid, (struct msgbuf *)&buf, strlen(text)+1, 0) == -1){
		perror("msgsnd");
		exit(1);
	}

	printf("Kirk: Reported attacks to Spock, he will send help!\n");

	return 0;
}
