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

	if(setenv(ENV_KEY, "12345", 1) != 0){
		perror("setenv");
		return -1;
	}

	if(fork() == 0){
		char *args_proc[] = {"./proizvodac", argv[1], NULL};
		execv("./proizvodac", args_proc);
	}

	if(fork() == 0){
		char *args_proc[] = {"./potrosac", NULL};
		execv("./potrosac", args_proc);
	}

	wait(NULL);
	wait(NULL);

	if(unsetenv(ENV_KEY) <= 1){
		perror("unsetenv");
		return -1;
	}
	return 0;
}
