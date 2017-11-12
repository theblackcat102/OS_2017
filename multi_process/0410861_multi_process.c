#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>
unsigned int share_a,share_b,share_c;

void initial_matrix(unsigned int **a,unsigned int **b,unsigned int **c,int dimension){
	if(dimension <= 0){
		printf("Invalid dimension, dimension size must be larger than 0");
		return;
	}

	// int *a_1,*b_1,*c_1;
	share_a = shmget(IPC_PRIVATE, dimension*dimension*sizeof(unsigned int),IPC_CREAT | 0666);
	share_b = shmget(IPC_PRIVATE, dimension*dimension*sizeof(unsigned int),IPC_CREAT | 0666);
	share_c = shmget(IPC_PRIVATE, dimension*dimension*sizeof(unsigned int),IPC_CREAT | 0666);
	if(share_a < 0 || share_b < 0 || share_c < 0){
		printf("Error locating share memory!");
		return;
	}
	*a = (unsigned int *)shmat(share_a,0,0);
	*b = (unsigned int *)shmat(share_b,0,0);
	*c = (unsigned int *)shmat(share_c,0,0);

	for(int i=0;i<dimension*dimension;i++){
		(*a)[i] = (unsigned int)i;
		(*b)[i] = (unsigned int)i;
		(*c)[i] = 0;
	}
}

unsigned int calculate_checksum(unsigned int *mat,int dimension){
	if(mat == NULL){
		printf("This matrix has nothing!");
		return -1;
	}
	unsigned int checksum = 0;
	for(int i=0;i<dimension*dimension;i++){
		checksum += mat[i];
	}
	return checksum;
}


void single_process(unsigned int **a,unsigned int **b,unsigned int **c,int dimension){
	//initial_matrix(a, b, c, dimension);
	struct timeval start,end;
	gettimeofday(&start,0);
	for(int i=0;i<dimension;i++){ // a row 
		for(int j=0;j<dimension;j++){ // b column
			(*c)[i*dimension+j] = 0;
			for(int k=0;k<dimension;k++){
				(*c)[j+(i*dimension)] += (*a)[(i*dimension)+k]*(*b)[(k*dimension)+j];
			}
		}
	}
	gettimeofday(&end,0);
	double micro_interval = (end.tv_usec-start.tv_usec);
	double time_interval = (end.tv_sec-start.tv_sec) + (micro_interval/1000000);
	printf("\nMultiplying using 1 process\n");
	printf("Elapsed time: %f sec,Checksum: %u\n",time_interval,calculate_checksum(*c,dimension));	
}

void multiple_process(unsigned int **a,unsigned int **b,unsigned int **c,int dimension,int n_process){
	int pid,status;
	struct timeval start,end;
	int increment = (dimension/n_process);
	int flag = shmget(IPC_PRIVATE,(n_process+1)*sizeof(int),IPC_CREAT|0666);
	int *start_point = (int*)shmat(flag,0,0);
	for (int i=1;i<=n_process;++i)
		start_point[i] += increment*i;
	gettimeofday(&start, 0);
	start_point[n_process] = dimension;
	int process_number,starting_point;

	for(process_number=1;process_number<=n_process;process_number++){
		pid = fork();
		if(pid < 0){
			printf("Error forking a child");
			process_number--;
		}else if(!pid){ // is a child process break from loop and start calculation
			printf("RUn");
			break;
		}
	}
	// set the starting flag for each child and parent proc
	if(pid == 0)
		starting_point = process_number;
	else{
		starting_point = 0;
	}
	if(starting_point){
		for(int i=start_point[starting_point-1];i<start_point[starting_point];i++){
			for(int g=0;g < dimension;g++){ 
				(*c)[i*dimension+g] = 0;
				for(int k=0;k < dimension;k++){
					(*c)[(i*dimension)+g] += (*a)[k+(i*dimension)]*(*b)[g+(k*dimension)];
				}
			}
		}
	}

	if(n_process==(process_number-1)&& pid != 0){ // parent wait for child
		int status;
		while(wait(&status)>0);
	}
	else{
		_exit(0);
	}

	for(int i=start_point[starting_point];i<start_point[starting_point];i++){
		for(int g=0;g < dimension;g++){ 
			(*c)[i*dimension+g] = 0;
			for(int k=0;k < dimension;k++){
				(*c)[(i*dimension)+g] += (*a)[k+(i*dimension)]*(*b)[g+(k*dimension)];
			}
		}
	}
	shmdt(start_point);
	shmctl(flag,IPC_RMID,0);
	gettimeofday(&end,0);
	double micro_interval = (end.tv_usec-start.tv_usec);
	double time_interval = (end.tv_sec-start.tv_sec) + (micro_interval/1000000);
	printf("\nMultiplying using %d process\n",n_process);
	printf("Elapsed time: %f sec,Checksum: %u\n",time_interval,calculate_checksum(*c,dimension));	
}

int main(){
	int dimension;
	do{
		printf("Dimension: ");
		scanf("%d",&dimension);
	}while(dimension <= 0);
	unsigned int *a;
	unsigned int *b;
	unsigned int *c;
	initial_matrix(&a, &b, &c, dimension);

	// this is for checksum validation for multi processing 
	// single_process(&a,&b,&c,dimension);

	// create process 1-16
	for(int i=1;i<=16;i++){
		multiple_process(&a,&b,&c,dimension,i);
	}
	shmdt(&a);
	shmdt(&b);
	shmdt(&c);
	shmctl(share_a,IPC_RMID,0);
	shmctl(share_b,IPC_RMID,0);
	shmctl(share_c,IPC_RMID,0);
	return 0;
}