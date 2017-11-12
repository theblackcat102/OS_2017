#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

int main ()
{
	int d;
	unsigned int chksum=0;
	struct timeval start, end;
	int shmid_a;
	int shmid_b;
	int shmid_c;
	int shmid_sub;
	unsigned int *a;
	unsigned int *b;
	unsigned int *c;
	int* sub;
	int count;
	int status;
	pid_t pid;

//********************************** input dimension ************************************
	printf("Dimension: ");
	scanf("%d",&d);

//********************************* init shared memory *********************************
	shmid_a = shmget(IPC_PRIVATE,d*d*sizeof(unsigned int),IPC_CREAT|0666);
	shmid_b = shmget(IPC_PRIVATE,d*d*sizeof(unsigned int),IPC_CREAT|0666);
	shmid_c = shmget(IPC_PRIVATE,d*d*sizeof(unsigned int),IPC_CREAT|0666);
	shmid_sub = shmget(IPC_PRIVATE,5*sizeof(int),IPC_CREAT|0666);
	a = (unsigned int*)shmat(shmid_a,NULL,0);
	b = (unsigned int*)shmat(shmid_b,NULL,0);
	c = (unsigned int*)shmat(shmid_c,NULL,0);
	sub = (int*)shmat(shmid_sub,NULL,0);

	for (int i=0;i<d*d;++i)
	{
		a[i] = i;
		b[i] = i;
		c[i] = 0;
	}

//****************************** 1-process calculate ***********************************
	gettimeofday(&start,0);
	for (int i=0;i<d;++i)
	{
		for (int x=0;x<d;++x)
		{
			c[i*d+x] = 0;
			for (int r=0;r<d;++r)
				c[(i*d)+x] += a[(i*d+r)] * b[(r*d)+x];
		}
	}

	for (int i=0;i<d;++i)
	{
		for (int x=0;x<d;++x)
			chksum += c[(i*d)+x];
	}	
	printf("1-process, checksum = %u\n", chksum);

	gettimeofday(&end,0);
	double sec = end.tv_sec - start.tv_sec;
	double usec = end.tv_usec - start.tv_usec;
	printf("elapsed %f s\n",sec+(usec/1000000));

//********************************* 4-process calculate ************************************
	int tmp = d/4;
	for (int i=1;i<=4;++i)
		sub[i] += tmp*i;
	sub[4] = d;

	gettimeofday(&start, 0);
	count = 0;
	while(count<3)
	{
		pid = fork();
		if(pid<0)
			perror("fork");
		else if (!pid)
		{
			++count;
			break;
		}
		else
			++count;
	}	

	// a = (unsigned int*)shmat(shmid_a,NULL,0);
	// b = (unsigned int*)shmat(shmid_b,NULL,0);
	// c = (unsigned int*)shmat(shmid_c,NULL,0);
	// sub = (int*)shmat(shmid_sub,NULL,0);

	if (pid)
		tmp = 0;
	else 
		tmp = count;

	for (int i=sub[tmp];i<sub[tmp+1];++i)
	{
		for (int x=0;x<d;++x)
		{	
			c[i*d+x] = 0;
			for (int r=0;r<d;++r) 
				c[i*d+x] += a[i*d+r] * b[r*d+x];
		}
	}

	shmdt(a);
	shmdt(b);
	shmdt(c);
	shmdt(sub);

	if (count==3&&!pid)
		_exit(1);
	else if(count==3)	
	{
		while(wait(&status)>0);
	}
	else 
		_exit(0);

	chksum = 0;
	c = (unsigned int*)shmat(shmid_c,NULL,0);
	for (int i=0;i<d;++i)
	{
		for (int x=0;x<d;++x)
			chksum += c[(i*d)+x];
	}
	printf("4-process,checksum = %u\n",chksum);
	shmdt(c);

	gettimeofday(&end, 0);
	sec = end.tv_sec - start.tv_sec;
	usec = end.tv_usec - start.tv_usec;
	printf("elapsed %f s\n",sec+(usec/1000000));

//****************************** end && clear the shared memory ******************************
	shmctl(shmid_a,IPC_RMID,0);
	shmctl(shmid_b,IPC_RMID,0);
	shmctl(shmid_c,IPC_RMID,0);
	shmctl(shmid_sub,IPC_RMID,0);
	return 0;
} 