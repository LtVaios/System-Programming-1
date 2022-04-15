#include <stdio.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(){
    int pid;
    if ((pid = fork()) < 0) {
        perror("Error! Could not fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        char* args[] = {"inotifywait", "-m", "-e", "create", "--format", "'%:e %f'", "./../../input_files", NULL};
        if (execvp("inotifywait", args) < 0) {
            perror("Error! exec failed");
            exit(EXIT_FAILURE);
        }
    }
}