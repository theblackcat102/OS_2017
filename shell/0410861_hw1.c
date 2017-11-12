#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#define BUFFERSIZE 200;
#define ARGSIZE 10;
#define DELIMITER " \t\r\n"
/*

	Reference link:
	https://github.com/hungys/NCTU_OS_2015_HW1/blob/master/QA.md
	https://cboard.cprogramming.com/c-programming/61297-execvp-ampersand.html

*/

struct CommandSegment{
	char** arguments;
	int mode;
	char *pipe_output;
};
char *directory;
pid_t childpid;

void show_prompt(){
	if(directory != NULL){
		printf("%s > ",directory );
	}else{
		directory = (char *)malloc(sizeof(char)*200);
		getcwd(directory,sizeof(char)*200);
		printf("%s > ",directory);
	}
}

void fix_directory(char *path){
	int string_buffer_size = BUFFERSIZE;
	if(path == NULL){
		return;
	}
	if(path[0] != '/' && strcmp(path,"..") != 0){
		char *buf = (char *)malloc(string_buffer_size*sizeof(char));
		getcwd(buf,sizeof(char)*string_buffer_size);
		if(buf != NULL){
			buf = strcat(buf,"/");
			path = strcat(buf,path);
		}
	}
}

int execute_cd(char *path){
	fix_directory(path);
	if(path == NULL){
		return 0;
	}
	int status = chdir(path);
	if(status == 0 ){
		getcwd(directory,sizeof(char)*200);
	}
	else{
		printf("Directory error!\n");
	}
	return status;
}

char *readline(){
	char *cmd = NULL;
	size_t buffer_size = 0;
	getline(&cmd,&buffer_size,stdin);
	return cmd;
}

struct CommandSegment read_cmd(){
	int buffer_size = ARGSIZE;
	char **arguments = (char **)malloc(buffer_size * sizeof(char*));
	char *command  = readline();
	int idx = 0;
	int mode =  0;
	struct CommandSegment command_seg;
	if(command == NULL){
		return command_seg;
	}
	char *cmd  = strtok(command,DELIMITER);
	while(cmd != NULL){
		if(strcmp(cmd,"&") == 0){
			mode = 1;
		}
		else if(strcmp(cmd,">")==0){
			mode = 2;
		}else{
			if(mode == 2){
				command_seg.pipe_output = (char *)malloc(sizeof(char)*buffer_size);
				strcpy(command_seg.pipe_output,cmd);
			}else{
				arguments[idx++] = cmd;										
			}
		}
		if(idx > buffer_size){
			arguments = (char **)realloc(arguments,buffer_size*(sizeof(char*)));
			if(arguments == NULL){
				printf("Memory reallocation failed");
				exit(0);
			}
		}
		cmd = strtok(NULL,DELIMITER);
	}
	arguments[idx] = NULL;
	
	command_seg.arguments = arguments;
	command_seg.mode = mode;
	return command_seg;
}

void sigint_handler(int sig) {   /* handler body */
    while(waitpid((pid_t)(-1),0,WNOHANG) > 0){}
}

int execute_command(struct CommandSegment command_seg){
	char **arguments = command_seg.arguments;
	int wait = command_seg.mode;
	char *file = command_seg.pipe_output;
	struct sigaction sigint_action = {
        .sa_handler = &sigint_handler,
        .sa_flags = SA_RESTART
    };
	
	if(arguments == NULL || arguments[0] == NULL){
		return 1;
	}
	if(strcmp(arguments[0],"exit()") ==0 ){
		kill(getpid(),SIGINT); // kill process
	}else if(strcmp(arguments[0],"cd") == 0){
		if(arguments[1] != NULL){
			execute_cd(arguments[1]);
		}
		return 1;
	}else if(strcmp(arguments[0], "rm") == 0){
		if(arguments[1] != NULL){
			fix_directory(arguments[1]);
		}
	}else if(strcmp(arguments[0], "mv") == 0){
		if(arguments[1] != NULL){
			fix_directory(arguments[1]);
		}
		if(arguments[2] != NULL){
			fix_directory(arguments[2]);
		}
	}
	
	childpid = fork();
	if(childpid < 0){
		printf("Child allocation failed");
		return 0;
	}
	else if(childpid == 0){

		if(wait == 2){
			int fd=open(file,O_WRONLY|O_CREAT,0644); // 0644 allows user access permission : -rw-r--r--
			dup2(fd, 1);   /* replace stdout */
	        close(fd);
	        // close(fd[1]);
		}
		int status = execvp(arguments[0],arguments);
		if(status == -1)
			perror("[1] EXECVP Error");
		    exit(EXIT_FAILURE);
	}else{
		if(wait == 1){
			printf("[1] %d\n",childpid);
			sigaction(SIGCHLD, &sigint_action, NULL); 
		}
		else{
			int status;
			do{
				int status = waitpid(childpid, &status, 0);
			}while(!WIFEXITED(status) && !WIFSIGNALED(status));
		}
	}	
	return 1;
}

void launch(){
	getcwd(directory,sizeof(directory));
	int run = 1;
	printf("|  0410861 Zhi Rui Tam\n|  Contain I/O directional command >\n|  Use exit() to leave this shell\n");
	while(run){
		show_prompt();
		struct CommandSegment seg = read_cmd();
		int status = execute_command(seg);
		if(status == -1){
			printf("command line exiting\n");
			exit(0);
			return;
		}
	}
}

int main(int argc, char **argv){
	launch();
	return 0;
}