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
    char www[5];

    char* fullpathname = malloc(256);
    strcpy(fullpathname, "./output_files/");

    //configuring the files we need to read from and write to
    char* read_from = malloc(100);
    strcpy(read_from, path);
    strcat(read_from, file);

    char* write_to = malloc(100);
    strcpy(write_to, file);
    strcat(write_to, ".out");
    
    strcat(fullpathname, write_to);
    //Checking if we already made this file so it is ready and we skip
    if (access(fullpathname, F_OK) != -1){
        free(write_to);
        free(read_from);
        free(fullpathname);
        return 0;
    }
    fd_read = open(read_from, O_RDWR);
    printf("File %s is ready.\n",write_to);

    set s = init_set();
    //In this loop we read the file byte-by-byte until the end of file and we filter out URLs
    while( 1 ){
        temp=read(fd_read, &byte, 1);
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
                        read(fd_read, &byte, 1);
                        if(byte != ':')
                            continue;
                        else{
                            read(fd_read, &byte, 1);
                            if(byte != '/')
                                continue;
                            else{
                                read(fd_read, &byte, 1);
                                if(byte != '/')
                                    continue;
                                else{
                                    //clear the buffer that holds the domains we find
                                    memset(buffer, 0, 1024);
                                    //if the code reached this section it means that we read "http" from the file so it is a link
                                    flag = 0;
                                    //checking if there is a www. in the link
                                    temp=read(fd_read, &www, 4);
                                    if(temp == -1 && errno != EINTR)
                                        printf("Error reading from file.\n");
                                    if(strcmp(www,"www.") != 0){
                                        for(int i=0 ; i<4 ; i++){
                                            if(www[i] == EOF){
                                                flag = 1;
                                                break;
                                            }
                                            else
                                                buffer[i] = www[i];
                                        }
                                        j = 4;
                                    }
                                    else{
                                        //if the URL did not have 'www.' then the index of the buffer becomes 0 so we write drom its start
                                        j = 0;
                                    }
                                    if(flag == 1)
                                        break;
                                    byte = 0;
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
            }
        }
    }
    write_all_to_file(s ,write_to);
    delete_set(s);
    free(read_from);
    free(fullpathname);
    free(write_to);
}

static void WorkerSIGINTHandler(int sig) {
    exit(1);
}