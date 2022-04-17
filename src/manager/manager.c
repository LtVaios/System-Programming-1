#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "manager.h"


int main(int argc, char *argv[]) {
    //Programm arguments checking
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
        //Listener handles the SIGINT signal so he can terminate inotifywait before exiting
        signal(SIGINT, ListenerHandler);
        if ((notify_pid = fork()) < 0) {
            perror("Error! Could not fork");
            exit(EXIT_FAILURE);
        }
        //The listener's child process code: first we connect the child's output to the pipe and then we call inotifywait through exec to replace it's code
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
        //Creating a named pipe
        int fd, temp, temp_pid, flag=0;
        if(mkfifo(_PIPE_, 0666) != 0)
            printf("Error creating named pipe or it already exists.\n");
        q = init_q();
        char* file_str_ptr = malloc(100);
        char* filename = malloc(100);
        char reader[1024];
        char writer[1024];
        int worker_pid;
        while (1) {
            ssize_t ret = read(listener_pipe[0], writer, BUFSIZE);
            if (ret == -1)
                continue;
            else {
                //Programm functionality here
                fd = open(_PIPE_, O_CREAT|O_RDWR);
                if(write(fd, writer, strlen(writer)) == -1)
                    printf("Error writing in named pipe.\n");
                close(fd);

                //if we have no available workers we must make some
                if(empty_q(q)){
                    if ((worker_pid = fork()) < 0) {
                        perror("Error! Could not fork");
                        exit(EXIT_FAILURE);
                    }
                    //worker code
                    if(worker_pid == 0){
                        while(1){
                            signal(SIGINT,WorkerHandler);
                            //Opening the named pipe that is connected to the manager to get the message sent by inotifywait which we then store inside "reader"
                            fd = open(_PIPE_, O_CREAT|O_RDWR);
                            temp=read(fd, reader, sizeof(reader));
                            if(temp == -1)
                                printf("Error reading from named pipe.\n");
                            reader[temp] = '\0';
                            close(fd);

                            //exctracting the filename from the inotify message which looks like: 'MOVED_TO test.txt'
                            //taking a pointer to the first space
                            file_str_ptr = strchr(reader, ' ');
                            //skiping the space character
                            file_str_ptr++;
                            //copying the filename without the last ' and now we have the filenamed of the file we added, stored in "filename"
                            strncpy(filename, file_str_ptr, strlen(file_str_ptr)-2);
                            printf("child %d got file:%s\n",getpid(),filename);
                            kill(getpid(), SIGSTOP);
                        }
                    }
                    //We make a signal is this section of the code because otherwise every forked worker would also have a copy of the signal
                    //And because this code will run >1 times then we use a flag to make only 1 signal
                    if(!flag){
                        signal(SIGINT, ManagerHandler);
                        flag = 1;
                    }
                }
                else{
                    temp_pid = pop(q);
                    kill(temp_pid, SIGCONT);  
                }
                temp_pid = waitpid(-1, NULL, WSTOPPED | WEXITED);
                push(q, worker_pid);
            }
        }
    }
}

//This SIGINT handler kills inotifywait() process before terminating
static void ListenerHandler(int sig) {
    kill(notify_pid, SIGKILL);
    //printf("\n");
    exit(1);
}

static void WorkerHandler(int sig) {
    exit(1);
}

//This SIGINT manager handler waits all children to finish, destroys the queue and closes the listener pipe before exiting
static void ManagerHandler(int sig) {
    int child_pid;
    close(listener_pipe[0]);
    //killng all workers
    while( empty_q(q) != 1 ) {
        child_pid = pop(q);
        kill(child_pid, SIGKILL);
    }
    //freeing the queue
    destroy_q(q);
    printf("\n");
    exit(1);
}