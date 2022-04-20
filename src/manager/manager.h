#include "../queue/queue.h"

#define BUFSIZE 1024
#define _PIPE_ "./named_pipes/worker_pipe_"

int notify_pid = -1;
int listener_pipe[2];
queue q;
int fd;
//These are global so we can free them from the SIGINT handler
char* pipename, *tmp_text_to_free, *path;

static void ManagerSIGCHLDHandler(int sig);
static void WorkerSIGINTHandler(int sig);
static void ListenerSIGINTHandler(int sig);
static void ManagerSIGINTHandler(int sig);