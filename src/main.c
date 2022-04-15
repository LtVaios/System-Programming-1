#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
  
int main(int argc, char *argv[]) {
    //Argument and flag checking
    if(argc == 1){
        printf("Default path is now current file.\n");
    }
    else{
        if(strcmp(argv[1],"[-p") != 0){
            printf("Unknown flag, default path is now current file.\n");
        }
        char* path = malloc(sizeof(argv[2]));
        char* ptr = argv[2];
        int counter = 0;
        int flag = 0;
        while(ptr){
            if(*ptr == ']'){
                flag = 1;
                break;
            }
            ptr++;
            counter++;
        }
        if(flag == 1)
            strncpy(path, argv[2], counter);
        else
            printf("Unknown argument syntax, default path is now the current file.\n");
    }
    
    return 0;
}