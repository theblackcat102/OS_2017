#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#define NUMTHREAD 16
// using namespace std;
int count = 0;

int size = 100;

typedef struct params{
    int low;
    int pivot;
    int high;
    int node;
    int *array;
}thread_params;

int *global_array;

pthread_t thread[NUMTHREAD+8];
sem_t lock[NUMTHREAD+8];
thread_params *t_params[NUMTHREAD];

void print_array(int *array){
    for(int i=0;i<size;i++){
        printf("%d ",array[i]);
    }
    printf("\n");
}

void swap(int *xp, int *yp){
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

int partition(int *sequence,int low,int high){
    int pivot = sequence[high];
    int i = low-1;
    for(int j=low;j< high;j++){
        if(sequence[j] <= pivot){
            i++;
            swap(&sequence[i],&sequence[j]);
        }
    }
    swap(&sequence[i+1],&sequence[high]);
    return i+1;
}

void bubbleSort(int *arr, int s, int n)
{
   int i, j;
   for (i = s; i < n-1; i++)      
        for (j = s; j < n-i-1; j++) 
           if (arr[j] > arr[j+1])
              swap(&arr[j], &arr[j+1]);
}

int is_sequential(int *arr,int n){
    for(int i=1;i<n;i++){
        if((arr[i]-arr[i-1])!=1){
            return 0;
        }
    }
    return 1;
}

int check_sequential(int *arr,int s,int n){
    for(int i=s+1;i<n;i++){
        if((arr[i]-arr[i-1])!=1){
            return 0;
        }
    }
    return 1;
}

int* spliter(char *line,int length){
    int* number = (int *) malloc(sizeof(int)*length);
    int count=  0;    
    line = strtok(line," ");
    while(line != NULL){
        number[count++] = atoi(line); 
		line = strtok(NULL," ");
    }
    return number;
}

int* read_file(char* file_name,int *total_length){
    int *number_list;  
    int line_num = 0,total_num=0,i=0;
    size_t length= 1000000000;
    char *line =(char*) malloc(sizeof(char)*length);
    FILE* fp = fopen(file_name, "r");

    if(fp){
      while ( getline(&line,&length,fp))
      {
        if(line_num == 0){
            char* length_size_str = strtok(line," \n");
            total_num = atoi(length_size_str);
            *total_length = total_num;
            line_num++;
            number_list = (int*) malloc(sizeof(int)*total_num);
        }else{
            number_list = spliter(line,total_num);
            break;
        }
      }
      fclose(fp);
    }
    printf("total %d\n",*total_length);
    return number_list;
}

void quickSort(int *arr,int low,int high){
    if (low < high){
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = partition(arr, low, high);

        quickSort(arr, low, pi - 1);  // Before pi
        quickSort(arr, pi + 1, high); // After pi
    }
}

void* split_thread(void *arguments){
    thread_params *arg = (thread_params *)arguments;
    sem_wait(&lock[arg->node]);
    // printf("Thread %d started, low:%d, high:%d\n", arg->node, arg->high, arg->low);
    int pivot = -1;
    if (arg->low < arg->high){
        pivot = partition(global_array,arg->low,arg->high);
        t_params[(arg->node)*2]->low = arg->low;
        t_params[(arg->node)*2]->high = pivot-1;
        t_params[(arg->node)*2]->array = global_array;
        t_params[((arg->node)*2)+1]->low = pivot+1;
        t_params[((arg->node)*2)+1]->high = arg->high;
        t_params[(arg->node)*2+1]->array = global_array;

        // printf("\nPivot number: %d \nLow:\n",arg->array[pivot]);
        // for(int i=arg->low;i<=pivot-1;i++){
        //     printf("%d ",arg->array[i]);
        // }
        // printf("\nHigh:\n",pivot);
        // for(int i=pivot+1;i<=arg->high;i++){
        //     printf("%d ",arg->array[i]);
        // }
        // printf("\n\n");
    }
    // printf("Thread %d finished,pivot= %d, signaling %d,%d\n", arg->node,pivot,arg->node*2,(arg->node*2)+1);
    sem_post(&lock[arg->node]);
    sem_post(&lock[arg->node*2]);
    sem_post(&lock[(arg->node*2)+1]);
    return NULL;
}


void* sort_thread(void *arguments){
    thread_params *arg = (thread_params *)arguments;
    sem_wait(&lock[arg->node/2]);
    sem_wait(&lock[arg->node]);
    printf("\n Thread %d started\n", arg->node);
    printf("\n%d %d\nOriginal sequence\n",arg->low,arg->high);

    for(int i=arg->low;i<=arg->high;i++){
        printf("%d ",global_array[i]);
    }
    // for (int k = arg->low-1; k <= arg->high; k++){
    //     for (int j = arg->low-1; j <= arg->high - k; j++){
    //        if (arg->array[j] > arg->array[j+1]){
    //            int temp = arg->array[j];
    //            arg->array[j] = arg->array[j+1];
    //            arg->array[j+1] = temp;
    //        }
    //     }
    // }
    bubbleSort(global_array,arg->low,arg->high);

    printf("\nPrint thread sequence\n");
    for(int i=arg->low;i<=arg->high;i++){
        printf("%d ",global_array[i]);
    }
    // }
    printf("\n Thread %d finished\n", arg->node);
    sem_post(&lock[arg->node]);
    sem_post(&lock[arg->node/2]);
    return NULL;
}


int main(int argc,char* argv[]){
    printf("Read file");
    int *a,*b,*number_list,*number_list2,err;
    a = (int *)malloc(sizeof(int));
    b = (int *)malloc(sizeof(int));
    number_list2 = read_file("example.txt",a);
    global_array = read_file("example.txt",b);

    quickSort(number_list2,0,*b-1);
    printf("Result: %d\n",is_sequential(number_list2,*a));
    print_array(number_list);
    for(int i=0;i<NUMTHREAD;i++){
        // ubuntu use sem_init
        sem_init(&lock[i], 0, 0);
        // mac use sem_open
        // sem_open(&lock[i],0,0);
    }

    for(int i=1;i<NUMTHREAD;i++){
        t_params[i] = malloc(sizeof(thread_params));
        t_params[i]->high = 0;
        if(i == 1)
            t_params[i]->high = *a-1;
        t_params[i]->low = 0;
        t_params[i]->node = i;
        t_params[i]->array = global_array;

        if( i < 8){
            pthread_create(&(thread[i]), NULL, &split_thread, (void *)t_params[i]);
        }else{
            pthread_create(&(thread[i]), NULL, &sort_thread, (void *)t_params[i]);
        }
    }
    sem_post(&lock[1]);
    getchar();
    // printf("Start bubble sort\n");
    // bubbleSort(number_list,*a);
    printf("Result: %d\n",is_sequential(number_list,*a));
    print_array(number_list);
    return 0;
}