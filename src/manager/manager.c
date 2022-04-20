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
    path = malloc(sizeof(argv[2]));
    strcpy(path, "./");
    if(argc == 1){
        printf("Default path is now current file.\n");
    }
    else{
        if(strcmp(argv[1],"-p") != 0){
            printf("Unknown flag, default path is now current file.\n");
        }
        else {
            char* ptr = argv[2];
            int counter = 0;
            int flag = 0;
            while(ptr){
                if(*ptr == '\0' || *ptr == ' '){
                    flag = 1;
                    break;
                }
                ptr++;
                counter++;
            }
            if(flag == 1){
                strncpy(path, argv[2], counter);
                strcat(path, "/");
            }
            else
                printf("Unknown argument syntax, default path is now the current file.\n");
        }
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
            char* args[] = {"inotifywait", "-m", "-e", "moved_to","-e","create", "--format", "'%:e %f'",path, NULL};
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
        //Queue initialization
        q = init_q();

        //rest variables
        pipename = malloc(100);

        //We use 2 pointer variables here because the first is the one returned by malloc so we can free it later 
        //Also we use the second because we need to move the pointer with strtok
        tmp_text_to_free = malloc(1024);
        char* tmp_text = tmp_text_to_free;

        char* token;
        const char s[2] = "\n";
        const char quote_char[2] = "'";
        char writer[1024];
        char buffer[100];
        pid_t new_worker_pid, curr_worker_pid;
        

        //Connecting the manager with the signal handlers (SIGCHLD, SIGINT)
        signal(SIGINT, ManagerSIGINTHandler);
        signal(SIGCHLD, ManagerSIGCHLDHandler);

        while (1) { 
            //Reading from the listener pipe
            ssize_t ret = read(listener_pipe[0], writer, BUFSIZE);
            strcpy(tmp_text, writer);
            if (ret == -1)
                continue;
            //Programm functionality
            else {
                //We make a loop with strtok here because inotifywait() might send one big message including a lot of notifications 
                //if we drag&drop a lot of files into the monitored directory so we need to check them all
                while((token = strtok_r(tmp_text, "\n", &tmp_text)) != NULL){
                    //I've noticed that in some cases when we drag&drop >1 files into watched folder, inotifywait() adds some junk characters after all the file notifications
                    //So the valid notifications look like 'MOVED_TO file' or 'CREATE file' ,if we se something else we break the loop
                    if((strncmp(token, "'M", 2) != 0) && (strncmp(token, "'C", 2) != 0))
                        break;
                    curr_worker_pid = pop(q);
                    //if we have no available workers we must make some
                    if(curr_worker_pid == -1){        
                        if ((new_worker_pid = fork()) < 0) {
                            perror("Error! Could not fork");
                            exit(EXIT_FAILURE);
                        }
                        //worker exec
                        if(new_worker_pid == 0){
                            if (execl("./worker", path, NULL) < 0) {
                                perror("Error! exec failed");
                                exit(EXIT_FAILURE);
                            }
                        }
                        //we make a new named pipe with that worker's PID named worker_pipe_PID
                        //and the manager will communicate with that worker only through this pipe
                        strcpy(pipename, _PIPE_);
                        sprintf(buffer, "%d", new_worker_pid);
                        strcat(pipename, buffer);
                        if(mkfifo(pipename, 0666) != 0)
                            printf("Error creating named pipe or it already exists.\n");
                        curr_worker_pid = new_worker_pid;
                    }
                    //We manipulate strings to make the appropriate named pipe filename so we talk with the right worker
                    strcpy(pipename, _PIPE_);
                    sprintf(buffer, "%d", curr_worker_pid);
                    strcat(pipename, buffer); 
                    fd = open(pipename, O_CREAT|O_RDWR);
                    if(write(fd, token, strlen(token)+1) == -1 && errno != EINTR)
                        printf("Error writing in named pipe.\n");
                    //we wrote in the worker's named pipe and now we send SIGCONT to that worker
                    kill(curr_worker_pid, SIGCONT); 
                }
            }
        }
    }
}

//This handler manages SIGCHLD signals received from workers and just pushes them back to the queue
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
    //close the pipe we used to communicate with the listener
    close(listener_pipe[0]);
    //killng all workers
    while( empty_q(q) != 1 ) {
        child_pid = pop(q);
        kill(child_pid, SIGKILL);
    }
    //freeing the queue
    destroy_q(q);
    free(tmp_text_to_free);
    free(pipename);
    free(path);
    exit(1);
}