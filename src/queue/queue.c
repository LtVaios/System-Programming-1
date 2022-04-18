#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

struct queue_struct{
    int no_of_nodes;
    node head;
};

struct queuenode{
    int pid;
    node next;
};

//Initializes a queue
queue init_q(void){
    queue q = malloc(sizeof(struct queue_struct));
    q->no_of_nodes = 0;
    q->head = NULL;
}

//Returns the PID at the head of the queue and frees that node
int pop(queue q){
    if(q->no_of_nodes == 0 /*empty*/)
        return -1;
    int ret = q->head->pid;
    node to_be_freed = q->head;
    q->head = q->head->next;
    free(to_be_freed);
    q->no_of_nodes-=1;
    return ret;
}

//returns 1 if queue is empty
int empty_q(queue q){
    if(q->no_of_nodes == 0)
        return 1;
    else
        return 0;
}

//make a new node given a pid and pushes that to the end of the queue
void push(queue q, int new_pid){
    q->no_of_nodes+=1;
    node new_node = malloc(sizeof(struct queuenode));
    new_node->pid = new_pid;
    new_node->next = NULL;
    if(q->head == NULL){
        q->head = new_node;
        return;
    }
    node temp = q->head;
    while(temp->next)
        temp = temp->next;
    temp->next = new_node;
    return;
}

//Frees all the allocated space used for the queue
void destroy_q(queue q){
    if(q == NULL)
        return;
    node curr = q->head;
    node prev;
    while(curr){
        prev = curr;
        curr = curr->next;
        prev->next = NULL;
        free(prev);
    }
    free(q);
    return;
}