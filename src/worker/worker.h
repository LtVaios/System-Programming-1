#define BUFSIZE 1024
#define _PIPE_ "./named_pipes/worker_pipe_"

static void WorkerSIGINTHandler(int sig);
int process_file(char* path, char* file);