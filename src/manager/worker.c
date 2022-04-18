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
    const char quote_char[2] = "'";
    char* file_str_ptr = malloc(100);
    char* filename = malloc(100);
    char reader[1024];
    char buffer[100];
    char* pipename = malloc(100);
    int fd, temp;
    
    signal(SIGINT,WorkerSIGINTHandler);
    while(1){
        //We manipulate strings to make the named pipe with this workers PID that is connected to the manager to get the message sent by inotifywait which we then store inside "reader"
        strcpy(pipename, _PIPE_);
        sprintf(buffer, "%d", getpid());
        strcat(pipename, buffer);
        fd = open(pipename, O_CREAT|O_RDWR);
        temp=read(fd, reader, sizeof(reader));
        if(temp == -1 && errno != EINTR)
            printf("Error reading from named pipe.\n");
        //reader[temp] = '\0';

        //exctracting the filename from the inotify message which looks like: 'MOVED_TO test.txt'
        //taking a pointer to the first space
        file_str_ptr = strchr(reader, ' ');
        //skiping the space character
        file_str_ptr++;
        //copying the filename without the last ' and now we have the filenamed of the file we added, stored in "filename"
        strncpy(filename, strtok(file_str_ptr, quote_char), strlen(file_str_ptr)+1);
        printf("child %d got file:%s\n",getpid(),filename);
        kill(getpid(), SIGSTOP);
    }
    close(fd);
}

static void WorkerSIGINTHandler(int sig) {
    exit(1);
}