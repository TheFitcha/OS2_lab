#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<wait.h>

int main(int argc, char **argv){
	FILE *tasks = fopen("foo", "r");

	int inPipe[2], outPipe[2];
	if(pipe(inPipe) < 0){
		perror("inPipe create error");
		exit(-1);
	}
	if(pipe(outPipe) < 0){
		perror("outPipe create error");
		exit(-1);
	}

	char line[50];
	int userResult;
	char bcResultChar[50];

	if(fork() == 0){
		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);
		dup2(outPipe[1], STDERR_FILENO);
		close(outPipe[0]);
		close(outPipe[1]);
		close(inPipe[1]);
		close(inPipe[0]);

		execl("/usr/bin/bc", "bc", NULL);
	}

	close(inPipe[0]);
	close(outPipe[1]);

	while(fgets(line, sizeof(line), tasks)){
		line[strcspn(line, "\r\n")] = 0;
		printf("%s=", line);
		scanf("%d", &userResult);

		write(inPipe[1], line, strlen(line));
		write(inPipe[1], "\n", 1);
		read(outPipe[0], bcResultChar, sizeof(bcResultChar));

		if(strstr(bcResultChar, "syntax error")){
			printf("Neispravan izraz!\n");
		}
		else if(userResult == atoi(bcResultChar)){
			printf("ISPRAVNO!\n");
		}
		else{
			printf("NEISPRAVNO! Tocan rezultat je %d.\n", atoi(bcResultChar));
		}
	}

	close(inPipe[1]);
	close(outPipe[0]);
	wait(NULL);
	fclose(tasks);
	return 0;
}
