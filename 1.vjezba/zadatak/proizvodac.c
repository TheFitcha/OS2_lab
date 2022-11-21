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

struct sending_buffer{
	long msg_type;
	char msg_text;
};

void send_message(char *message){
	int msgid;

	char *pid_char = malloc(sizeof(char) * 10);
	sprintf(pid_char, "%d", getpid());

	struct sending_buffer *sbuf = malloc(sizeof(struct sending_buffer));
	sbuf->msg_type = getpid();

	printf("Pid: %s\nSending message: %s\n", pid_char, message);

	if((msgid = msgget(key, 0666 | IPC_CREAT)) == -1){
		perror(strcat("msgget, child pid: ", pid_char));
		exit(1);
	}

	struct msqid_ds *queue_ds = malloc(sizeof(struct msqid_ds));
	struct ipc_perm *perms = malloc(sizeof(struct ipc_perm));
	perms->mode = 0600;
	perms->uid = getuid();
	perms->gid = getgid();
	queue_ds->msg_perm = *perms;
	queue_ds->msg_qbytes = 5;

	if(msgctl(msgid, IPC_SET, queue_ds) == -1){
		perror("msgctl");
		exit(-1);
	}

	for(int i = 0; i<strlen(message); i++){
		sbuf->msg_text = message[i];
		printf("sbuf->msg_text: %c\n", sbuf->msg_text);
		if(msgsnd(msgid, sbuf, sizeof(sbuf->msg_text), 0) == -1){
			perror(strcat("msgsnd, child pid: ", pid_char));
		}
	}

	sbuf->msg_text = '\0';
	printf("sbuf->msg_text: %c\n", sbuf->msg_text);
	if(msgsnd(msgid, sbuf, sizeof(sbuf->msg_text), 0) == -1){
		perror(strcat("msgsnd, child pid: ", pid_char));
	}

	free(sbuf);
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
		char *message = malloc(sizeof(char)*200);
		strcpy(message, argv[i]);
		if(fork() == 0){
			send_message(message);
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
