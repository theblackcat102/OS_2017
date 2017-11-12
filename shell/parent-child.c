#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int x=0;

int main(){
	pid_t pid;
	pid = vfork();

	/*
		vfork()
		where the copying of the process address space is transparently faked by allowing both processes 
		to refer to the same physical memory
		fork()
		both process were reference on different physical memory
	*/
	if (pid < 0){
		fprintf(stderr, "Fork Failed");
		exit(-1);
	}	
	else if(pid == 0){
		x++; // both parent and child was execute on different process, data is not share
		// how to do shared data? Pipeline
		_exit(0);
	}
	else{
		wait(NULL);
		printf("X number: %d\n",x);
		exit(0);
	}
}