#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include "set.h"

struct set_struct{
    setnode head;
};

struct set_node{
    char* link;
    int counter;
    setnode next;
};

//Initializes a new set
set init_set(void){
    set s = malloc(sizeof(struct set_struct));
    s->head = NULL;
}

void set_insert(set s, char new_link[1024]){
    setnode new_setnode = malloc(sizeof(struct set_node));
    new_setnode->link = malloc(1024);
    strcpy(new_setnode->link, new_link);
    if(s->head == NULL){
        new_setnode->next = NULL;
        new_setnode->counter = 1;
        s->head = new_setnode;
        return;
    }
    else{
        setnode curr = s->head;
        while(curr->next){
            if(strcmp(curr->link,new_link) == 0){
                curr->counter++;
                free(new_setnode->link);
                free(new_setnode);
                return;
            }
        }
        new_setnode->counter = 1;
        curr->next = new_setnode;
        return;
    }
}

void delete_set(set s){
    if(s == NULL)
        return;
    setnode curr = s->head;
    setnode prev;
    while(curr){
        prev = curr;
        curr = curr->next;
        prev->next = NULL;
        free(prev->link);
        free(prev);
    }
    free(s);
    return;
}

void write_all_to_file(set s, char* write_to){
    char* fullpath = malloc(512);
    strcpy(fullpath, "./output_files/");
    strcat(fullpath, write_to);
    printf("fullpath:%s\n",fullpath);
    int fd_write = open(fullpath, O_CREAT | O_RDWR);
    setnode curr = s->head;
    while(curr){
        strcat(curr->link, " ");
        sprintf(curr->link, "%d", curr->counter);
        if(write(fd_write, curr->link, strlen(curr->link)+1) == -1 && errno != EINTR)
            printf("Error writing in out file.\n");
        curr = curr->next;
    }
    if(write(fd_write, "\n", 2) == -1 && errno != EINTR)
            printf("Error writing in out file.\n");
}