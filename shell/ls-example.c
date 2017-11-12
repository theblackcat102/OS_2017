#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    pid_t childpid;

    childpid = fork();

    if (childpid < 0) {   /* cannot allocate new process */
        printf("fork failed\n");
    } else if (childpid == 0) {   /* child process */
        // execlp("cd","cd","/Users/theblackcat/Downloads",(char *) NULL);
        // printf("eh eh eheheheb\n");
        execlp("cd","cd", "/Users/theblackcat/Downloads",(char *)NULL);
        execlp("ls","ls", "-alh",(char *)NULL);
        printf("(pid=%d) I'm child, I will sleep for 3 seconds\n", getpid());
        sleep(3);
    } else {   
        /* 
            ---parent process--- 
            If you don't receive it child will became zombie
        */ 
        pid_t pid = getpid();
        printf("(pid=%d) I'm parent, and my child's pid is %d\n", pid, childpid);
        int status;
        waitpid(childpid, &status, 0);   /* wait for child process terminated  else child became zombie, how to do if add & ???*/
        /*  if status is 0, then fork is success, else if status is 1 then fork failed  */
        printf("(pid=%d) My child has been terminated\nStatus %d\n", pid,status);
    }

    return 0;
}