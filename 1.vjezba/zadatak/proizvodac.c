#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/wait.h>
#include<string.h>

#define ENV_KEY "MSG_KEY"

key_t key;

struct msg_buffer{
	long msg_type;
	char msg_text[200];
};

void send_message(struct msg_buffer *msgbuf){
	int msgid;

	char *pid_char = malloc(sizeof(char) * 10);
	sprintf(pid_char, "%d", getpid());

	msgbuf->msg_type = getpid();

	printf("Pid: %s\nSending message: %s\n", pid_char, msgbuf->msg_text);

	if((msgid = msgget(key, 0666 | IPC_CREAT)) == -1){
		perror(strcat("msgget, child pid: ", pid_char));
		exit(1);
	}

	if(msgsnd(msgid, msgbuf, sizeof(msgbuf->msg_text), 0) == -1){
		perror(strcat("msgsnd, child pid: ", pid_char));
	}

	free(msgbuf);
}

int main(int argc, char **argv){
	if(!getenv(ENV_KEY)){
		perror(strcat("getenv err, value: ", getenv(ENV_KEY)));
		exit(1);
	}

	key = atoi(getenv(ENV_KEY));
	printf("Args number: %d\n", argc);

	int child_counter = 0;
	for(int i = 1; i<argc; i++){
		struct msg_buffer *msgbuf = malloc(sizeof(struct msg_buffer));
		strcpy(msgbuf->msg_text, argv[i]);
		if(fork() == 0){
			send_message(msgbuf);
			exit(0);
		}
		child_counter++;
	}

	if(child_counter == 0){
		printf("No messages were sent!\n");
	}

	while(child_counter--) wait(NULL);

	return 0;
}
