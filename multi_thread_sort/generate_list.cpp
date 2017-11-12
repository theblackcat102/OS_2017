#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;


int main(int argc,char *argv[]){
    int j,i;
    if(argc == 2){
        printf("%s",argv[0]);
        int total = atoi(argv[1]);
        ofstream myfile ("example.txt");
        if(myfile.is_open()){
            int *num_list = new int[total];
            for(i =1;i<=total;i++){
                num_list[i-1] = i;
            }
            for(i=0;i<total;i++){
                j = rand()%total;
                if(i != j){
                    int temp = num_list[i];
                    num_list[i] = num_list[j];
                    num_list[j] = temp;
                }else if(i>0){
                    i--;
                }
            }
            myfile<<total;
            myfile<<" \n";
            for(int i=0;i<total;i++){
                myfile<<num_list[i];
                myfile<<" ";
            }    
        }
        myfile.close();
    }

}