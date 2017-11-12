#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>


int main(){
    int i, pid;
    int count = 0;
    for(i = 0; i < 5; i++) {
        pid = fork();
        if(pid < 0) {
            printf("Error");
            exit(1);
        } else if (pid == 0) {
            printf("Child (%d): %d\n", i + 1, getpid());
            exit(0); 
        } else  {
            printf("\nParent Process %d",pid);
            wait(NULL);
        }
    }

    return 0;
}