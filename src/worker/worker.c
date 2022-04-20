#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include "../set/set.h"
#include "worker.h"


int main(int argc, char *argv[]) {
    const char quote_char[2] = "'";
    char* file_str_ptr = malloc(100);
    char* filename = malloc(100);
    char* pathname = malloc(100);
    char* pipename = malloc(100);
    char reader[1024];
    char buffer[100];
    int fd, temp;

    //Configuring path to the monitored directory
    strcpy(pathname, argv[0]);
    
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

        //printf("child %d got file:%s\n",getpid(),filename);
        process_file(pathname, filename);

        //Send stop signal to yourself
        kill(getpid(), SIGSTOP);
    }
    close(fd);
    free(pipename);
    free(file_str_ptr);
    free(filename);
    free(pathname);
}

int process_file(char* path, char* file){
    int j, flag=0, temp, fd_read, fd_write;
    char byte;
    char buffer[1024];

    //configuring the files we need to read from and write to
    char* read_from = malloc(100);
    strcpy(read_from, path);
    strcat(read_from, file);

    char* write_to = malloc(100);
    strcpy(write_to, file);
    strcat(write_to, ".out");
    
    fd_read = open(read_from, O_RDWR);

    set s = init_set();
    //In this loop we read the file byte-by-byte until the end of file and we filter out URLs
    while( 1 ){
        temp=read(fd_read, &byte, 1);
        //printf("byte %c\n",byte);
        if(temp == 0)
            break;
        if(temp == -1 && errno != EINTR){
            perror("Error");
            break;
        }
        if(byte != 'h')
            continue;
        else{
            read(fd_read, &byte, 1);
            if(byte != 't')
                continue;
            else{
                read(fd_read, &byte, 1);
                if(byte != 't')
                    continue;
                else{
                    read(fd_read, &byte, 1);
                    if(byte != 'p')
                        continue;
                    else{
                        //if the code reached this section it means that we read "http" from the file so it is a link
                        flag = 0;
                        //no we skip the next 7 characters which is "://www."
                        for(int i=0 ; i<7 ; i++){
                            if(byte == EOF){
                                flag = 1;
                                break;
                            }
                            temp=read(fd_read, &byte, 1);
                            if(temp == -1 && errno != EINTR)
                                printf("Error reading from file.\n");
                            continue;
                        }
                        if(flag == 1)
                            break;
                        j = 0;
                        //clear buffer that holds the domains we find
                        memset(buffer, 0, 1024);
                        //In this loop we read until we find a slash or a space which means it is the end of the domain of the link
                        while(byte != '/' && byte != ' ' && byte != '\n'){
                            temp=read(fd_read, &byte, 1);
                            if(temp == -1 && errno != EINTR)
                                printf("Error reading from file.\n");
                            buffer[j] = byte;
                            j++;
                        }
                        //we get rid ot the last slash or space character
                        buffer[j-1] = '\0';
                        //we insert the domain we found into our list
                        set_insert(s, buffer);
                    }
                }
            }
        }
    }
    write_all_to_file(s ,write_to);
    delete_set(s);
    free(read_from);
    free(write_to);
}

static void WorkerSIGINTHandler(int sig) {
    exit(1);
}