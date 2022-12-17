#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

#define ENV_KEY "MSG_KEY"


int main(int argc, char **argv){
	if(argc <= 1){
		printf("No args provided! Exiting...\n");
		return 0;
	}

	char *uid_char = malloc(sizeof(char)*10);
	sprintf(uid_char, "%d", getuid());

	if(setenv(ENV_KEY, uid_char, 1) != 0){
		perror("setenv");
		return -1;
	}


	if(fork() == 0){
		char **args = malloc(sizeof(char)*100*argc);
		for(int i = 0; i<argc; i++){
			args[i] = argv[i];
			printf("%s\n", args[i]);
		}

		execvp("./proizvodac", args);
	}

	if(fork() == 0){
		execvp("./potrosac", NULL);
	}

	wait(NULL);
	wait(NULL);

	if(unsetenv(ENV_KEY) != 0){
		perror("unsetenv");
		return -1;
	}
	return 0;
}
