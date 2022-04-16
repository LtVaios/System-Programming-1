#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include "manager.h"
#include "../queue/queue.h"

int main(int argc, char *argv[]) {
    //Argument and flag checking
    char* path = malloc(sizeof(argv[2]));
    strcpy(path, "./");
    if(argc == 1){
        printf("Default path is now current file.\n");
    }
    else{
        if(strcmp(argv[1],"[-p") != 0){
            printf("Unknown flag, default path is now current file.\n");
        }
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

    //Pipe that connects manager and listener initialization
    int listener_pipe[2];
    if (pipe(listener_pipe) < 0){
        exit(1);
        printf("Error! pipe failed.\n");
    }

    //Listener init
    int listen_pid;
    if ((listen_pid = fork()) < 0) {
        perror("Error! Could not fork");
        exit(EXIT_FAILURE);
    }
    //Listener code
    if (listen_pid == 0) {
        //Listener handles the SIGINT signal so he can terminate inotify before exiting
        signal(SIGINT, Handler);
        if ((notify_pid = fork()) < 0) {
            perror("Error! Could not fork");
            exit(EXIT_FAILURE);
        }
        //The listener's child process code: first we connect the child's output to the pipe and then we call inotify to replace it's code
        else if (notify_pid == 0) {
            if(!dup2(listener_pipe[1], STDOUT_FILENO)) {
                perror("dup2 error");
            }
            close(listener_pipe[1]);
            close(listener_pipe[0]);
            char* args[] = {"inotifywait", "-m", "-e", "moved_to,create", "--format", "'%:e %f'",path, NULL};
            if (execvp("inotifywait", args) < 0) {
                perror("Error! exec failed");
                exit(EXIT_FAILURE);
            }
        }
        //rest listener code
        else
            //listener doesn't have much else to do because inotify does the rest of the job. We just wait here for the SIGINT signal.
            wait(NULL);
    }
    //manager code
    else {
        char str[1024];
        while (1) {
            ssize_t ret = read(listener_pipe[0], str, BUFSIZE);
            if (ret == -1)
                printf("Pipe read error");
            else if (ret == 0) {
                //if read returns 0 (\0) it means we reached the end of file so we break the loop 
                break;
            } 
            else {
                //Programm functionality here
                printf("edw\n");
            }
        }
        close(listener_pipe[0]);
        wait(0);
    }
    return 0;
}

//This SIGINT handler kills inotifywait() process before terminating
static void Handler(int sig) {
    kill(notify_pid, SIGKILL);
    printf("\n");
    exit(1);
}