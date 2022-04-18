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
        signal(SIGINT, ListenerSIGINTHandler);
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
        int temp, flag=0;

        //Queue initialization
        q = init_q();

        //sigaction struct initialise to handle SIGCHLD signals
        // struct sigaction sa;
        // sa.sa_handler = ManagerSIGCHLDHandler;
        // sa.sa_flags = 0;
        // sigset_t set;
        // sigemptyset(&set);
        // sa.sa_mask = set;
        // sigaction(SIGCHLD, &sa, NULL);

        //rest variables
        char* token = malloc (100);
        char* tmp_text = malloc(1024);
        const char s[2] = "\n";
        const char quote_char[2] = "'";
        char* file_str_ptr = malloc(100);
        char* filename = malloc(100);
        char reader[1024];
        char writer[1024];
        char buffer[100];
        pid_t worker_pid, temp_pid;
        char* pipename = malloc(100);


        while (1) { 
            signal(SIGINT, ManagerSIGINTHandler);
            signal(SIGCHLD, ManagerSIGCHLDHandler);
            ssize_t ret = read(listener_pipe[0], writer, BUFSIZE);
            strcpy(tmp_text, writer);
            if (ret == -1)
                continue;
            else {
                //Programm functionality here
                while((token = strtok_r(tmp_text, "\n", &tmp_text)) != NULL){
                    //I've noticed that in some cases when we drag&drop >1 files into watched folder, inotifywait() adds some junk characters after all the file notifications
                    //So the valid notifications look like 'MOVED_TO file' or 'CREATE file' ,if we se something else we break the loop
                    if((strncmp(token, "'M", 2) != 0) && (strncmp(token, "'C", 2) != 0))
                        break;
                  
                    temp_pid = pop(q);
                    //if we have no available workers we must make some
                    if(temp_pid == -1){        
                        if ((worker_pid = fork()) < 0) {
                            perror("Error! Could not fork");
                            exit(EXIT_FAILURE);
                        }
                        //worker code
                        if(worker_pid == 0){
                            if (execl("./worker", "123", NULL) < 0) {
                                perror("Error! exec failed");
                                exit(EXIT_FAILURE);
                            }
                        }
                        strcpy(pipename, _PIPE_);
                        sprintf(buffer, "%d", worker_pid);
                        strcat(pipename, buffer);
                        if(mkfifo(pipename, 0666) != 0)
                            printf("Error creating named pipe or it already exists.\n");
                        temp_pid = worker_pid;
                    }
                    //If the queue is not empty then we take the first available worker and send SIGCONT
                    strcpy(pipename, _PIPE_);
                    sprintf(buffer, "%d", temp_pid);
                    strcat(pipename, buffer); 
                    fd = open(pipename, O_CREAT|O_RDWR);
                    //printf("i write %s in %s\n",token,pipename);
                    if(write(fd, token, strlen(token)+1) == -1 && errno != EINTR)
                        printf("Error writing in named pipe.\n");
                    //close(fd);
                    kill(temp_pid, SIGCONT); 
                }
            }
        }
    }
}

static void ManagerSIGCHLDHandler(int sig) {
    while(1){
        int stat;
        int chld = waitpid(-1, &stat, WUNTRACED | WNOHANG);
        if(chld <= 0)
            break;
        push(q, chld);
    }
}


//This SIGINT handler kills inotifywait() process before terminating
static void ListenerSIGINTHandler(int sig) {
    kill(notify_pid, SIGKILL);
    //printf("\n");
    exit(1);
}

//This SIGINT manager handler waits all children to finish, destroys the queue and closes the listener pipe before exiting
static void ManagerSIGINTHandler(int sig) {
    printf("\n");
    int child_pid;
    close(listener_pipe[0]);
    //killng all workers
    while( empty_q(q) != 1 ) {
        child_pid = pop(q);
        kill(child_pid, SIGKILL);
    }
    //freeing the queue
    destroy_q(q);
    close(fd);
    exit(1);
}