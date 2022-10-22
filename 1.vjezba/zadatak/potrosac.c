#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<signal.h>

#define ENV_KEY "MSG_KEY"

key_t key;
int msqid;

struct msg_data{
	long msg_type;
	char msg_text[200];
};

struct msg_buffer{
	struct msg_data data;
	struct msg_buffer *next;
} *buf_head;

void exit_program(int signal){
	if(msgctl(msqid, IPC_RMID, NULL) == -1){
		perror("msgctl");
		exit(1);
	}
	exit(0);
}

void receive_message(){
	struct msg_data temp_buf;

	for(;;){
		printf("Receiving messages...\n");
		if(msgrcv(msqid, (struct msg_data*)&temp_buf, sizeof(temp_buf.msg_text), 0, 0) == -1){
			perror("msgrcv");
			exit(1);
		}

		printf("Received msg_type: %ld\nReceived msg_text: %s\n", temp_buf.msg_type, temp_buf.msg_text);
	}
}


int main(int argc, char** argv){
	if(!getenv(ENV_KEY)){
		perror("getenv");
		exit(1);
	}

	key = atoi(getenv(ENV_KEY));
	printf("Queue key: %d\n", key);

	sigset(SIGINT, exit_program);

	buf_head = malloc(sizeof(struct msg_buffer));
	if((msqid = msgget(key, 0600 | IPC_CREAT)) == -1){
		perror("msgget");
		exit(1);
	}

	receive_message();

	free(buf_head);
	return 0;
}
