#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<signal.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>

#define ENV_KEY "MSG_KEY"

key_t key;
int msqid;

struct msg_data{
	long msg_type;
	char msg_text;
};

struct msg_buffer{
	long msg_type;
	char message[200];
	struct msg_buffer *next;
} *buf_head;

void exit_program(int signal){
	struct msg_buffer *temp;
	while(buf_head){
		temp = buf_head;
		buf_head = buf_head->next;
		free(temp);
	}

	if(msgctl(msqid, IPC_RMID, NULL) == -1){
		perror("msgctl");
		exit(1);
	}
	exit(0);
}

void print_list(){
	struct msg_buffer *temp = buf_head;
	while(temp->next){
		printf("msg_type: %ld\nmsg_text: %s\n---------\n", temp->next->msg_type, temp->next->message);
		temp = temp->next;
	}
}

void receive_message(){
	struct msg_data temp_buf;
	bool found, end_message = true;
	int counter;

	for(;;){
		if(end_message){
			counter = 1;
			temp_buf.msg_type = -1;
			while(counter <= 10 && temp_buf.msg_type == -1){
				printf("Receiving messages...(%d)\n", counter);
				msgrcv(msqid, (struct msg_data*)&temp_buf, sizeof(char), 0, IPC_NOWAIT);
				counter++;
				sleep(1);
			}
			if(counter > 10){
				printf("msgrcv timeout... No more messages arrived!\n");
				return;
			}
		}
		else{
			msgrcv(msqid, (struct msg_data*)&temp_buf, sizeof(char), 0, 0);
		}

		end_message = false;
		printf("Received msg_type: %ld\nReceived msg_text: %c\n", temp_buf.msg_type, temp_buf.msg_text);

		struct msg_buffer *temp = buf_head;
		found = false;
		while(temp->next){
			if(temp->next->msg_type == temp_buf.msg_type){
				strncat(temp->next->message, &temp_buf.msg_text, 1);
				found = true;
				break;
			}
			temp = temp->next;
		}

		if(!found){
			struct msg_buffer *new_msg = malloc(sizeof(struct msg_buffer));
			new_msg->msg_type = temp_buf.msg_type;
			new_msg->message[0] = temp_buf.msg_text;
			temp->next = new_msg;
		}

		if(temp_buf.msg_text == '\0') end_message = true;
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
	print_list();

	exit_program(0);
	return 0;
}
