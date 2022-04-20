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
    //domain name
    char* link;
    //this counter symbolizes how many times it appeared
    int counter;
    setnode next;
};

//Initializes a new set
set init_set(void){
    set s = malloc(sizeof(struct set_struct));
    s->head = NULL;
}


//This is like a regular list insert but if we find a node with the same 'link' we just increase the counter and we don't push another actual node
void set_insert(set s, char new_link[1024]){
    setnode new_setnode = malloc(sizeof(struct set_node));
    new_setnode->link = malloc(1024);
    strcpy(new_setnode->link, new_link);
    new_setnode->next = NULL;
    if(s->head == NULL){
        new_setnode->counter = 1;
        s->head = new_setnode;
        return;
    }
    else{
        setnode curr = s->head;
        while(curr->next != NULL){
            if(strcmp(curr->link,new_link) == 0){
                curr->counter++;
                free(new_setnode->link);
                free(new_setnode);
                return;
            }
            curr = curr->next;
        }
        new_setnode->counter = 1;
        curr->next = new_setnode;
        return;
    }
}

//deletes every malloced resource in the list
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

//This function writes every domain that is listed with the number it appeared as we made it in worker.c
void write_all_to_file(set s, char* write_to){
    char* fullpath = malloc(512);
    char* temp;
    //make full path to output files
    strcpy(fullpath, "./output_files/");
    strcat(fullpath, write_to);
    int fd_write = open(fullpath, O_CREAT | O_RDWR, 0777);
    setnode curr = s->head;
    //repeat until the end of the list
    while(curr){
        //add space
        strcat(curr->link, " ");
        //add the number a certain domain appeared
        sprintf(temp, "%d", curr->counter);
        strcat(curr->link, temp);
        strcat(curr->link, "\n");
        if(write(fd_write, curr->link, strlen(curr->link)+1) == -1 && errno != EINTR)
            printf("Error writing in out file.\n");
        curr = curr->next;
    }
    free(fullpath);
}