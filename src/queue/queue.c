#include <stdlib.h>
#include "queue.h"

struct queue_struct{
    int no_of_nodes;
    node head;
};

struct queuenode{
    int pid;
    node next;
};

queue init_q(void){
    queue q = malloc(sizeof(struct queue_struct));
    q->no_of_nodes = 0;
    q->head = NULL;
}

int pop(queue q){
    if(q->no_of_nodes == 0 /*empty*/)
        return -1;
    int ret = q->head->pid;
    node to_be_freed = q->head;
    q->head = q->head->next;
    free(to_be_freed);
    q->no_of_nodes--;
    return ret;
}

int empty_q(queue q){
    if(q->no_of_nodes == 0)
        return 1;
    else
        return 0;
}

void push(queue q, int new_pid){
    q->no_of_nodes++;
    if(q->head == NULL){
        node new_node = malloc(sizeof(struct queuenode));
        new_node->pid = new_pid;
        new_node->next = NULL;
        q->head = new_node;
        return;
    }
    node temp = q->head;
    while(temp->next)
        temp = temp->next;
    node new_node = malloc(sizeof(struct queuenode));
    new_node->pid = new_pid;
    new_node->next = NULL;
    temp->next = new_node;
    return;
}

void destroy_q(queue q){
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